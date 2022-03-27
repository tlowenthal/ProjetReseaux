#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <poll.h>

#include "log.h"
#include "format.h"

int print_usage(char *prog_name) {
    ERROR("Usage:\n\t%s [-f filename] [-s stats_filename] [-c] receiver_ip receiver_port", prog_name);
    return EXIT_FAILURE;
}


int main(int argc, char **argv) {
    int opt;

    char *filename = NULL;
    char *stats_filename = NULL;
    char *receiver_ip = NULL;
    char *receiver_port_err;
    bool fec_enabled = false;
    uint16_t receiver_port;

    while ((opt = getopt(argc, argv, "f:s:h:c")) != -1) {
        switch (opt) {
        case 'f':
            filename = optarg;
            break;
        case 'h':
            return print_usage(argv[0]);
        case 's':
            stats_filename = optarg;
            break;
	    case 'c':
	        fec_enabled = true;
	        break;
        default:
            return print_usage(argv[0]);
        }
    }

    if (optind + 2 != argc) {
        ERROR("Unexpected number of positional arguments");
        return print_usage(argv[0]);
    }

    receiver_ip = argv[optind];
    receiver_port = (uint16_t) strtol(argv[optind + 1], &receiver_port_err, 10);
    if (*receiver_port_err != '\0') {
        ERROR("Receiver port parameter is not a number");
        return print_usage(argv[0]);
    }

    ASSERT(1 == 1); // Try to change it to see what happens when it fails
    DEBUG_DUMP("Some bytes", 11); // You can use it with any pointer type

    //On met les adresses
    struct sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(receiver_port);

    // Convert IPv6 addresses from text to binary form
    if(inet_pton(AF_INET6, receiver_ip, &addr.sin6_addr)<=0) 
    {
        ERROR("\nInvalid address/ Address not supported \n");
        return -1;
    }

    //creation du socket
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        ERROR("\n Socket creation error \n");
        return -1;
    }

    int err = connect(sock, (const struct sockaddr *) &addr, sizeof(addr));
    if (err < 0)
    {
        ERROR("\n Connecting error \n");
        return -1;
    }

    //ouverture du fichier
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        ERROR("\n File opening error \n");
        return -1;
    }

    struct stat buf;
    err = fstat(fd, &buf);
    if (err < 0)
    {
        ERROR("\n Fstat error \n");
        return -1;
    }
    off_t size = buf.st_size;
    int n_msg = size/512 + 1;
    ERROR("nombre de messages à envoyer : %d", n_msg);

    //mapping du contenu de filename dans msg
    uint8_t *mapping = (uint8_t *) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED){
        ERROR("\n Mapping error \n");
        return -1;
    }

    uint8_t *ptr = mapping;
    uint8_t mywindow = 32;
    uint8_t hiswindow = 1;
    uint8_t seqnum = 1;
    uint8_t expected_seq = 1;
    off_t current_size = size;
    int i = 0;
    while (i < n_msg){

        while (current_size > 0 && seqnum <= expected_seq + hiswindow){

            uint8_t *buf = malloc(640);
            if (buf == NULL){
                ERROR("\n buf malloc error \n");
                err = munmap(mapping, size);
                if (err < 0){
                    ERROR("\n munmap error \n");
                    return -1;
                }
                return -1;
            }

            uint16_t len = (current_size > 512) ? 512 : current_size;
            err = format(buf, mywindow, ptr, len, seqnum);
            if (err < 0){
                ERROR("\n Formatting error \n");
                err = munmap(mapping, size);
                if (err < 0){
                    ERROR("\n munmap error \n");
                    return -1;
                }
                return -1;
            }

            err = send(sock, buf, len, 0);
            if (err < 0){
                ERROR("\n Sending error \n");
                err = munmap(mapping, size);
                if (err < 0){
                    ERROR("\n munmap error \n");
                    return -1;
                }
                return -1;
            }

            free(buf);
            current_size-=len;
            ptr+=len;
            seqnum++;
        }

        struct pollfd pfds[1];
        pfds[0].fd = sock; // 0 represents stdin
        pfds[0].events = POLLIN; // check ready to read
        err = poll(pfds, 1, -1);
        if (err < 0){
            ERROR("\n Polling error \n");
            err = munmap(mapping, size);
            if (err < 0){
                ERROR("\n munmap error \n");
                return -1;
            }
            return -1;
        }
        if(pfds[0].revents & POLLIN) {
            uint8_t *rec_buf = malloc(640);
            if (rec_buf == NULL){
                ERROR("\n buf malloc error \n");
                err = munmap(mapping, size);
                if (err < 0){
                    ERROR("\n munmap error \n");
                    return -1;
                }
                return -1;
            }
            err = recv(sock, rec_buf, 640, 0);
            if (err < 0){
                ERROR("\n Reception error \n");
                err = munmap(mapping, size);
                if (err < 0){
                    ERROR("\n munmap error \n");
                    return -1;
                }
                return -1;
            }
            if (*rec_buf & 1 << 5) continue;
            *rec_buf = *rec_buf & ~(1 << 6);
            *rec_buf = *rec_buf & ~(1 << 7);
            hiswindow = *rec_buf;
            expected_seq = *(rec_buf + 1);
            i++;
        }

    }


    // This is not an error per-se.
    ERROR("Sender has following arguments: filename is %s, stats_filename is %s, fec_enabled is %d, receiver_ip is %s, receiver_port is %u",
        filename, stats_filename, fec_enabled, receiver_ip, receiver_port);

    DEBUG("You can only see me if %s", "you built me using `make debug`");
    ERROR("This is not an error, %s", "now let's code!");

    // Now let's code!
    return EXIT_SUCCESS;
}