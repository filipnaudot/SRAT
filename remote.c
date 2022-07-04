/**
 * @file remote.c
 * @author Filip Naudot
 * @brief Establishes a connection and executes commands
 * @version 0.1
 * @date 2022-07-02
 * 
 */

#include "drat.h"
#include <ctype.h>

#define PORT "8080"

int main(int argc, char *argv[]) {
    printf("Configuring local address...\n");

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo *bind_address;
    getaddrinfo(0, PORT, &hints, &bind_address);


    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                            bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Binding socket to local address...\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    printf("Waiting for connections...\n");

    while(1) {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        for(SOCKET i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {

                if (i == socket_listen) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(socket_listen,
                            (struct sockaddr*) &client_address,
                            &client_len);
                    if (!ISVALIDSOCKET(socket_client)) {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                GETSOCKETERRNO());
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket) {
                        max_socket = socket_client;
                    }

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address, client_len,
                                address_buffer, sizeof(address_buffer), 0, 0,
                                NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);

                } else {
                    char read[MAX_RECEIVE] = {0}; // Buffer to read in to (NOTE: move this out of loop?)
                    int bytes_received = recv(i, read, MAX_RECEIVE, 0); // receieve MAX_RECEIVE bytes
                    
                    #ifdef VERBOSE
                    printf("Recieved %d bytes\n", bytes_received);
                    #endif

                    if (bytes_received < 1) {
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    
                    #ifdef VERBOSE
                    for (int i = 0; i < bytes_received; i++) {
                        printf("%c", read[i]);
                    }
                    printf("\nNumber of commands: %d\n", num_commands(read, bytes_received));
                    #endif

                    char** commands = malloc(num_commands(read, bytes_received) * sizeof(char*));

                    parse_commands(read, commands);

                    for (int j = 0; j < bytes_received; ++j) {
                        read[j] = toupper(read[j]);
                    }
                    send(i, read, bytes_received, 0);
                    
                    free(commands);
                }

            } //if FD_ISSET
        } //for i to max_socket
    } //while(1)

    return 0;
}


/**
 * @brief Counts the number of commands in a buffer
 * 
 * @param read the receive data buffer
 * @param bytes_received the number of bytes received
 * @return int the number of commands
 */
int num_commands(char* read, int bytes_received) {
    int count = 0;
    int in_word = false;

    for(int i = 0; i < bytes_received; i++) {
        if(read[i] == ' ' || read[i] == '\n' || read[i] == '\0') {
            in_word = false;
        } else {
            if(!in_word) {
                count++;
                in_word = true;
            }
        } // else
    } // for-loop

    return count;
}


/**
 * @brief parses the commands in a buffer
 * 
 * @param read the buffer to parse
 * @param commands the buffer to place commands in
 */
void parse_commands(char* read, char** commands) {
    const char separator[2] = " ";
    char *token;
    
    #ifdef VERBOSE
    printf("Parsing commands...\n");
    #endif

    token = strtok(read, separator); // first token
    int i = 0;
    while(token != NULL) {
        #ifdef VERBOSE
        printf("%s\n", token);
        #endif
        commands[i] = token;
        token = strtok(NULL, separator);
        i++;
    }
}