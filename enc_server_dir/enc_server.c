#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include "../usefulFunctions.h"
char* createCypherText(char*);


void dealWithClient(int socket);

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]) {
    int connectionSocket, charsRead;

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
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for connections. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    while (1) {
        for (int i = 0; i < amountOfChildProcesses; i++) {
            int childExitMethod;
            if (waitpid(-1, &childExitMethod, WNOHANG) != 0) {
                amountOfChildProcesses--;
            }
            else {
                break;
            }
        }
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
            //child
            // Get the message from the client and display it
            
            // Read the client's message from the socket
           
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


char* createCypherText(char* buffer) {
    char* cypherText = malloc(strlen(buffer) + 1);
    strcpy(cypherText, buffer);
    buffer += strlen(cypherText) + 1;
    
    char* plainText = malloc(strlen(buffer) + 1);
    strcpy(plainText, buffer);
   
    char* encryptedText = malloc(strlen(plainText) + 1);

    int encryptedTextIndex = 0;
    for (int i = 0; i < strlen(plainText); i++) {
        int currentValue;
        if (plainText[i] == ' ') {
            currentValue = 26;
        }
        else {
            currentValue = plainText[i] - 'A';
          
        }
       
        int addValue;
        if (cypherText[i] == ' ') {
            addValue = 26;
        }
        else {
            addValue = cypherText[i] - 'A';
        
        }
 
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


void dealWithClient(int connectionSocket) {
    char bufferSize[4];     //first 4 bytes to keep track of how long the file is.
    char* returnBuffer;
    char sentFrom[1];
    int amountOfCharsInBuffer = 0;
    char* buffer;
    int bufferIndex = 0;
    char* cypherText;
    char failString[] = { 0,0,0,0 };

    receiveMessage(0, 1, sentFrom, "ERROR reading from socket", connectionSocket, 0);

    bufferIndex = 0;
    if (sentFrom[0] != '1') {
     
        sendMessage(0, 4, failString, "ERROR writing to socket", connectionSocket, 0);
        close(connectionSocket);
        exit(EXIT_FAILURE);
    }

    receiveMessage(0, 4, bufferSize, "ERROR reading from socket", connectionSocket, 0);



    amountOfCharsInBuffer = getBinaryNumber(bufferSize);


    buffer = malloc(sizeof(char) * (amountOfCharsInBuffer));
    memset(buffer, '\0', amountOfCharsInBuffer);
    bufferIndex = 0;


    receiveMessage(0, amountOfCharsInBuffer, buffer, "ERROR reading from socket", connectionSocket, 0);
   
    cypherText = createCypherText(buffer);
    free(buffer);
    amountOfCharsInBuffer = 4 + strlen(cypherText) + 1;
    returnBuffer = createNumberString(amountOfCharsInBuffer - 4);


    buffer = malloc(amountOfCharsInBuffer);
    bufferIndex = 0;
    for (int i = 0; i < 4; i++) {
        buffer[bufferIndex] = returnBuffer[i];
        bufferIndex++;
    }
    free(returnBuffer);

    copyString(buffer, &bufferIndex, cypherText);

    buffer[bufferIndex] = 0;
    
    sendMessage(0, amountOfCharsInBuffer, buffer, "ERROR writing to socket", connectionSocket, 0);
    // Close the connection socket for this client
    close(connectionSocket);
    exit(EXIT_SUCCESS);
}