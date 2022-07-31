#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netdb.h>

void send_file(FILE *fp, int sockfd, long file_size);
void write_file(int socket_peer, char* filename);