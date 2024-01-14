#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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
#define FILE_EXISTS 3
#define FILE_MISSING 4

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
 * @brief retrieves name of a file to either send or recieve
 * 
 * @param string string with command including the file name
 */
void retreive_filename(char* string);

/**
 * @brief Creates a connection to remote host and returns socket
 * 
 * @param ip the ip-address of remote host
 * @param port the remote host port to connect to
 * 
 * @return SOCKET the new socket connected to remote host
 */
SOCKET get_connection(char* ip, char* port);