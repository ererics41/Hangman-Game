/* 
    Tiange Wang, ID: 3717659, CS 176A Homework 5
    Code Cited: http://www.linuxhowtos.org/data/6/server_c_tcp.c, Sockets Tutorial, Example TCP Server.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr,"Usage: hostname port\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    int pid;
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        error("ERROR on accept");
        //fork new process
        pid = fork();
        if (pid < 0) {
            error("ERROR in new process creation");
        } else if (pid == 0) {
            //child process
            close(sockfd);
            char word[256] = "apple";
            int word_len = 5;
            char guess[256];
            for(int i = 0; i < word_len; i++) {
                guess[i] = '_';
            }
            int attempts = 0;
            char attempt_store[256];
            while(1) {
                bzero(buffer, 256);
                n = read(newsockfd, buffer, 256);
                if (n < 0) error("ERROR reading from socket");
                if(buffer[0] == '0') {
                    buffer[1] = word_len; // Word length
                    buffer[2] = 0; // Num incorrect
                    for(int i = 3; i < 3 + word_len; i++) {
                        buffer[i] = '_';
                    }
                    n = write(newsockfd, buffer, 256);
                    if (n < 0) error("ERROR writing to socket");
                } else {
                    int no_correct = 1;
                    int all_correct = 1;
                    for(int i = 0; i < word_len; i++) { 
                        if(word[i] == buffer[1]) {
                            guess[i] = buffer[1];
                            no_correct = 0;
                        }
                    }
                    for(int i = 0; i < word_len; i++) {
                        if(guess[i] == '_') {
                            all_correct = 0;
                            break;
                        }
                    }
                    if(all_correct == 1) {
                        break; // Success
                    }
                    if(no_correct == 1) {
                        attempt_store[attempts] = buffer[1];
                        attempts++;
                        if(attempts >= 6) { // Fail
                            break;
                        }
                    }
                    
                }
            }
            close(newsockfd);
            exit(0);
        } else {
            //parent process
            close(newsockfd);
        }
    }
    return 0;
}