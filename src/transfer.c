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
        int n = recv(socket, buffer, 1024, 0);
        total_bytes_recieved += n;

        fprintf(fp, "%s", buffer);
        bzero(buffer, 1024);
        
    } while(total_bytes_recieved < file_size);

    fclose(fp);
}


/**
 * @brief Get size of a given file
 * 
 * @param fp the file pointer
 * @return long the size of the file
 */
long get_file_size(FILE* fp) {
    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return file_size;
}