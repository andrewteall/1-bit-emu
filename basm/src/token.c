#include <stdio.h>
#include <string.h>

#include "token.h"
#include "ulog.h"


const char *tokenTypeStrings[] = {  "MNENOMIC", \
									"ASSIGNMENT", \
									"INCLUDE", \
									"DIRECTIVE", \
									"FLAG", \
									"NUMBER", \
									"NEWLINE", \
									"LABEL_MOD", \
									"WHITESPACE", \
									"COMMENT", \
									"LABEL"};

void printTokens(struct TOKEN_LIST* tokenList){
	printf("Printing Token List:\n");
	for(int i=0; i < tokenList->numTokens;i++){
		printf("%02i: File: %s:%i \t| Type: %-10s | Address: 0x%04x | Value: 0x%02x | Size: %i | Lexeme: %s\n", \
		i,tokenList->list[i].filename,tokenList->list[i].lineNumber, tokenTypeStrings[tokenList->list[i].type], \
		tokenList->list[i].address, tokenList->list[i].numericValue, tokenList->list[i].size, tokenList->list[i].stringValue );
	}
}

int addNewToken(struct TOKEN_LIST* tokenList, char* tokenStr, int type, char* filename, int* lineNumber){
	ulog(TRACE,"Adding token string: %s", tokenStr);
	if(tokenList->numTokens == MAX_NUM_TOKENS){
		ulog(ERROR,"Maximum number of tokens exceeded: %i in file %s:%i", tokenList->numTokens, filename, *lineNumber);
		return 1;
	} else {
		tokenList->nextToken->type = type;
		strcpy(tokenList->nextToken->stringValue, tokenStr);
		tokenList->nextToken->filename = filename;
		tokenList->nextToken->lineNumber = *lineNumber;
		
		tokenList->numTokens++;
		tokenList->nextToken = &tokenList->list[tokenList->numTokens];
		if(type == NEWLINE){
			(*lineNumber)++;
		}

		return 0;
	}
}