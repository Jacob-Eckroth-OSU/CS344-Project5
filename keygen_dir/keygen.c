/*
Name: Jacob Eckroth
Date: 3/8/2021
Project Name: Assignment 5: One-Time Pads
Description: This program generates a random sequence of capital characters and spaces and outputs it to stdout.
**			the amount of characters is decided by the first commandline argument. All errors are handled.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <assert.h>

int getLength(int, char**);

bool isPositiveNumber(char*);
void outputKey(int length);


int main(int argc, char**argv) {
	srand(time(NULL));
	int length = getLength(argc, argv);
	if (length <= 0) {
		fprintf(stderr, "Please execute the program like this: 'keygen keylength' where keylength is a positive integer\n");
	}
	else {
		outputKey(length);
	}
	
	return 0;
}

/*
** Description: Converts the 2nd command line argument into an integer, returns -1 if invalid
** Prerequisites: Argc and argv are correlated.
** Updated/Returned: Returns the 2nd command line argument converted into an integer. Returns -1 if invalid.
*/
int getLength(int argc, char** argv) {

	//only run program with only 2 command lines
	if (argc != 2) {
		return -1;
	}
	else {
		if (isPositiveNumber(argv[1])) {
			return atoi(argv[1]);
		}
	}

	return -1;
}

/*
** Description: returns true if a string is a integer, false otherwise.
** Prerequisites: string is allocated.
** Updated/Returned: Returns true if the string is a positive integer, false otherwise.
*/
bool isPositiveNumber(char* string) {
	assert(string);
	if (strlen(string) == 0) {
		return false;
	}
	for (int i = 0; i < strlen(string); i++) {
		//checking if each character is in the right range.
		if (!(string[i] - '0' >= 0 && string[i] - '0' <= 9) ){
			return false;
		}
	}

	return true;
}


/*
** Description: Outputs a random key to stdout of the lenght of "length of key"
** Prerequisites: None
** Updated/Returned: Outputs random capital characters or spaces, amount based on the length of the key.
*/
void outputKey(int lengthOfKey) {
	char outputChar;
	for (int i = 0; i < lengthOfKey; i++) {
		int choice = rand() % 27;
		//spaces have equal chances as everything else
		if (choice == 26) {
			outputChar = ' ';
		}
		else {
			outputChar = 'A' + choice;
		}
		putchar(outputChar);
	}
	putchar('\n');
}