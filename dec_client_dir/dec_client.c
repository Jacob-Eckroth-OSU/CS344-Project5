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

char* initializeSendBuffer(char* fileName, char* keyFileName, int*);


/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/



// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber,
    char* hostname) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char* argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
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
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    int bufferLength;

    char* Fullbuffer = initializeSendBuffer(fileName, keyFileName, &bufferLength);


    // Send message to server
    // Write to the server
   

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


char* initializeSendBuffer(char* fileName, char* keyFileName, int* bufferLength) {



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
