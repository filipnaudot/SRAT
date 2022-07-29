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


typedef struct data_packet {
    int transfer_status;
    char read[STANDARD_BUFFER_SIZE];
} data_packet;


typedef struct addrinfo addrinfo;

void parse_commands(char* read, char** commands);
int execute_command(char* command, char* return_buffer);


void write_file(int socket_peer, char* filename);
void send_file(FILE *fp, int sockfd, long file_size);