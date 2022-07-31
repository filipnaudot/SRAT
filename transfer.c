#include "transfer.h"

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
    long file_size = 0;
    long total_bytes_recieved = 0;

    fp = fopen(filename, "w");
    
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