#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)


#define MAX_RECEIVE 1024

#define true 1
#define false 0


typedef struct addrinfo addrinfo;

int num_commands(char* read, int bytes_received);
void parse_commands(char* read, int bytes_received, char** commands);