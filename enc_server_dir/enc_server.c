/*
Name: Jacob Eckroth
Date: 3/8/2021
Project Name: Assignment 4: Multi-threaded Producer Consumer Pipeline
Description: This process runs a server that takes in decrypted text and a key from the user
** and then sends back encrypted text.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include "../usefulFunctions.h"
#include <assert.h>
char* createCypherText(char*);


void dealWithClient(int socket);


/*
** Description: This is the main function.... it runs the program... that's it
** Prerequisites: command line arguments: ./enc_server port
** Updated/Returned: this is the main function of a program. Everything is very clearly well named. You got this I believe in you
*/
int main(int argc, char* argv[]) {
    int connectionSocket;

    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    int amountOfChildProcesses = 0;

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStructServer(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
        exit(1);
    }

    // Start listening for connections. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    while (1) {
        checkForZombies(&amountOfChildProcesses);
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
            (struct sockaddr*)&clientAddress,
            &sizeOfClientInfo);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }
        pid_t currentPID = fork();
        
        switch (currentPID) {

        case -1:
            perror("Hull Breach in creating a process!");
            exit(1);
            break;

        case 0:   
            dealWithClient(connectionSocket);
            break;
        default:
            amountOfChildProcesses++;
            //parent
            break;

        }

       
    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}


/*
** Description: Returns encrypted text using a buffer that starts at the cypher key.
** Prerequisites: buffer is allocated
** Updated/Returned: returns plain text encrypted using the key.
*/
char* createCypherText(char* buffer) {
    assert(buffer);
    //get key
    char* cypherText = malloc(strlen(buffer) + 1);
    strcpy(cypherText, buffer);
    buffer += strlen(cypherText) + 1;
    
    //get plaintext
    char* plainText = malloc(strlen(buffer) + 1);
    strcpy(plainText, buffer);
   
    char* encryptedText = malloc(strlen(plainText) + 1);

    int encryptedTextIndex = 0;
    for (int i = 0; i < strlen(plainText); i++) {
        int currentValue;

        //get current value in plain text
        if (plainText[i] == ' ') {
            currentValue = 26;
        }
        else {
            currentValue = plainText[i] - 'A';
          
        }
       
        int addValue;
        //get add value from cypher key
        if (cypherText[i] == ' ') {
            addValue = 26;
        }
        else {
            addValue = cypherText[i] - 'A';
        
        }
 
        //calculates the new value for the return text
        int totalValue = (currentValue + addValue) % 27;
        if (totalValue == 26) {
            encryptedText[encryptedTextIndex] = ' ';
        }
        else {
            encryptedText[encryptedTextIndex] = 'A' + totalValue;
        }
        encryptedTextIndex++;
        

    }
    encryptedText[encryptedTextIndex] = 0;
    
   
    free(cypherText);
    free(plainText);

    
    return encryptedText;



}

/*
** Description: Handles receiving and sending data to a client over a connection socket
** Prerequisites: None
** Updated/Returned: Sends client encrypted data back.
*/
void dealWithClient(int connectionSocket) {
    char bufferSize[4];     //first 4 bytes to keep track of how long the file is.
    char* returnBuffer;
    char sentFrom[1];
    int amountOfCharsInBuffer = 0;
    char* buffer;
    int bufferIndex = 0;
    char* cypherText;
    char failString[] = { 0,0,0,0 };

    //receives the first byte indicating where the message is sent from
    receiveMessage(0, 1, sentFrom, "ERROR reading from socket", connectionSocket, 0);

    //the first byte must be a '1' for it to be from an encrypted client
    if (sentFrom[0] != '1') {
        //sends a message of {0,0,0,0} if it's incorrect
        sendMessage(0, 4, failString, "ERROR writing to socket", connectionSocket, 0);
        close(connectionSocket);
        exit(EXIT_FAILURE);
    }

    //receives the size of the buffer message
    receiveMessage(0, 4, bufferSize, "ERROR reading from socket", connectionSocket, 0);


    //allocates memory for the buffer sent by the client
    amountOfCharsInBuffer = getBinaryNumber(bufferSize);
    buffer = malloc(sizeof(char) * (amountOfCharsInBuffer));
    memset(buffer, '\0', amountOfCharsInBuffer);


    //receives the buffer from the client
    receiveMessage(0, amountOfCharsInBuffer, buffer, "ERROR reading from socket", connectionSocket, 0);
   

    //cypers the text
    cypherText = createCypherText(buffer);
    free(buffer);

    //creates the string with the amount of bytes in the buffer -4 (-4 because 4 bytes ARE the string)
    amountOfCharsInBuffer = 4 + strlen(cypherText) + 1;
    returnBuffer = createNumberString(amountOfCharsInBuffer - 4);
    buffer = malloc(amountOfCharsInBuffer);

    //write the amount of bytes into the buffer
    bufferIndex = 0;
    for (int i = 0; i < 4; i++) {
        buffer[bufferIndex] = returnBuffer[i];
        bufferIndex++;
    }
    free(returnBuffer);

    //writes the text into the buffer.
    copyString(buffer, &bufferIndex, cypherText);

    buffer[bufferIndex] = 0;
    
    //sends the buffer
    sendMessage(0, amountOfCharsInBuffer, buffer, "ERROR writing to socket", connectionSocket, 0);
    // Close the connection socket for this client
    close(connectionSocket);
    exit(EXIT_SUCCESS);
}
