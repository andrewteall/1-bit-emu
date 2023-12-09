#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"
#include "token.h"
#include "../../ulog/include/ulog.h"

const int delimeterList[] = {' ', '\t', '\n', EOF, ':','=','+', '-', '*', 0};

const char *mnenomicStrings[] = {
			"NOPO","LD","LDC","AND","ANDC","OR","ORC","XNOR","STO","STOC","IEN","OEN","JMP","RTN","SKZ","NOPF","END"};
const char *assignmentStrings[] = {"=","EQU","END"};
const char *includeStrings[] = {"INCLUDE","END"};
const char *directiveStrings[] = {"ORG","REMAP","SUB", "END_S","SUBROUTINE","END_SUBROUTINE","REP","REPEAT","REPEND","END"};
const char *labelModStrings[] = {"+","-","*","END"};

int isTokenDelimeter(int c){
	for(int i=0; delimeterList[i] != 0;i++){
		if(c == delimeterList[i]){
			return 1;
		}
	}
	return 0;
}

int isIncludeStatement(struct TOKEN* sTokenArray, int sTokenArrayLength){
	return (sTokenArray[sTokenArrayLength-3].type == INCLUDE && sTokenArray[sTokenArrayLength-1].type == NEWLINE);
}

int containsMatch(const char* arrayToMatch[],char* tokenStr){
	toUpperString(tokenStr);
	for(int index = 0;strcmp("END",arrayToMatch[index]);index++){
		if (!strcmp(tokenStr,arrayToMatch[index])){
			return 1;
		}
	}
	return 0;
}

int isNumber(char* tokenStr){
	if ((tokenStr[0] == '0' && (tokenStr[1] == 'x' || tokenStr[1] == 'X')) || tokenStr[0] == '$'){
		int startPos = 2;
		if(tokenStr[0] == '$'){
			startPos = 1;
		}

		for (int j = startPos; tokenStr[j] != '\0' ; j++){
			if(!(((tokenStr[j] >= 'A') && (tokenStr[j] <= 'F')) || \
			 ((tokenStr[j] >= 'a') && (tokenStr[j] <= 'f')) || \
			 ((tokenStr[j] >= '0') && (tokenStr[j] <= '9')))){
				return 0;
			}
		}

	} else if ((tokenStr[0] == '0' && (tokenStr[1] == 'b' || tokenStr[1] == 'B')) || tokenStr[0] == '%'){
		int startPos = 2;
		if(tokenStr[0] == '%'){
			startPos = 1;
		}
		for (int j = startPos; tokenStr[j] != 0 ; j++){
			if(tokenStr[j] != '1' && tokenStr[j] != '0' ){
				return 0;
			}
		}
	} else {
		for (int j = 0; tokenStr[j] != 0 ; j++){
			if(tokenStr[j] < '0' || tokenStr[j] > '9'   ){
				return 0;
			}
		}
	}
	return 1;
}

int determineTokenType(char* tokenStr){
	int tokenType;
	char tokenTmp[strlen(tokenStr)];
	strcpy(tokenTmp,tokenStr);
	
	if(containsMatch(mnenomicStrings,tokenStr)){
		tokenType = MNENOMIC;
	} else if(containsMatch(assignmentStrings,tokenStr)){
		tokenType = ASSIGNMENT;
	} else if(containsMatch(includeStrings,tokenStr)){
		tokenType = INCLUDE;
	} else if(containsMatch(directiveStrings,tokenStr)){
		tokenType = DIRECTIVE;
	} else if(isNumber(tokenStr)){
		tokenType = NUMBER;
	} else if(*tokenStr == '\n' || !strcmp(tokenStr,"\\N") || (tokenStr[0] == 255 && tokenStr[255])){
		tokenType = NEWLINE;
	} else if(containsMatch(labelModStrings,tokenStr)){
		strcpy(tokenStr,tokenTmp);
		tokenType = LABEL_MOD;
	} else {
		strcpy(tokenStr,tokenTmp);
		tokenType = LABEL;
	} 

	return tokenType;
}

void printFileTable(struct FILE_TABLE* sFileTable){
	printf("================================= File Table ===================================\n");
	printf("| %s %28s %44s\n","Idx","Filename","Parent Idx");
	for(int i=0; i<sFileTable->length; i++){
		printf("| %2i: %32s %*i\n",i,sFileTable->table[i],(int)(70-strlen(sFileTable->table[i])),sFileTable->parentIdx[i]);
	}
	printf("================================================================================\n\n");
}

