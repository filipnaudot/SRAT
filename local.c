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

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
        perror("getaddrinfo");
        return 1;
    }

    #ifdef VERBOSE
    printf("Remote address is: ");
    #endif
    
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr,
                peer_address->ai_addrlen,
                address_buffer,
                sizeof(address_buffer),
                service_buffer,
                sizeof(service_buffer),
                NI_NUMERICHOST);
    #ifdef VERBOSE
    printf("%s %s\n", address_buffer, service_buffer);
    #endif


    #ifdef VERBOSE
    printf("Creating socket...\n");
    #endif

    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
                        peer_address->ai_socktype,
                        peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        perror("socket");
        return 1;
    }

    #ifdef VERBOSE
    printf("Connecting...\n");
    #endif

    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
        perror("connect");
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
            perror("select");
            return 1;
        }

        data_packet data;
        data.is_get = false;
        memset(&data.read, 0, sizeof(data.read));

        if (FD_ISSET(socket_peer, &reads)) {
            int bytes_received = recv(socket_peer, data.read, STANDARD_BUFFER_SIZE, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("%.*s\n", bytes_received, data.read);
        }

        if (FD_ISSET(STDIN_FILENO, &reads)) {
            if (!fgets(data.read, STANDARD_BUFFER_SIZE, stdin)) break;
            
            #ifdef VERBOSE
            printf("Sending: %s", data.read);
            #endif

            if (strncmp("get ", data.read, 4) == 0) {
                printf("START FILE TRANSFER\n");
                data.is_get = true;
                int i;
                for (i =  0; i < strlen(data.read) - 4; i++) {
                    data.read[i] = data.read[i+4];
                }
                data.read[i] = '\0';
            }

            send(socket_peer, &data.is_get, sizeof(data.is_get), 0); 
            int bytes_sent = send(socket_peer, data.read, strlen(data.read), 0); 
            
            #ifdef VERBOSE
            printf("Sent %d bytes.\n", bytes_sent);
            #endif
            
            if (data.is_get) {
                write_file(socket_peer);
            }
        }
    } //end while(1)
    
    close(socket_peer);

    return 0;
}


void write_file(int socket_peer) {
    int n;
    FILE *fp;
    char *filename = "recv.txt";
    char buffer[1024];
    
    printf("RECIEVING FILE...\n");
    fp = fopen(filename, "w");
    
    int i = 0;
    while (1) {
        printf("%d\n", i++);

        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(STDIN_FILENO, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
    
        if (select(socket_peer+1, &reads, 0, 0, &timeout) < 0) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(socket_peer, &reads)) {
            int n = recv(socket_peer, buffer, 1024, 0);
            if (n < 1) {
                break;
            }
            printf("Recieved %d bytes\n", n);
            printf("RECEIVED: [%s]\n", buffer);
        
            fprintf(fp, "%s", buffer);
            bzero(buffer, 1024);
        }
    }
    fclose(fp);
    printf("FILE RECIEVED\n");
}