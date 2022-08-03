#include "transfer.h"


/**
 * @brief Reads and sends a file
 * 
 * @param fp file pointer to the file to send
 * @param socket the socket used the send file
 * @param file_size the size of the file to send
 */
void send_file(FILE *fp, int socket, long file_size) {
    int n;
    char data[1024] = {0};

    if (send(socket, &file_size, sizeof(long), 0) < 0) {
        perror("send");
        exit(1);
    }

    while (fgets(data, 1024, fp) != NULL) {
        if (send(socket, data, strlen(data), 0) < 0) {
            perror("send");
            exit(1);
        }
        bzero(data, 1024);
    }
}


/**
 * @brief Recives and writes file
 * 
 * @param socket the socket used to recieve the file
 * @param filename the name of the file
 */
void write_file(int socket, char* filename) {
    int n;
    FILE *fp;
    char buffer[1024];
    long file_size = 0;
    long total_bytes_recieved = 0;

    fp = fopen(filename, "w");
    
    recv(socket, &file_size, sizeof(long), 0);

    do {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket+1, &reads, 0, 0, &timeout) < 0) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(socket, &reads)) {
            int n = recv(socket, buffer, 1024, 0);
            total_bytes_recieved += n;

            fprintf(fp, "%s", buffer);
            bzero(buffer, 1024);
        }
    } while(total_bytes_recieved < file_size);

    fclose(fp);
}