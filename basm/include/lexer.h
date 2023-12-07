#ifndef LEXER_H
#define LEXER_H 1

#include "token.h"

#ifndef MAX_FILE_INCLUDES
	#define MAX_FILE_INCLUDES 64
#endif

#ifndef MAX_NUM_TOKENS
	#define MAX_NUM_TOKENS 16384
#endif


struct TOKENIZER_CONFIG {
	int includedFileDepth;
	int maxFileDepth;
	char* filename;
	int onlyTokenize;
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
int tokenizer(struct TOKENIZER_CONFIG* tokenizerConfig, struct TOKEN* sTokenArray, struct FILE_TABLE* sFileTable, int* tokenArrayLength);
int tokenizeFile(struct TOKENIZER_CONFIG* tokenizerConfig, char* filename, struct TOKEN* sTokenArray);

void printFileTable(struct FILE_TABLE* fileTable);
char* getCurrentFilename(struct FILE_TABLE* sFileTable, int index);


/**
 * Determines the token type of tokenStr.
 *
 * @param tokenStr Pointer to the string to determine what type of token.
 * @returns int The type of token from tokenType enum.
 **/
int determineTokenType(char* tokenStr);

/**
 * Adds a Token to an Array.
 *
 * @param tokenArray Array to hold tokens.
 * @param tokenArrayIndex Index to place the next Token in the Array.
 * @param tokenStr The string value of the token.
 * @param filename The name of the file that contains the Token.
 * @param lineNumber The line number the token is found on.
 * @returns int error
 **/
int addNewTokenToArray(struct TOKEN* tokenArray, int* tokenArrayIndex, char* tokenStr, char* filename, int* lineNumber);

/**
 * Clears a tokenStrBuffer Array and sets it's tokenStrBufferLength to 0.
 *
 * @param tokenStrBuffer Pointer to the Char Array to clear.
 * @param tokenStrBufferLength The length of the tokenStr to clear.
 * @returns int - The length of the cleared tokenStrBuffer.
 **/
int clearTokenStringBuffer(char* tokenStrBuffer, int tokenStrBufferLength);

/**
 * Determines the full path of a file, from includedFile and adds the 
 * fullpath of includedFile to the file table
 *
 * @param tokenizerConfig Configuration for the lexer.
 * @param fileTable[][PATH_MAX] 2-D Array to hold Filenames.
 * @param includedFile Pointer to the file to obtain the full path from.
 * @param line Current line number the lexer is on.
 * @param fileIdx Current file table index of the file
 * @returns it 0 if successful 1 if there was an error.
 **/
int addFileToFileTable(struct TOKENIZER_CONFIG* tokenizerConfig, struct FILE_TABLE* sFileTable, char** includedFile, int line, int fileIdx);

#endif