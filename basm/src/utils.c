#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************/
/*********************************** Utils ************************************/
/******************************************************************************/
/* Makes a String uppercase */
char* toUpperString(char* string){
	for(int i=0; i<strlen(string);i++){
		string[i] = toupper(string[i]);
	}
	return string;
}

/* Round a float up and return an int */
int roundUp(float floatNumber){
	int intNumber = (int)floatNumber;
	if(intNumber < floatNumber){
		intNumber++;
	}
	return intNumber;
}

/* Converts a number string to a number */
int str2num(char *numStr){
	char* indexPtr;
	int index = 0;
	int base = 10;

	if ((numStr[0] == '0' && (numStr[1] == 'x' || numStr[1] == 'X')) || numStr[0] == '$'){ 
		// convert hexidecimal number
		index = 2;
		if(numStr[0] == '$'){
			index = 1;
		}
		base = 16;
	} else if ((numStr[0] == '0' && (numStr[1] == 'b' || numStr[1] == 'B')) || numStr[0] == '%'){
		// convert binary number
		index = 2;
		if(numStr[0] == '%'){
			index = 1;
		}
		base = 2;
	}

	long num = strtol(numStr+index, &indexPtr, base);
	if (*indexPtr != '\0' || num < 0){
		num = -1;
	}
	return (int)num;
}
