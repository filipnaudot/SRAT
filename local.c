// gcc local.c -o local && ./local  178.62.198.171 8080
/**
 * @file local.c
 * @author Filip Naudot
 * @brief Establishes a connection and sends commands
 * @version 0.1
 * @date 2022-07-02
 * 
 */

#include "drat.h"

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    #ifdef VERBOSE
    printf("Configuring remote address...\n");
    #endif

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    #ifdef VERBOSE
    printf("Remote address is: ");
    #endif
    
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
            address_buffer, sizeof(address_buffer),
            service_buffer, sizeof(service_buffer),
            NI_NUMERICHOST);
    #ifdef VERBOSE
    printf("%s %s\n", address_buffer, service_buffer);
    #endif


    #ifdef VERBOSE
    printf("Creating socket...\n");
    #endif

    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
            peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    #ifdef VERBOSE
    printf("Connecting...\n");
    #endif

    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);

    #ifdef VERBOSE
    printf("Connected.\n");
    #endif

    while (1) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(STDIN_FILENO, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer+1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        if (FD_ISSET(socket_peer, &reads)) {
            char read[4096];
            int bytes_received = recv(socket_peer, read, 4096, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("%s", read);
        }

        if (FD_ISSET(0, &reads)) {
            char read[4096];
            if (!fgets(read, 4096, stdin)) break;
            
            #ifdef VERBOSE
            printf("Sending: %s", read);
            #endif
            
            int bytes_sent = send(socket_peer, read, strlen(read), 0);

            #ifdef VERBOSE
            printf("Sent %d bytes.\n", bytes_sent);
            #endif
        }
    } //end while(1)

    printf("Closing socket...\n");
    CLOSESOCKET(socket_peer);

    printf("Finished.\n");
    return 0;
}