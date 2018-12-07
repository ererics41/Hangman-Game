/* Tiange Wang, Eric Shen */
/* Code Cited: http://www.linuxhowtos.org/data/6/server_c_tcp.c, Sockets Tutorial, Example TCP Server. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

/*struct game_elements {
    int *newsockfd;
    int *counter;
};*/

int counter = 0;
char wordsList[15][256];
int listCount = 0;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void * runGame(void * args) {
    int socket_fd;
    char buffer[256];
    bzero(buffer,256);
    // struct game_elements *g = args;
    socket_fd = (size_t)args;

    // response to the initial message
    // **** IMPORTANT: WE ARE USING ASCII LETTER 'A' TO REPRESENT THE NUMBER 0 WITH THE MESSAGE LENGTH BEING THE DIFFERENCE FROM 'A'. ****
    buffer[0] = 'A';
    buffer[1] = '\0';
    int n = write(socket_fd, buffer, 256);
    if (n < 0) error("ERROR writing to socket");

    // Response based on client response
    bzero(buffer, 256);
    n = read(socket_fd, buffer, 256);
    if (n < 0) error("ERROR reading from socket");
    if(buffer[1] == 'y'){
        //run the game 
        //pick randomly from list
        int r = rand() % listCount; 
        char word[256];
        strcpy(word, wordsList[r]);
        int word_len = strlen(word)-1;
        char guess[256];
        for(int i = 0; i < word_len; i++) {
            guess[i] = '_';
        }
        int incorrects = 0;
        char incorrect_arr[256];
        while(1) {
            // Sending a game packet
            bzero(buffer, 256);
            char converted = word_len + '0';
            char converted_i = incorrects + '0';
            buffer[0] = 'A';
            buffer[1] = converted;
            buffer[2] =  converted_i;
            strcat(buffer, guess);
            strcat(buffer, incorrect_arr);
            int n = write(socket_fd, buffer, 256);
            if (n < 0) error("ERROR writing to socket");

            // Reading client response
            bzero(buffer,256);
            n = read(socket_fd, buffer,256);
            if (n < 0) error("ERROR reading from socket");
            char g = buffer[1];
            int no_correct = 1;
            int all_correct = 1;
            // Check the input against the word. If something matches, change the corrspoding position in guess and mark the nothing correct flag as 0
            for(int i = 0; i < word_len; i++) { 
                if(word[i] == g && guess[i] == '_') {
                    guess[i] = g;
                    no_correct = 0;
                }
            }
            // Check if there are no more underscores in guess anymore. If not, every character has been guessed out and the user wins the game
            for(int i = 0; i < word_len; i++) {
                if(guess[i] == '_') {
                    all_correct = 0;
                    break;
                }
            }
            // Executes under all correct flag (User wins)
            if(all_correct == 1) {
                int total_len = word_len + 8 + 13 + 10 + 3;
                char len = 'A' + total_len;
                bzero(buffer, 256);
                buffer[0] = len;
                buffer[1] = '\0';
                strcat(buffer, "The word was ");
                strcat(buffer, word);
                strcat(buffer, "\nYou Win!\nGame Over!\n");
                n = write(socket_fd, buffer, 256);
                if (n < 0) error("ERROR writing to socket");
                break; // Success
            }
            // Executes under no correct flag(incorrect guess). Appends to the incorrect guesses array and increments the attempt value
            if(no_correct == 1) {
                incorrect_arr[incorrects] = buffer[1];
                incorrects++;
                incorrect_arr[incorrects] = '\0';
                if(incorrects >= 6) { // Users fails if there has been 6 attempts
                    int total_len = word_len + 9 + 13 + 10 + 3;
                    char len = 'A' + total_len;
                    bzero(buffer, 256);
                    buffer[0] = len;
                    buffer[1] = '\0';
                    strcat(buffer, "The word was ");
                    strcat(buffer, word);
                    strcat(buffer, "\nYou Lose!\nGame Over!\n");
                    n = write(socket_fd, buffer, 256);
                    if (n < 0) error("ERROR writing to socket");
                    break;
                }
            }
        }
    }

    // finished with game close connection
    close(socket_fd);
    counter--;
    return NULL;
}

int main(int argc, char *argv[]) {
    // Read from file
    FILE *fp = fopen("hangman_words.txt", "r");
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        strcpy(wordsList[listCount], line);
        bzero(line,256); 
        listCount++;
    }
    fclose(fp);

    int sockfd, newsockfd, portno;
    //pthread_t threads[4];
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t thread;
    if (argc < 2) {
        fprintf(stderr,"Usage: hostname port\n");
        exit(1);
    }

    // Connection setup
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

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        // struct game_elements args;
        // args.newsockfd = &newsockfd;
        // args.counter = &counter; 
        if (newsockfd < 0)
        error("ERROR on accept");
        if (counter >= 3){
            //return overloard
            bzero(buffer, 256);
            buffer[0] = 'R';
            buffer[1] = '\0';
            strcat(buffer, "server-overloaded");
            int n = write(newsockfd, buffer, 256);
            if (n < 0) error("ERROR writing to socket");
        } else {
            counter++;
            // printf("counter is now %d\n", counter);
            pthread_create(&thread, NULL, runGame, (void *) (size_t) newsockfd);
        }
    }
    return 0;
}

