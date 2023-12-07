#include <stdio.h>

#include "token.h"


const char *tokenTypeStrings[] = {  "MNENOMIC", \
									"ASSIGNMENT", \
									"INCLUDE", \
									"DIRECTIVE", \
									"FLAG", \
									"NUMBER", \
									"NEWLINE", \
									"LABEL_MOD", \
									"LABEL"};


void printTokens(struct TOKEN* sTokenArray, int sTokenArrayLength){
	printf("Printing Token List:\n");
	for(int i=0; i < sTokenArrayLength;i++){
		printf("%02i: File: %s:%i \t| Type: %-10s | Address: 0x%04x | Value: 0x%02x | Size: %i | Lexeme: %s\n", \
		i,sTokenArray[i].filename,sTokenArray[i].lineNumber, tokenTypeStrings[sTokenArray[i].type], \
		sTokenArray[i].address, sTokenArray[i].numericValue, sTokenArray[i].size, sTokenArray[i].stringValue );
	}
}