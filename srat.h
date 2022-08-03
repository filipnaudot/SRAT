#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define SOCKET int


#define MAX_RECEIVE 1024
#define STANDARD_BUFFER_SIZE 4096

#define true 1
#define false 0

// Tranfer status
#define NO_TRANSFER 0
#define GET 1
#define PUT 2

typedef struct addrinfo addrinfo;

typedef struct data_packet {
    int transfer_status;
    char read[STANDARD_BUFFER_SIZE];
} data_packet;

/**
 * @brief execute given command string
 * 
 * @param command the command string to be executed
 * @param return_buffer the output from the command execution
 * @return int the execution status, 0 on success, -1 on failure
 */
int execute_command(char* command, char* return_buffer);

/**
 * @brief Recives and writes file
 * 
 * @param socket_peer the socket used to recieve the file
 * @param filename the name of the file
 */
void write_file(int socket_peer, char* filename);

/**
 * @brief Reads and sends a file
 * 
 * @param fp file pointer to the file to send
 * @param sockfd the socket used the send file
 * @param file_size the size of the file to send
 */
void send_file(FILE *fp, int sockfd, long file_size);

/**
 * @brief retrieves name of a file to either send or recieve
 * 
 * @param string string with command including the file name
 */
void retreive_filename(char* string);