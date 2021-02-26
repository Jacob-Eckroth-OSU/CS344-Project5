#include "usefulFunctions.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h> // send(),recv()

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(1);
}



bool checkForBadContents(char* fileContents) {
    for (int i = 0; i < strlen(fileContents); i++) {
        if (fileContents[i] == ' ' || (fileContents[i] >= 'A' && fileContents[i] <= 'Z')) {

        }
        else {

            return true;
        }
    }
    return false;
}


char* getContentsOfFile(char* fileName) {
    FILE* fptr;
    fptr = fopen(fileName, "r");
    char* textContent;
    size_t buffSize = 0;
    if (fptr == NULL) {
        fprintf(stderr, "Error opening input file %s\n", fileName);
        exit(2);
    }
    else {
        getline(&textContent, &buffSize, fptr);

        if (textContent[strlen(textContent) - 1] == 10) {
            textContent[strlen(textContent) - 1] = 0;
        }

    }
    fclose(fptr);
    return textContent;
}


void copyString(char* copyToString, int* copyIndex, char* copyFromString) {
    for (int i = 0; i < strlen(copyFromString); i++) {
        copyToString[*copyIndex] = copyFromString[i];
        (*copyIndex)++;
    }
}

char* createNumberString(int num) {

    char* result = malloc(sizeof(char) * 4);    //4 bytes
    for (int i = 0; i < 4; i++) {
    
        result[i] = getNByte(num, i);
    }
   

    return result;

}

char getNByte(int num, int byteNum) {

    return (num >> (8 * byteNum)) & 0xff;
}

//https://stackoverflow.com/questions/27961054/byte-array-to-decimal
int getBinaryNumber(char* numberString) {
    unsigned char char1 = numberString[0];
    unsigned char char2 = numberString[1];
    unsigned char char3 = numberString[2];
    unsigned char char4 = numberString[3];

    
    return char1 | char2 << 8 | char3 << 16 | char4 << 24;


}


void receiveMessage(int initialIndex, int finalIndex, char* storeLocation, const char* errorMessage,int sockFD,int flags) {
    int charsRead;
    while (initialIndex < finalIndex) {
        charsRead = recv(sockFD, storeLocation + initialIndex, finalIndex - initialIndex, flags);
        if (charsRead < 0) {
            error(errorMessage);
        }
        initialIndex += charsRead;
    }
}


void sendMessage(int initialIndex, int finalIndex, char* sendMessage, const char* errorMessage, int sockFD, int flags) {
    int charsRead;
    while (initialIndex < finalIndex) {
        charsRead = send(sockFD, sendMessage + initialIndex, finalIndex - initialIndex, flags);
        if (charsRead < 0) {
            error(errorMessage);
        }
        initialIndex += charsRead;
    }

}