#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "token.h"
#include "ulog.h"
#include "utils.h"

const int delimeterList[] = {' ', '\t', '\n', '\r', EOF, ':', ';', '=', '+', '-', '*', 0};

const char *mnenomicStrings[] = {
			"NOPO","LD","LDC","AND","ANDC","OR","ORC","XNOR","STO","STOC","IEN","OEN","JMP","RTN","SKZ","NOPF","END"};
const char *assignmentStrings[] = {"=","EQU","END"};
const char *includeStrings[] = {"INCLUDE","END"};
const char *directiveStrings[] = {"ORG","REMAP","SUB", "END_S","SUBROUTINE","END_SUBROUTINE","REP","REPEAT","REPEND","END"};
const char *labelModStrings[] = {"+","-","*","END"};
const char *whitespaceStrings[] = {" ",":","\t","\r","END"};

static int isDelimeter(int c){
	for(int i=0; delimeterList[i] != 0;i++){
		if(c == delimeterList[i]){
			return 1;
		}
	}
	return 0;
}

static int containsMatch(const char* arrayToMatch[], char* tokenStr){
	char tokenTmp[strlen(tokenStr)];
	strcpy(tokenTmp,tokenStr);
	toUpperString(tokenTmp);
	
	for(int index = 0; strcmp("END",arrayToMatch[index]); index++){
		if(!strcmp(tokenTmp, arrayToMatch[index])){
			return 1;
		}
	}
	return 0;
}

static int determineTokenType(char* tokenStr){
	int tokenType;

	if(*tokenStr == '\n' || *tokenStr == 255){
		tokenType = NEWLINE;
	} else if(*tokenStr == ';'){
		tokenType = COMMENT;
	} else if(containsMatch(whitespaceStrings, tokenStr) || tokenStr[0] == ' '){
		tokenType = WHITESPACE;
	} else if(containsMatch(mnenomicStrings, tokenStr)){
		tokenType = MNENOMIC;
	} else if(containsMatch(assignmentStrings, tokenStr)){
		tokenType = ASSIGNMENT;
	} else if(containsMatch(includeStrings, tokenStr)){
		tokenType = INCLUDE;
	} else if(containsMatch(directiveStrings, tokenStr)){
		tokenType = DIRECTIVE;
	} else if(str2num(tokenStr) != -1){
		tokenType = NUMBER;
	} else if(containsMatch(labelModStrings, tokenStr)){
		tokenType = LABEL_MOD;
	} else {
		tokenType = LABEL;
	} 

	return tokenType;
}

static int isIncludeStatement(struct TOKEN_LIST* tokenList){
	int idx = tokenList->numTokens-1;
	if(tokenList->numTokens == 0 || tokenList->list[idx].type != NEWLINE){
		return 0;
	}
	--idx;
	while(idx >= 0 && tokenList->list[idx].type != INCLUDE){
		if(tokenList->list[idx].type == NEWLINE || idx == 0){
			return 0;
		}
		--idx;
	}
	
	int idx2 = idx-1;
	while(idx2 >= 0 && tokenList->list[idx2].type == WHITESPACE){
		--idx2;
	}
	if(tokenList->list[idx2].type != NEWLINE){
		return 0;
	}

	++idx;
	while(idx < tokenList->numTokens-1){
		if(tokenList->list[idx].type == WHITESPACE){
			++idx;
		} else if(tokenList->list[idx].type == LABEL){
			return 1;
		} else {
			return 0;
		}
	}

	return 0;
}

static char* getIncludeFilename(struct TOKEN_LIST* tokenList){
	int idx = tokenList->numTokens-1;
	while(idx >= 0 && tokenList->list[idx].type != LABEL){
		--idx;
	}
	return tokenList->list[idx].stringValue;
}

static int hasCircularFileInclude(struct FILE_TABLE* fileTable, int line){
	int parentIdx = fileTable->parentIdx[fileTable->length-1];
	while(parentIdx != -1){
		if(!strcmp(fileTable->table[fileTable->length-1], fileTable->table[parentIdx])){
			ulog(ERROR,"Circular Include in file: %s:%i", fileTable->table[fileTable->parentIdx[fileTable->length-1]], line);
			return 1;
		}
		parentIdx = fileTable->parentIdx[parentIdx];
	}
	return 0;
}

static int setFullPathNameInTable(struct FILE_TABLE* fileTable, char* includeFileName, int fileIdx, int line){
	char* includeFullPathBuffer = fileTable->table[fileTable->length];
	if(fileTable->length == 0){
		includeFullPathBuffer = realpath(includeFileName, includeFullPathBuffer);
		if(includeFullPathBuffer == NULL){
			return 1;
		}
	} else {
		includeFullPathBuffer = realpath(fileTable->table[fileIdx-1], includeFullPathBuffer);
		if(includeFullPathBuffer == NULL){
			return 1;
		}
		do {
			char* slashIndex = strrchr(includeFullPathBuffer, '/');
			if(slashIndex == NULL){
				return 1;
			}
			includeFullPathBuffer[slashIndex - includeFullPathBuffer + 1] = '\0';

			if(includeFileName[0] == '.' && includeFileName[1] == '.' && includeFileName[2] == '/'){
				includeFullPathBuffer[strlen(includeFullPathBuffer)-2] = '\0';
				for(int i=0;i != strlen(includeFileName)+1; i++){
					includeFileName[i] = includeFileName[i+3];
					includeFileName[i+3] = '\0';
				}
			}
		} while(includeFileName[0] == '.' && includeFileName[1] == '.' && includeFileName[2] == '/');
		strcat(includeFullPathBuffer, includeFileName);
	}
	
	ulog(DEBUG,"Adding File to FileTable: %s at index %i", includeFullPathBuffer, fileTable->length);
	fileTable->parentIdx[fileTable->length] = fileIdx-1;
	fileTable->length++;

	return hasCircularFileInclude(fileTable, line);
}

