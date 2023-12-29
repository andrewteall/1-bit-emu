#ifndef LEXER_H
#define LEXER_H 1

#include "token.h"

#ifndef MAX_FILE_INCLUDES
	#define MAX_FILE_INCLUDES 64
#endif

struct FILE_TABLE {
	char table[MAX_FILE_INCLUDES][4096]; // Change back to PATH_MAX
	int parentIdx[MAX_FILE_INCLUDES];
	int length;
};

struct TOKENIZER_CONFIG {
	struct FILE_TABLE fileTable;
};

void printFileTable(struct FILE_TABLE* sFileTable);

/**
 * Opens and Reads a file, filename, tokenizes all elements within, and 
 * stores the tokens in tokenList.
 * @param filename The filename to be tokenized.
 * @param tokenList Token_List struct to store tokens read from file.
 * @param tokenizerConfig A Pointer to the tokenizer configuration.
 * @returns int The number of tokens read from the file.
 **/
int tokenizeFile(char* filename, struct TOKEN_LIST* tokenList, struct TOKENIZER_CONFIG* tokenizerConfig);



#endif