int hasCircularFileInclude(struct FILE_TABLE* sFileTable){
	int idx = sFileTable->parentIdx[sFileTable->length-1];
	while(idx != -1){
		if(!strcmp(sFileTable->table[sFileTable->length-1],sFileTable->table[idx])){
			return 1;
		}
		idx = sFileTable->parentIdx[idx];
	}
	return 0;
}

int addFileToFileTable(struct TOKENIZER_CONFIG* tokenizerConfig, struct FILE_TABLE* sFileTable,char** includedFile, \
								int line,int fileIdx){
	if (sFileTable->length == MAX_FILE_INCLUDES){
		ulog(ERROR,"Too many includes: %i Last Include: %s:%i",MAX_FILE_INCLUDES, \
						sFileTable->table[sFileTable->parentIdx[sFileTable->length]-1],line-1);
		return 1;
	} else if(tokenizerConfig->includedFileDepth == tokenizerConfig->maxFileDepth){
		ulog(ERROR,"Too many nested includes %i: %s:%i  You can override this value with the -m flag", \
						tokenizerConfig->maxFileDepth,sFileTable->table[fileIdx-1],line-1);
		return 1;
	} else {
		char* sourceFullPathFilename = sFileTable->table[sFileTable->length];
		if(sFileTable->length == 0){
			realpath(*includedFile,sourceFullPathFilename);
		} else {
			sourceFullPathFilename = realpath(sFileTable->table[fileIdx-1], sourceFullPathFilename);
			char* includedFilePtr = *includedFile;
			do {
				for(int i=strlen(sourceFullPathFilename)-1;sourceFullPathFilename[i] != '/'; i--){
					sourceFullPathFilename[i] = '\0';
				}
				if(includedFilePtr[0] == '.' && includedFilePtr[1] == '.' && includedFilePtr[2] == '/'){
					sourceFullPathFilename[strlen(sourceFullPathFilename)-2] = '\0';
					for(int i=0;i != strlen(*includedFile)+1; i++){
						includedFilePtr[i] = includedFilePtr[i+3];
						includedFilePtr[i+3] = '\0';
					}
				}
			} while(includedFilePtr[0] == '.' && includedFilePtr[1] == '.' && includedFilePtr[2] == '/');
			strcat(sFileTable->table[sFileTable->length],*includedFile);
		}
		ulog(DEBUG,"Adding File to FileTable: %s at index %i",sourceFullPathFilename,sFileTable->length);
		sFileTable->parentIdx[sFileTable->length] = fileIdx-1;
		(sFileTable->length)++;

		tokenizerConfig->includedFileDepth++;
		*includedFile = sourceFullPathFilename;
		if(hasCircularFileInclude(sFileTable)){
			ulog(DEBUG,"File Table Length: %i  Included File Depth: %i",sFileTable->length,tokenizerConfig->includedFileDepth);
			printFileTable(sFileTable);
			ulog(ERROR,"Circular Include in file: %s:%i",sFileTable->table[sFileTable->parentIdx[sFileTable->length-1]],line-1);
			
			return 1;
		}
		return 0;
	}
}

int addNewTokenToArray(struct TOKEN* sTokenArray, int* sTokenArrayLength, char* tokenStr, char* filename, int* lineNumber){
	if(*sTokenArrayLength == MAX_NUM_TOKENS){
		ulog(ERROR,"Maximum number of tokens exceeded: %i in file %s:%i", *sTokenArrayLength, filename, *lineNumber);
		return 1;
	} else {
		sTokenArray[*sTokenArrayLength].type = determineTokenType(tokenStr);
		strcpy(sTokenArray[*sTokenArrayLength].stringValue, tokenStr);
		sTokenArray[*sTokenArrayLength].filename = filename;
		sTokenArray[*sTokenArrayLength].lineNumber = *lineNumber;
		
		if(sTokenArray[*sTokenArrayLength].type == NEWLINE){
			(*lineNumber)++;
		}
		(*sTokenArrayLength)++;

		return 0;
	}
}

int addCharToTokenStrBuffer(int c, char* tokenStrBuffer,int tokenStrBufferLength){
	// If c is not a tab or space add it to the token string buffer
	if(c != ' ' && c != '\t' && c != ':'){
		// Make sure we haven't exceded our buffer length
		if(tokenStrBufferLength < MAX_TOKEN_LENGTH){
			// if c is EOF or newline then escape it to its own string
			if(c == EOF || c == '\n'){
				strcpy(tokenStrBuffer, "\\n"); // EOF becomes a newline
				tokenStrBufferLength = strlen(tokenStrBuffer);
			} else {
				// Otherwise just add c to the buffer
				tokenStrBuffer[tokenStrBufferLength++] = c;
			}
		} else {
			return -1;
		}
	}
	return tokenStrBufferLength;
}

