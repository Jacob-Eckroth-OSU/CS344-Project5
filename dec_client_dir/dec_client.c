/*
Name: Jacob Eckroth
Date: 3/8/2021
Project Name: Assignment 4: Multi-threaded Producer Consumer Pipeline
Description: This process takes input from the user in the form of ./dec_client encryptedtext key port
**  It attempts to communicate with an dec_server running on that port and sends the data and key to it
**  to do the decrypting. it then receives and prints out the decrypted text to stdout.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "../usefulFunctions.h"
#include <assert.h>

char* initializeSendBuffer(char* fileName, char* keyFileName, int*);


/*
** Description: This is the main function.... it runs the program... that's it
** Prerequisites: command line arguments: ./enc_client encryptedtext key port
** Updated/Returned: this is the main function of a program. Everything is very clearly well named. You got this I believe in you
*/
int main(int argc, char* argv[]) {
    int socketFD;
    struct sockaddr_in serverAddress;

    // Check usage & args
    if (argc < 4) {
        fprintf(stderr, "USAGE: %s encryptedText key port\n", argv[0]);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }
    char* fileName = argv[1];
    char* keyFileName = argv[2];
    // Set up the server address struct
    setupAddressStructClient(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    int bufferLength;

    //initializes the buffer we will send to the server.
    char* Fullbuffer = initializeSendBuffer(fileName, keyFileName, &bufferLength);


    sendMessage(0, bufferLength, Fullbuffer, "CLIENT: ERROR writing to socket", socketFD, 0);


    char dataReceivingLength[4];
    receiveMessage(0, 4, dataReceivingLength, "CLIENT: ERROR reading from socket", socketFD, 0);
    int amountOfCharacters = getBinaryNumber(dataReceivingLength);

    if (amountOfCharacters == 0) {
        fprintf(stderr, "Error: Could not contact dec_server on port %d\n", atoi(argv[3]));
        exit(2);
    }
    else {
        char* decryptedText = malloc(amountOfCharacters);
        receiveMessage(0, amountOfCharacters, decryptedText, "CLIENT: ERROR reading from socket", socketFD, 0);
        printf("%s\n", decryptedText);

        free(decryptedText);
    }


    // Close the socket
    close(socketFD);
    return 0;
}


/*
** Description: Initializes and returns the buffer to be sent to the server based on the 2 files
** Prerequisites: fileName and keyFileName are allocated
** Updated/Returned: Returns the buffer to be sent to the user
*/

//1st byte of buffer is '2' to indicate it's from dec_client. Next 4 bytes are binary number of bytes in the rest of the buffer.
//then the text from the cypher Key, then a null terminator, then the encrypted text, then a null terminator.
char* initializeSendBuffer(char* fileName, char* keyFileName, int* bufferLength) {
    assert(fileName);
    assert(keyFileName);
    assert(bufferLength);


    char* encryptedTextContents = getContentsOfFile(fileName);
    char* cypherContents = getContentsOfFile(keyFileName);
   

    //check for bad characters in both files, and check if the key is shorter than the text.
    if (checkForBadContents(cypherContents)) {
        fprintf(stderr, "Bad Contents in %s. Use capital characters A-Z and spaces only.\n",keyFileName);
        exit(1);
    }
    else if (checkForBadContents(encryptedTextContents)) {
        fprintf(stderr, "Bad Contents in %s. Use capital characters A-Z and spaces only.\n",fileName);
        exit(1);
    }
    else if (strlen(cypherContents) < strlen(encryptedTextContents)) {
        fprintf(stderr, "Error, key '%s' is too short\n",keyFileName);
        exit(1);
    }

    int lengthOfMessage = 1 + 4 + strlen(cypherContents) + 1 + strlen(encryptedTextContents) + 1;
    //plus 8 because first 8 bytes are used for number storage, plus 2 because there's a null terminator after the strings.
    //first byte is a 1 if it's enc.
    char* newMessage = malloc(sizeof(char) * lengthOfMessage);


    char* numberStr = createNumberString(lengthOfMessage - 5); //-5 because the first byte indicates decode/encode and the next 4 are the number of bytes in binary

   

    int newMessageIndex = 0;

    //initializing the first byte to 2 so it knows that we're decrypting.
    newMessage[newMessageIndex++] = '2';
    for (int i = 0; i < 4; i++) {
        newMessage[newMessageIndex] = numberStr[i];
        newMessageIndex++;
    }
    free(numberStr);
    

    //Copying the number of bytes of the actual message.
    copyString(newMessage, &newMessageIndex, numberStr);

    //copying the cypher text into the message
    copyString(newMessage, &newMessageIndex, cypherContents);

    //adding null terminating byte in between strings
    newMessage[newMessageIndex++] = 0;

    //copying encrypted text into message.
    copyString(newMessage, &newMessageIndex, encryptedTextContents);

    //adding final null terminating byte
    newMessage[newMessageIndex] = 0;

    *bufferLength = lengthOfMessage;
    free(encryptedTextContents);
    free(cypherContents);
    return newMessage;
}
