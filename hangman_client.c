/* Tiange Wang, Eric Shen */
/* Code Cited: http://www.linuxhowtos.org/data/6/client_c_tcp.c, Sockets Tutorial, Example TCP Client. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    // Initialize
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"Usage: %s hostname port\n", argv[0]);
       exit(1);
    }

    // Establish connection
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // Waiting for server response to join game
    bzero(buffer, 256);
    n = read(sockfd, buffer, 256);
    if (n < 0) error("recvfrom");
    if(buffer[0] == 'A') {
        int start;
        printf("Ready to start the game? (y/n): ");
        while(1) {
            bzero(buffer, 256);
            fgets(buffer, 256, stdin);
            if(strlen(buffer) == 2 && (buffer[0] == 'y' || buffer[0] == 'Y')) {
                start = 1;
                break;
            } else if(strlen(buffer) == 2 && (buffer[0] == 'n' || buffer[0] == 'N')) {
                start = 0;
                break;
            } else {
                printf("Error! Please enter y or n.\n");
            }
        }
        bzero(buffer,256);
        buffer[0] = 'B';
        if(start == 1)
            buffer[1] = 'y';
        else
            buffer[1] = 'n';
        n = write(sockfd, buffer, 256);
        if (n < 0) error("Sendto");
        if(start == 1){
            while(1){
                // Run game
                n = read(sockfd,buffer,256);
                if (n < 0) error("recvfrom");
                if(buffer[0] == 'A') { // In game
                    int word_len = buffer[1] - '0';
                    int incorrects = buffer[2] - '0';
                    for(int i = 3; i < 3+word_len; i++) {
                        printf("%c", buffer[i]);
                    }
                    printf("\nIncorrect Guesses: ");
                    for(int i = 3+word_len; i < 3+word_len+incorrects; i++) {
                        printf("%c ", buffer[i]);
                    }
                    printf("\n");
                }else{
                    printf("%s", buffer+1);
                    break;    
                }
                // User input
                while(1) {
                    printf("\nLetter to guess: ");
                    bzero(buffer, 256);
                    fgets(buffer, 256, stdin);
                    if(strlen(buffer) == 2 && isalpha(buffer[0]) != 0) {
                        buffer[1] = tolower(buffer[0]);
                        buffer[0] = 'B';
                        buffer[2] = '\0';
                        n = write(sockfd, buffer, 256);
                        if (n < 0) error("Sendto");
                        break;
                    } else {
                        printf("Error! Please enter one letter.\n");
                    }
                }
            }
        }
    } else {
        printf("%s\n", buffer+1);
    }
    close(sockfd);
    return 0;
}

/*

printf("Letter to guess: ");
bzero(buffer,256);
fgets(buffer,256,stdin);
buffer[1] = buffer[0];
buffer[0] = '1';
buffer[2] = '\0';
n = write(sockfd,buffer,strlen(buffer));
if (n < 0) error("Sendto");

if(strlen(buffer) == 2 && isalpha(buffer[0]) != 0) {   

*/

/* 

// Check if the server has an available spot for the client
    bzero(buffer, 256);
    buffer[0] = 'a';
    n = write(sockfd, buffer, 256);
    if (n < 0) error("Sendto");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 256);

    // Preparing for game to start
    while(1) {
        printf("Ready to start the game? (y/n): ");
        bzero(buffer, 256);
        fgets(buffer, 256, stdin);
        if(strlen(buffer) == 2 && buffer[0] == 'y') {
            break;
        } else if(strlen(buffer) == 2 && buffer[0] == 'n') {
            close(sockfd);
            return 0;
        } else {
            printf("Error! Please enter y or n.\n");
        }
    }

    // Actual game 
    while(1) {
        // --------Reading Portion---------
        n = read(sockfd,buffer,256);
        if (n < 0) error("recvfrom");
        if(buffer[0] == '0') { // In game
            int word_len = buffer[1] - '0';
            int attempts = buffer[2] - '0';
            for(int i = 3; i < 3+word_len; i++) {
                printf("%c", buffer[i]);
            }
            printf("\nIncorrect Guesses: ");
            for(int i = 0; i < attempts; i++) {
                printf("%c ", buffer[i]);
            }
            printf("\n");
        } else { // Game ends
            int msg_len;
            if(buffer[0] == 'a') {
                msg_len = 10;
            } else {
                msg_len = buffer[0] - '0';
            }
            printf("The word was ");
            for(int i = 1; i < msg_len+1; i++) {
                printf("%c", buffer[i]);
            }
            printf("\n");
            n = read(sockfd,buffer,255);
            if (n < 0) error("recvfrom");
            if(buffer[0] == 'a') {
                msg_len = 10;
            } else {
                msg_len = buffer[0] - '0';
            }
            for(int i = 1; i < msg_len+1; i++) {
                printf("%c", buffer[i]);
            }
            printf("\n");
            n = read(sockfd,buffer,255);
            if (n < 0) error("recvfrom");
            if(buffer[0] == 'a') {
                msg_len = 10;
            } else {
                msg_len = buffer[0] - '0';
            }
            for(int i = 1; i < msg_len+1; i++) {
                printf("%c", buffer[i]);
            }
            printf("\n");
            break;
        }
        // --------Reading Portion---------
        // --------Writing Portion---------

            printf("Error! Please guess one letter.\n");

    }
*/