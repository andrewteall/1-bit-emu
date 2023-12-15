#ifndef LEXER_H
#define LEXER_H 1

#include "token.h"

#ifndef MAX_FILE_INCLUDES
	#define MAX_FILE_INCLUDES 64
#endif

struct TOKENIZER_CONFIG {
	int includedFileDepth;
	int maxFileDepth;
};

struct FILE_TABLE {
	char table[MAX_FILE_INCLUDES][4096]; // Change back to PATH_MAX
	int parentIdx[MAX_FILE_INCLUDES];
	int length;
};


/**
 * Opens and Reads a file, filename, tokenizes all elements within, and 
 * stores the tokens in tokenBuffer.
 * @param tokenizerConfig Configuration for the lexer.
 * @param sTokenArray Array used to store tokens read from file.
 * @param sFileTable Array to store filenames in as they are encountered.
 * @param tokenArrayLength Index to put the next token generated.
 
 * @returns int Size of the Token Buffer.
 **/
int tokenizeFile(char* filename, struct TOKEN* sTokenArray, int maxIncludeFileDepth);



#endif