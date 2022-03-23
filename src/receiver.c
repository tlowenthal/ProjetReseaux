#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "log.h"

#define MYPORT "8080"
#define BACKLOG 5

int create_sock_and_accept(char *listen_ip, uint16_t listen_port){

    //ON PREPARE L'ADRESSE QUE VA UTILISER LE PORT RECEIVER : port 8080 sur la machine locale
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, MYPORT, &hints, &res);

    printf("1\n");

    //ON CREE UN SOCKET AVEC L'ADRESSE CREEE
    int sock = socket(res->ai_family, res->ai_socktype, 0);
    if (sock < 0) return -1;
    int err = bind(sock, res->ai_addr, res->ai_addrlen);  // assign our address to the socket
    if (err == -1) return -1;

    printf("2\n");

    //LETS NOW LISTEN ON THE PORT TO HEAR IF PEER WANTS TO CONNECT
    int listenResult = listen(sock, BACKLOG);
    if (listenResult == -1){perror("listenResult");}

    printf("3\n");

    //ON ENREGISTRE DANS UNE STRUCT IPv6 L'ADRESSE SENDER SU'ON SOUHAITE ACCEPTER
    struct sockaddr_in6 peer_addr;                      // allocate the address on the stack
    memset(&peer_addr, 0, sizeof(peer_addr));           // fill the address with 0-bytes to avoid garbage-values
    peer_addr.sin6_family = AF_INET6;                   // indicate that the address is an IPv6 address
    peer_addr.sin6_port = htons(listen_port);           // indicate that the programm is running on port 55555
    if(!(inet_pton(AF_INET6, listen_ip, &peer_addr.sin6_addr)>0)){
        ERROR("sender could not understand your ipv6 address !");
        return -1;
    }

    //ON ACCEPTE LA CONNECTION QUI CORRESPOND
    socklen_t addr_size = sizeof(peer_addr);
    int new_fd = accept(sock, (struct sockaddr *) &peer_addr, &addr_size);

    return new_fd;
}


int print_usage(char *prog_name) {
    ERROR("Usage:\n\t%s [-s stats_filename] listen_ip listen_port", prog_name);
    return EXIT_FAILURE;
}


int main(int argc, char **argv) {
    int opt;


    //char *stats_filename = NULL;
    char *listen_ip = NULL;
    char *listen_port_err;
    uint16_t listen_port;

    while ((opt = getopt(argc, argv, "s:h")) != -1) {
        switch (opt) {
        case 'h':
            return print_usage(argv[0]);
        case 's':
            //stats_filename = optarg;
            break;
        default:
            return print_usage(argv[0]);
        }
    }


    //FILE* fd = fopen(stats_filename,"w");
    printf("test\n");

    if (optind + 2 != argc) {
        ERROR("Unexpected number of positional arguments");
        return print_usage(argv[0]);
    }

    listen_ip = argv[optind];
    listen_port = (uint16_t) strtol(argv[optind + 1], &listen_port_err, 10);
    if (*listen_port_err != '\0') {
        ERROR("Receiver port parameter is not a number");
        return print_usage(argv[0]);
    }

    printf("test2\n");
    int sock = create_sock_and_accept(listen_ip,listen_port);

    /*
    //ASSERT(1 == 1); // Try to change it to see what happens when it fails
    //DEBUG_DUMP("Some bytes", 11); // You can use it with any pointer type

    // This is not an error per-se.
    ERROR("Receiver has following arguments: stats_filename is %s, listen_ip is %s, listen_port is %u",
        stats_filename, listen_ip, listen_port);

    //DEBUG("You can only see me if %s", "you built me using `make debug`");
    //ERROR("This is not an error, %s", "now let's code!");
    */

    /*
    char buffer[MAX_MESSAGE_SIZE];  // allocate a buffer of MAX_MESSAGE_SIZE bytes on the stack
    ssize_t n_received = recv(sock, buffer, MAX_MESSAGE_SIZE, 0);   // equivalent to do: read(sock, buffer, MAX_MESSAGE_SIZE);
    if (n_received == -1) return -1;


    struct pollfd pollfds*;

    // let's print what we received !
    printf("received %ld bytes:\n", n_received);
    for (int i = 0 ; i < n_received ; i++) {
        printf("0x%hhx ('%c') ", buffer[i], buffer[i]);
    }
    printf("\n");
    */

    close(sock);    // release the resources used by the socket

    return EXIT_SUCCESS;
}
