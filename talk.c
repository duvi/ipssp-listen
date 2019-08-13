/*
** talker.c - egy datagram "kliens" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"

void talk(char *message, int dest_port)
{

int sockfd;
struct sockaddr_in their_addr; // a csatlakozó címinformációja
struct hostent *he;
int numbytes;

usleep(300);

if ((he=gethostbyname("127.0.0.1")) == NULL) { // veszi a hosztinformációt
perror("gethostbyname");
exit(1);
}

if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {

perror("socket");
exit(1);
}

their_addr.sin_family = AF_INET; // host byte order
their_addr.sin_port = htons(dest_port); // short, network byte order
their_addr.sin_addr = *((struct in_addr *)he->h_addr);
memset(&(their_addr.sin_zero), '\0', 8); // kinullázza a struktúra maradék részét

if ((numbytes=sendto(sockfd, message, strlen(message), 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {

perror("sendto");
exit(1);
}

//printf("%s", message);
printf("sent %d bytes to %s:%i containing: %s\n", numbytes,inet_ntoa(their_addr.sin_addr),their_addr.sin_port,message);

close(sockfd);

}

