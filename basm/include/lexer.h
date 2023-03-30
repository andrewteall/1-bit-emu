#ifndef LEXER_H
#define LEXER_H 1

#include "basm.h"

#ifndef MAX_FILE_INCLUDES
	#define MAX_FILE_INCLUDES 64
#endif

#ifndef MAX_NUM_TOKENS
	#define MAX_NUM_TOKENS 16384
#endif


struct FILE_TABLE {
	char table[MAX_FILE_INCLUDES][PATH_MAX];
	int parentIdx[MAX_FILE_INCLUDES];
	int length;
};


/**
 * Opens and Reads a file, filename, tokenizes all elements within, and 
 * stores the tokens in tokenBuffer.
 * @param options Configuration for the lexer.
 * @param sTokenArray Array used to store tokens read from file.
 * @param sFileTable Array to store filenames in as they are encountered.
 * @param tokenArrayLength Index to put the next token generated.
 
 * @returns int Size of the Token Buffer.
 **/
int tokenizeFile(struct OPTIONS* sOptions, struct TOKEN* sTokenArray, struct FILE_TABLE* sFileTable, int* tokenArrayLength);


/**
 * Determines whether or not a string is a Mnemonic.
 *
 * @param tokenStr The string to determine if it is a Mnemonic value.
 * @returns int 1 if tokenStr is a mnemonic 0 if tokenStr is not.
 **/
int containsMatch(const char* arrayToMatch[],char* tokenStr);
int isMnenomic(char* tokenStr);
int isNumber(char* tokenStr);
int isAssignment(char* tokenStr);
int isInclude(char* tokenStr);
int isDirective(char* tokenStr);
int isLabelMod(char* tokenStr);

int isTokenDelimeter(int c);
int isIncludeStatement(struct TOKEN* sTokenArray, int tokenArrayLength);

void printFileTable(struct FILE_TABLE* fileTable);
int openFile(char* filename, FILE** sourceFile);
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
 * @param options Configuration for the lexer.
 * @param fileTable[][PATH_MAX] 2-D Array to hold Filenames.
 * @param includedFile Pointer to the file to obtain the full path from.
 * @param line Current line number the lexer is on.
 * @param fileIdx Current file table index of the file
 * @returns it 0 if successful 1 if there was an error.
 **/
int addFileToFileTable(struct OPTIONS* sOptions, struct FILE_TABLE* sFileTable, char** includedFile, int line, int fileIdx);

#endif