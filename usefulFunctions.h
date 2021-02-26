#ifndef USEFUL_H
#define USEFUL_H
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
bool checkForBadContents(char* fileContents);
char* getContentsOfFile(char* fileName);
char* createNumberString(int number);
char getNByte(int num, int byteNum);
void copyString(char* copyToString, int* index, char* copyString);

void receiveMessage(int initialIndex, int finalIndex, char* storeLocation, const char* errorMessage, int sockFD, int flags);
void sendMessage(int initialIndex, int finalIndex, char* sendMessage, const char* errorMessage, int sockFD, int flags);
int getBinaryNumber(char*);


void setupAddressStructClient(struct sockaddr_in* address, int portNumber, char* hostName);

void setupAddressStructServer(struct sockaddr_in* address, int portNumber);

void error(const char*);


void checkForZombies(int *childProcesses);

#endif