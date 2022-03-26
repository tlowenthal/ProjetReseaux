#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>

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

    //ouverture du fichier
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        ERROR("\n File opening error \n");
        return -1;
    }

    /*
    //allocation memoire pour le msg a envoyer
    uint8_t *msg = malloc(512);
    if (msg == NULL){
        ERROR("\n msg malloc failed \n");
        return -1;
    }
    */

    //mapping du contenu de filename dans msg
    uint8_t *mapping = (uint8_t *) mmap(NULL, 512, PROT_READ, MAP_SHARED, fd, 0);


    // This is not an error per-se.
    ERROR("Sender has following arguments: filename is %s, stats_filename is %s, fec_enabled is %d, receiver_ip is %s, receiver_port is %u",
        filename, stats_filename, fec_enabled, receiver_ip, receiver_port);

    DEBUG("You can only see me if %s", "you built me using `make debug`");
    ERROR("This is not an error, %s", "now let's code!");

    // Now let's code!
    return EXIT_SUCCESS;
}
