#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>


/**
 * @brief Reads and sends a file
 * 
 * @param fp file pointer to the file to send
 * @param socket the socket used the send file
 * @param file_size the size of the file to send
 */
void send_file(FILE *fp, int socket, long file_size);


/**
 * @brief Recives and writes file
 * 
 * @param socket the socket used to recieve the file
 * @param filename the name of the file
 */
void write_file(int socket, char* filename);