/* Backup Code

while(1) {
    char word[256] = "apple";
    int word_len = 5;
    char guess[256];
    for(int i = 0; i < word_len; i++) {
        guess[i] = '_';
    }
    int attempts = 0;
    char attempt_store[256];
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 256);
    if (n < 0) error("ERROR reading from socket");
    // If the msg flag is 0, start a new game under some fixed conditions
    if(buffer[0] == '0') {
        buffer[0] = '0';
        buffer[1] = word_len; // Word length
        buffer[2] = 0; // Num incorrect
        for(int i = 3; i < 3+word_len; i++) {
            buffer[i] = '_';
        }
        n = write(newsockfd, buffer, 256);
        if (n < 0) error("ERROR writing to socket");
    } else { // Otherwise countinue the game based on stored conditions
        int no_correct = 1;
        int all_correct = 1;
        // Check the input against the word. If something matches, change the corrspoding position in guess and mark the nothing correct flag as 0
        for(int i = 0; i < word_len; i++) { 
            if(word[i] == buffer[1]) {
                guess[i] = buffer[1];
                no_correct = 0;
            }
        }
        // Check if there are no more underscores in guess anymore. If not, every character has been guessed out and the user wins the game
        for(int i = 0; i < word_len; i++) {
            if(guess[i] == '_') {
                all_correct = 0;
                break;
            }
        }
        // Executes under all correct flag (User wins)
        if(all_correct == 1) {
            char len = word_len + '0';
            buffer[0] = len;
            for(int i = 1; i < word_len+1; i++) {
                buffer[i] = word[i-1];
            }
            n = write(newsockfd, buffer, 256);
            if (n < 0) error("ERROR writing to socket");
            buffer[0] = '9';
            buffer[1] = 'Y'; buffer[2] = 'o'; buffer[3] = 'u'; buffer[4] = ' '; buffer[5] = 'W'; buffer[6] = 'i'; buffer[7] = 'n'; buffer[8] = '!';
            n = write(newsockfd, buffer, 256);
            if (n < 0) error("ERROR writing to socket");
            buffer[0] = 'a';
            buffer[1] = 'G'; buffer[2] = 'a'; buffer[3] = 'm'; buffer[4] = 'e'; buffer[5] = ' '; buffer[6] = 'O'; buffer[7] = 'v'; buffer[8] = 'e'; buffer[9] = 'r'; buffer[10] = '!';
            n = write(newsockfd, buffer, 256);
            if (n < 0) error("ERROR writing to socket");
            break; // Success
        }
        // Executes under no correct flag(incorrect guess). Appends to the incorrect guesses array and increments the attempt value
        if(no_correct == 1) {
            attempt_store[attempts] = buffer[1];
            attempts++;
            if(attempts >= 6) { // Users fails if there has been 6 attempts
                char len = word_len + '0';
                buffer[0] = len;
                for(int i = 1; i < word_len+1; i++) {
                    buffer[i] = word[i-1];
                }
                n = write(newsockfd, buffer, 256);
                if (n < 0) error("ERROR writing to socket");
                buffer[0] = '9';
                buffer[1] = 'Y'; buffer[2] = 'o'; buffer[3] = 'u'; buffer[4] = ' '; buffer[5] = 'L'; buffer[6] = 'o'; buffer[7] = 's'; buffer[8] = 'e'; buffer[9] = '.';
                n = write(newsockfd, buffer, 256);
                if (n < 0) error("ERROR writing to socket");
                buffer[0] = 'a';
                buffer[1] = 'G'; buffer[2] = 'a'; buffer[3] = 'm'; buffer[4] = 'e'; buffer[5] = ' '; buffer[6] = 'O'; buffer[7] = 'v'; buffer[8] = 'e'; buffer[9] = 'r'; buffer[10] = '!';
                n = write(newsockfd, buffer, 256);
                if (n < 0) error("ERROR writing to socket");
                break;
            }
        }
        buffer[0] = '0';
        buffer[1] = word_len;
        buffer[2] = attempts;
        for(int i = 0; i < word_len; i++) {
            buffer[3+i] = guess[i];
        }
        for(int i = 0; i < attempts; i++) {
            buffer[3+word_len+i] = attempt_store[i];
        }
        n = write(newsockfd, buffer, 256);
        if (n < 0) error("ERROR writing to socket");
    }
}

*/