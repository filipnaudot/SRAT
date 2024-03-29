#include "srat.h"
#include "transfer.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: local [IP] [PORT]\n");
        exit(EXIT_FAILURE);
    }

    #ifdef VERBOSE
    printf("Configuring remote address...\n");
    #endif

    SOCKET socket_peer = get_connection(argv[1], argv[2]);

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
            exit(EXIT_FAILURE);
        }

        data_packet data;
        data.transfer_status = NO_TRANSFER;
        memset(&data.read, '\0', STANDARD_BUFFER_SIZE);

        if (FD_ISSET(socket_peer, &reads)) {
            int bytes_received = recv(socket_peer, data.read, STANDARD_BUFFER_SIZE, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("%.*s\n", bytes_received, data.read);
        }

        if (FD_ISSET(STDIN_FILENO, &reads)) {
            memset(&data.read, '\0', STANDARD_BUFFER_SIZE);
            if (!fgets(data.read, STANDARD_BUFFER_SIZE, stdin)) break;
            data.read[strcspn(data.read, "\n")] = '\0';

            #ifdef VERBOSE
            printf("Sending: %s", data.read);
            #endif
            
            if (strncmp("get ", data.read, 4) == 0) {
                data.transfer_status = GET;
                retreive_filename(data.read);
            } else if (strncmp("put ", data.read, 4) == 0) {
                data.transfer_status = PUT;
                send(socket_peer, &data.transfer_status, sizeof(int), 0);
                retreive_filename(data.read);
                size_t filename_size = strlen(data.read);
                // Send size of file name
                send(socket_peer, &filename_size, sizeof(size_t), 0);
                // Send the file name
                send(socket_peer, data.read, strlen(data.read), 0);
            }
            
            if (data.transfer_status != PUT) {
                send(socket_peer, &data.transfer_status, sizeof(int), 0); 
                int bytes_sent = send(socket_peer, data.read, strlen(data.read), 0);
                
                #ifdef VERBOSE
                printf("Sent %d bytes.\n", bytes_sent);
                #endif
            }
            
            if (data.transfer_status == GET) {
                recv(socket_peer, &data.transfer_status, sizeof(int), 0);
                if (data.transfer_status == FILE_EXISTS) {
                    write_file(socket_peer, data.read);    
                }
            } else if (data.transfer_status == PUT) {
                FILE* fp = fopen(data.read, "r");
                long file_size = get_file_size(fp);
                send_file(fp, socket_peer, file_size);
            }
        }
    } //end while(1)
    
    close(socket_peer);

    return 0;
}


void retreive_filename(char* string) {
    int i;
    for (i = 0; i < (strlen(string) - 4); i++) {
        string[i] = string[i+4];
    }
    string[i] = '\0';
}


SOCKET get_connection(char* ip, char* port) {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *peer_address;
    if (getaddrinfo(ip, port, &hints, &peer_address)) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }
    
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
    printf("Remote address is: %s %s\n", address_buffer, service_buffer);
    printf("Creating socket...\n");
    #endif

    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
                        peer_address->ai_socktype,
                        peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    #ifdef VERBOSE
    printf("Connecting...\n");
    #endif

    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(peer_address);

    return socket_peer;
}