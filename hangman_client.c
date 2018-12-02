/* 
    Tiange Wang, ID: 3717659, CS 176A Homework 5
    Code Cited: http://www.linuxhowtos.org/data/6/client_udp.c, Sockets Tutorial, Example UDP Client.
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sock, n;
    unsigned int length;
    struct sockaddr_in server, from;
    struct hostent *hp;
    char buffer[256];
    if (argc != 3) { 
        printf("Usage: server port\n");
        exit(1);
    }
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) error("socket");
    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    if (hp==0) error("Unknown host");
    bcopy((char *)hp->h_addr, 
        (char *)&server.sin_addr,
            hp->h_length);
    server.sin_port = htons(atoi(argv[2]));
    length=sizeof(struct sockaddr_in);

    while(1) {
        printf("Ready to start the game? (y/n): ");
        bzero(buffer,256);
        fgets(buffer,256,stdin);
        if(strlen(buffer) == 2 && buffer[0] == 'y') {
            break;
        } else if(strlen(buffer) == 2 && buffer[0] == 'n') {
            close(sock);
            return 0;
        } else {
            printf("Error! Please enter y or n.\n");
        }
    }

    while(1) {
        printf("Letter to guess: ");
        bzero(buffer,256);
        fgets(buffer,256,stdin);
        if(strlen(buffer) == 1 && isalpha(buffer[0]) != 0) {
            printf("placeholder stuff");
            return 0;
        } else {
            printf("Error! Please guess one letter.");
        }
    }
    
    n = sendto(sock,buffer,strlen(buffer),0,(const struct sockaddr *)&server,length);
    if (n < 0) error("Sendto");
    n = recvfrom(sock,buffer,256,0,(struct sockaddr *)&from, &length);
    if (n < 0) error("recvfrom");
    write(1,"Got an ack: ",12);
    write(1,buffer,n);
    close(sock);
    return 0;
}