#include "usefulFunctions.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <assert.h>

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(1);
}


/*
** Description: Returns true if there are bad characters in the string. False otherwise.
** Prerequisites:fileContents is allocated
** Updated/Returned: true or false, depending if there are bad characters
*/
bool checkForBadContents(char* fileContents) {
    assert(fileContents);
    for (int i = 0; i < strlen(fileContents); i++) {
        if (fileContents[i] == ' ' || (fileContents[i] >= 'A' && fileContents[i] <= 'Z')) {

        }
        else {

            return true;
        }
    }
    return false;
}

/*
** Description: Returns one line of a file.
** Prerequisites:None
** Updated/Returned: Returns a string containing the contents of a file. Removes the newline from the end if there is one.
*/
char* getContentsOfFile(char* fileName) {
    assert(fileName);
    FILE* fptr;
    fptr = fopen(fileName, "r");
    char* textContent;
    size_t buffSize = 0;

    //if we can't open a file
    if (fptr == NULL) {
        fprintf(stderr, "Error opening input file %s\n", fileName);
        exit(2);
    }
    else {
        getline(&textContent, &buffSize, fptr);

        //replacing the newline in the string.
        if (textContent[strlen(textContent) - 1] == 10) {
            textContent[strlen(textContent) - 1] = 0;
        }

    }
    fclose(fptr);
    return textContent;
}

/*
** Description: Copies a destination string into a target string
** Prerequisites: Strings are allocated
** Updated/Returned: Copies destination string into target string at copyIndex in the copy string.
*/
void copyString(char* copyToString, int* copyIndex, char* copyFromString) {
    assert(copyToString);
    assert(copyFromString);
    assert(copyIndex);
    for (int i = 0; i < strlen(copyFromString); i++) {
        copyToString[*copyIndex] = copyFromString[i];
        (*copyIndex)++;
    }
}


/*
** Description: Creates a binary number string from an int num
** Prerequisites: None
** Updated/Returned: Returns a binary string from the int num.
*/
char* createNumberString(int num) {

    char* result = malloc(sizeof(char) * 4);    //4 bytes
    for (int i = 0; i < 4; i++) {
    
        result[i] = getNByte(num, i);
    }
   

    return result;

}


/*
** Description: Gets the Nth byte from an integer.
** Prerequisites: None
** Updated/Returned: Returns the Nth byte of num.
*/
char getNByte(int num, int byteNum) {

    return (num >> (8 * byteNum)) & 0xff;
}


/*
** Description: returns a 32 bit number constructed from 4 char bytes.
** Prerequisites: numberString is allocated
** Updated/Returned: returns an int based on the binary number
*/
int getBinaryNumber(char* numberString) {
    assert(numberString);
    //we cast them to unsigned chars because they're stored as signed chars. The math works this way.
    unsigned char char1 = numberString[0];
    unsigned char char2 = numberString[1];
    unsigned char char3 = numberString[2];
    unsigned char char4 = numberString[3];

    //creates a number from 4 bytes.
    return char1 | char2 << 8 | char3 << 16 | char4 << 24;


}

/*
** Description: Receives a message through a socket, of a given length.
** Prerequisites: error message is allocated, message is being received
** Updated/Returned: message is received and stored in the storelLocation
*/
void receiveMessage(int initialIndex, int finalIndex, char* storeLocation, const char* errorMessage,int sockFD,int flags) {
    assert(storeLocation);
    assert(errorMessage);
    int charsRead;
    while (initialIndex < finalIndex) {
        charsRead = recv(sockFD, storeLocation + initialIndex, finalIndex - initialIndex, flags);
        if (charsRead < 0) {
            error(errorMessage);
        }
        initialIndex += charsRead;
    }
}

/*
** Description: Sends a message through a socket, of a given length.
** Prerequisites: error message is allocated, message being sent is allocated
** Updated/Returned: message is sent
*/
void sendMessage(int initialIndex, int finalIndex, char* sendMessage, const char* errorMessage, int sockFD, int flags) {
    assert(errorMessage);
    assert(sendMessage);
    int charsRead;
    while (initialIndex < finalIndex) {
        charsRead = send(sockFD, sendMessage + initialIndex, finalIndex - initialIndex, flags);
        if (charsRead < 0) {
            error(errorMessage);
        }
        initialIndex += charsRead;
    }

}




// Set up the address struct
void setupAddressStructClient(struct sockaddr_in* address, int portNumber, char* hostname) {
    assert(address);
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

// Set up the address struct for the server socket
void setupAddressStructServer(struct sockaddr_in* address, int portNumber) {
    assert(address);
    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}


/*
** Description: Checks and ends any zombie processes
** Prerequisites: None
** Updated/Returned: ends any child zombie processes.
*/
void checkForZombies(int *childProcesses) {
    assert(childProcesses);
    for (int i = 0; i < *childProcesses; i++) {
        int childExitMethod;
        if (waitpid(-1, &childExitMethod, WNOHANG) != 0) {
            (*childProcesses)--;
        }
        else {
            break; //if we don't find a zombie at first we can just exit
        }
    }
}