int determineNextChar(FILE* file, int c){
	// Check to see if c is a delimeter we also need to re-process
	if (c == '\n' || c == EOF || c == '=' || c == '+' || c == '-' || c == '*') {
		// Set c to be a blank delimeter to reprocess the delimeter as a token
		c = ' ';
	} else {
		// If c is a not delimeter we need to re-process
		// Get next char in the file
		c = fgetc(file);
	}
	return c;
}

int tokenizer(struct TOKENIZER_CONFIG* tokenizerConfig, struct TOKEN* sTokenArray, \
					struct FILE_TABLE* sFileTable, int* sTokenArrayLength){
	int  error = 0;
	int  commentFlag = 0;
	int  lineNumber = 1;
	int  eofProcessingCounter = 0;
	int  tokenStrBufferLength = 0;
	char tokenStrBuffer[MAX_TOKEN_LENGTH] = {'\0'};
	int  fileIdx = sFileTable->length;
	char* filename = sFileTable->table[fileIdx-1];

	//open file
	// TODO: Add where file is being opened from.
	ulog(DEBUG,"Opening File: %s",filename);
	FILE* sourceFile = fopen(filename, "r");
	if(sourceFile == NULL){
		ulog(ERROR,"Error Opening File: %s",filename);
		return 1;
	}

	int c = ' '; // Init c to a delimiter so we fall through to grab a char and start processing
	while(eofProcessingCounter < 3){ // We need to make 2 more passes once we hit EOF
		if(!commentFlag){
			if(isTokenDelimeter(c) && tokenStrBufferLength){
				// Add Token to TokenArray
				error = addNewTokenToArray(sTokenArray,sTokenArrayLength,tokenStrBuffer,filename,&lineNumber);
				if(error){
					break;
				}

				// Clear token string and index
				memset(tokenStrBuffer,'\0',MAX_TOKEN_LENGTH);
				tokenStrBufferLength = 0;

				if(isIncludeStatement(sTokenArray, *sTokenArrayLength) && !error){  // Check for Include Statement
					char* includedFilename = sTokenArray[*sTokenArrayLength-2].stringValue;
					error = addFileToFileTable(tokenizerConfig, sFileTable, &includedFilename,lineNumber,fileIdx);
					if(error){
						break;
					}
					
					error = tokenizer(tokenizerConfig,sTokenArray,sFileTable,sTokenArrayLength);
					if(error){
						break;
					}
					tokenizerConfig->includedFileDepth--;
				}
			}

			tokenStrBufferLength = addCharToTokenStrBuffer(c, tokenStrBuffer, tokenStrBufferLength);
			if(tokenStrBufferLength == -1){
				ulog(ERROR,"Size of the Token String excedes its maximum length: %i in File %s:%i", \
							tokenStrBufferLength,filename,lineNumber);	
				error = 1;
				break;
			}
		}
	
		// Get the next char depending on if we need to re-process the delimeter
		c = determineNextChar(sourceFile, c);
		// Turn the comment flag on or off depending on if we hit a comment
		commentFlag = (c != '\n' && c != EOF && (c == ';' || commentFlag ));
		
		// Check for EOF or if we are already doing the EOF processing
		if(c == EOF || eofProcessingCounter){
			eofProcessingCounter++;
		}
	}
	
	//close file
	ulog(DEBUG,"Closing File: %s  File Depth: %i",filename,tokenizerConfig->includedFileDepth);
	fclose(sourceFile);
	
    return error;
}

int tokenizeFile(char* filename, struct TOKEN* tokenArray, int maxIncludeFileDepth){
	int tokenArrayLength;
	struct FILE_TABLE fileTable;
	fileTable.length = 0;
	struct TOKENIZER_CONFIG tokenizerConfig;
	tokenizerConfig.includedFileDepth = 0;
	tokenizerConfig.maxFileDepth = maxIncludeFileDepth;

	// Add the first file to the filetable if the table is empty
	addFileToFileTable(&tokenizerConfig, &fileTable, &filename, 0, 0);
	
	ulog(INFO,"Starting Tokenizer");
	tokenizer(&tokenizerConfig, tokenArray, &fileTable, &tokenArrayLength);
	if(getLoggingLevel() >= DEBUG){
		printFileTable(&fileTable);
	}
	return tokenArrayLength;
}