static char* getLastFileParentFilename(struct FILE_TABLE* fileTable){
	return fileTable->table[fileTable->parentIdx[fileTable->length]-1];
}

static int addFileToFileTable(struct FILE_TABLE* fileTable, char* includedFile, int line, int fileIdx){
	int error = 0;
	if (fileTable->length >= MAX_FILE_INCLUDES){
		ulog(ERROR,"Too many includes: %i Last Include: %s:%i", MAX_FILE_INCLUDES, getLastFileParentFilename(fileTable), line);
		error = 1;
	} else {
		error = setFullPathNameInTable(fileTable, includedFile, fileIdx, line);
	}
	return error;
}

static int tokenizer(struct TOKENIZER_CONFIG* tokenizerConfig, struct TOKEN_LIST* tokenList){
	int  error = 0;
	int  commentFlag = 0;
	int  whitespaceFlag = 0;
	int  lineNumber = 1;

	int  tokenStrBufferLength = 0;
	char tokenStrBuffer[MAX_TOKEN_LENGTH] = {'\0'};
	
	int  fileIdx = tokenizerConfig->fileTable.length;
	char* filename = tokenizerConfig->fileTable.table[fileIdx-1];

	// Open file
	ulog(DEBUG,"Opening File: %s",filename);
	FILE* sourceFile = fopen(filename, "r");
	if(sourceFile == NULL){
		ulog(ERROR,"Error Opening File: %s",filename);
		return 1;
	}
	
	int c = 0;
	int prevC = 0;
	while(prevC != EOF && !error){
		// Get the next character
		prevC = c;
		if(c != EOF){
			c = fgetc(sourceFile);
		}

		// Turn the comment flag on if prevC is a ';' or if the comment flag is
		// already set. Turn the comment flag off if c is a newline or EOF.
		commentFlag = (c != '\n' && c != EOF && (prevC == ';' || commentFlag ));
		whitespaceFlag = (c == ' ' && prevC == ' ');
		if((isDelimeter(c) || isDelimeter(tokenStrBuffer[0])) && !whitespaceFlag && !commentFlag && tokenStrBufferLength){
			// Add Token to TokenList
			error = addNewToken(tokenList, tokenStrBuffer, determineTokenType(tokenStrBuffer), filename, &lineNumber);
			if(error){
				break;
			}

			// Clear token string and index
			memset(tokenStrBuffer,'\0',tokenStrBufferLength);
			tokenStrBufferLength = 0;

			if(isIncludeStatement(tokenList) && !error){
				char* includedFilename = getIncludeFilename(tokenList);
				error = addFileToFileTable(&tokenizerConfig->fileTable, includedFilename, lineNumber-1, fileIdx);
				if(error){
					break;
				}
				
				error = tokenizer(tokenizerConfig, tokenList);
				if(error){
					break;
				}
			}
		}
		tokenStrBuffer[tokenStrBufferLength++] = c;
		if(tokenStrBufferLength >= MAX_TOKEN_LENGTH){
			ulog(ERROR,"Length of the Token String excedes its maximum length: %i in File %s:%i", \
						tokenStrBufferLength, filename, lineNumber);	
			error = 1;
			break;
		}
	}
	
	// Close file
	ulog(DEBUG,"Closing File: %s", filename);
	fclose(sourceFile);
	
    return error;
}

int tokenizeFile(char* filename, struct TOKEN_LIST* tokenList, struct TOKENIZER_CONFIG* tokenizerConfig){
	// Initialize Tokenizer Config 
	tokenizerConfig->fileTable.length = 0;
	tokenList->numTokens = 0;
	tokenList->nextToken = tokenList->list;

	// Add the first file to the filetable
	if(addFileToFileTable(&tokenizerConfig->fileTable, filename, 0, 0)){
		ulog(ERROR, "Cannot Add File to File Table: %s", filename);
		return 1;
	}
	
	ulog(INFO,"Starting Tokenizer");
	tokenizer(tokenizerConfig, tokenList);
	
	ulog(INFO,"Generated %i Tokens", tokenList->numTokens);
	return tokenList->numTokens;
}

void printFileTable(struct FILE_TABLE* sFileTable){
	printf("================================= File Table ===================================\n");
	printf("| %s %28s %44s\n", "Idx", "Filename", "Parent Idx");
	for(int i=0; i < sFileTable->length; i++){
		printf("| %2i: %-65s %3i\n", i, sFileTable->table[i], sFileTable->parentIdx[i]);
	}
	printf("================================================================================\n\n");
}
