// gcc local.c -o local && ./local  178.62.198.171 8080
/**
 * @file local.c
 * @author Filip Naudot
 * @brief Establishes a connection and sends commands
 * @version 0.1
 * @date 2022-07-02
 * 
 */

#include "srat.h"

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
        data.transfer_status = NO_TRANSFER;
        memset(&data.read, '\0', sizeof(data.read));

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
                data.transfer_status = GET;
                retreive_filename(data.read);
                /*
                int i;
                for (i =  0; i < strlen(data.read) - 4; i++) {
                    data.read[i] = data.read[i+4];
                }
                data.read[i] = '\0';
                */
            } else if (strncmp("put ", data.read, 4) == 0) {
                data.transfer_status = PUT;
                // make prepare_filename function
                int i;
                for (i =  0; i < strlen(data.read) - 4; i++) {
                    data.read[i] = data.read[i+4];
                }
                data.read[i] = '\0';
            }

            send(socket_peer, &data.transfer_status, sizeof(data.transfer_status), 0); 
            int bytes_sent = send(socket_peer, data.read, strlen(data.read), 0); 
            
            #ifdef VERBOSE
            printf("Sent %d bytes.\n", bytes_sent);
            #endif
            
            if (data.transfer_status == GET) {
                if (data.read[strlen(data.read) - 1] == '\n') data.read[strlen(data.read) - 1] = '\0';
                write_file(socket_peer, data.read);
            } else if (data.transfer_status == PUT) {
                if (data.read[strlen(data.read) - 1] == '\n') data.read[strlen(data.read) - 1] = '\0';
                printf("OPENING [%s]\n", data.read);
                FILE* fp = fopen(data.read, "r");

                fseek(fp, 0L, SEEK_END);
                long file_size = ftell(fp);
                fseek(fp, 0L, SEEK_SET);

                send_file(fp, socket_peer, file_size);
            }
        }
    } //end while(1)
    
    close(socket_peer);

    return 0;
}


void send_file(FILE *fp, int sockfd, long file_size) {
    int n;
    char data[1024] = {0};


    printf("SENDING file_size...\n");
    if (send(sockfd, &file_size, sizeof(long), 0) < 0) {
        perror("send");
        exit(1);
    }
    printf("DONE SENDING file_size\n");

    while (fgets(data, 1024, fp) != NULL) {
        if (send(sockfd, data, strlen(data), 0) < 0) {
            perror("send");
            exit(1);
        }
        bzero(data, 1024);
    }
}


void write_file(int socket_peer, char* filename) {
    int n;
    FILE *fp;
    char buffer[1024];
    long file_size = 0;
    long total_bytes_recieved = 0;

    fp = fopen(filename, "w");
    // recieve file size
    recv(socket_peer, &file_size, sizeof(long), 0);

    do {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
    
        if (select(socket_peer+1, &reads, 0, 0, &timeout) < 0) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(socket_peer, &reads)) {
            int n = recv(socket_peer, buffer, 1024, 0);
            total_bytes_recieved += n;
        
            fprintf(fp, "%s", buffer);
            bzero(buffer, 1024);
        }
    } while(total_bytes_recieved < file_size);

    fclose(fp);
}



void retreive_filename(char* string) {
    int i;
    for (i =  0; i < strlen(string) - 4; i++) {
        string[i] = string[i+4];
    }
    string[i] = '\0';
}