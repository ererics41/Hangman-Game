# Hangman-Game
Authors: Tiange Wang, Eric Shen

A hangman game implemented in the form of a client-server application over TCP using socket programming in C. A client connects to the server and a word is chosen randomly from the text file hangman_words.txt. The client then guesses letters in the word, too many incorrect guess and you lose. If you guess all the letters in the word you win. The server uses multi-threading to allow for up to 3 clients to play the game at once. 
