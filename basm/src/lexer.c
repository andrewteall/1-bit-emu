#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "token.h"
#include "ulog.h"
#include "utils.h"

const int tokenDelimeterList[] = {' ', '\t', '\n', EOF, ':', '=', '+', '-', '*', 0};
const int statmentDelimeterList[] = {'\n', EOF, 0};
const int modifierList[] = {'=', '+', '-', '*', 0};

const char *mnenomicStrings[] = {
			"NOPO","LD","LDC","AND","ANDC","OR","ORC","XNOR","STO","STOC","IEN","OEN","JMP","RTN","SKZ","NOPF","END"};
const char *assignmentStrings[] = {"=","EQU","END"};
const char *includeStrings[] = {"INCLUDE","END"};
const char *directiveStrings[] = {"ORG","REMAP","SUB", "END_S","SUBROUTINE","END_SUBROUTINE","REP","REPEAT","REPEND","END"};
const char *labelModStrings[] = {"+","-","*","END"};

int isDelimeter(int c, const int* delimiterList){
	for(int i=0; delimiterList[i] != 0;i++){
		if(c == delimiterList[i]){
			return 1;
		}
	}
	return 0;
}

int isTokenDelimeter(int c){
	return isDelimeter(c, tokenDelimeterList);
}

int isStatementDelimeter(int c){
	return isDelimeter(c, statmentDelimeterList);
}

int isModifier(int c){
	return isDelimeter(c, modifierList);
}

int isIncludeStatement(struct TOKEN_LIST* tokenList){
	// FIXME: Check for NEWLINE before include
	return (tokenList->list[tokenList->numTokens-3].type == INCLUDE && tokenList->list[tokenList->numTokens-1].type == NEWLINE);
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

int determineTokenType(char* tokenStr){
	int tokenType;
	char tokenTmp[strlen(tokenStr)];
	strcpy(tokenTmp,tokenStr);
	
	if(*tokenStr == '\n' || !strcmp(tokenStr,"\\n") || !strcmp(tokenStr,"\\N")){
		tokenType = NEWLINE;
	} else if(containsMatch(mnenomicStrings,tokenStr)){
		tokenType = MNENOMIC;
	} else if(containsMatch(assignmentStrings,tokenStr)){
		tokenType = ASSIGNMENT;
	} else if(containsMatch(includeStrings,tokenStr)){
		tokenType = INCLUDE;
	} else if(containsMatch(directiveStrings,tokenStr)){
		tokenType = DIRECTIVE;
	} else if(str2num(tokenStr) != -1){
		tokenType = NUMBER;
	} else if(containsMatch(labelModStrings,tokenStr)){
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
		printf("| %2i: %32s %*i\n", i, sFileTable->table[i], (int)(70-strlen(sFileTable->table[i])), sFileTable->parentIdx[i]);
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

int addFileToFileTable(struct TOKENIZER_CONFIG* tokenizerConfig, struct FILE_TABLE* fileTable, char** includedFile, \
								int line,int fileIdx){
	if (fileTable->length == MAX_FILE_INCLUDES){
		ulog(ERROR,"Too many includes: %i Last Include: %s:%i",MAX_FILE_INCLUDES, \
						fileTable->table[fileTable->parentIdx[fileTable->length]-1],line-1);
		return 1;
	} else if(tokenizerConfig->includedFileDepth == tokenizerConfig->maxFileDepth){
		ulog(ERROR,"Too many nested includes %i: %s:%i  You can override this value with the -m flag", \
						tokenizerConfig->maxFileDepth,fileTable->table[fileIdx-1],line-1);
		return 1;
	} else {
		char* sourceFullPathFilename = fileTable->table[fileTable->length];
		if(fileTable->length == 0){
			realpath(*includedFile,sourceFullPathFilename);
		} else {
			sourceFullPathFilename = realpath(fileTable->table[fileIdx-1], sourceFullPathFilename);
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
			strcat(fileTable->table[fileTable->length],*includedFile);
		}
		ulog(DEBUG,"Adding File to FileTable: %s at index %i",sourceFullPathFilename,fileTable->length);
		fileTable->parentIdx[fileTable->length] = fileIdx-1;
		(fileTable->length)++;

		tokenizerConfig->includedFileDepth++;
		*includedFile = sourceFullPathFilename;
		if(hasCircularFileInclude(fileTable)){
			ulog(DEBUG,"File Table Length: %i  Included File Depth: %i",fileTable->length,tokenizerConfig->includedFileDepth);
			printFileTable(fileTable);
			ulog(ERROR,"Circular Include in file: %s:%i",fileTable->table[fileTable->parentIdx[fileTable->length-1]],line-1);
			
			return 1;
		}
		return 0;
	}
}

int tokenizer(struct TOKENIZER_CONFIG* tokenizerConfig, struct TOKEN_LIST* tokenList){
	int  error = 0;
	int  commentFlag = 0;
	int  lineNumber = 1;

	int  tokenStrBufferLength = 0;
	char tokenStrBuffer[MAX_TOKEN_LENGTH] = {'\0'};
	
	int  fileIdx = tokenizerConfig->fileTable.length;
	char* filename = tokenizerConfig->fileTable.table[fileIdx-1];

	//open file
	// TODO: Add where file is being opened from.
	ulog(DEBUG,"Opening File: %s",filename);
	FILE* sourceFile = fopen(filename, "r");
	if(sourceFile == NULL){
		ulog(ERROR,"Error Opening File: %s",filename);
		return 1;
	}
	
	int c;
	while(c != EOF && !error){
		// Get the next character
		c = fgetc(sourceFile);

		// Turn the comment flag on or off depending on if we hit a comment
		commentFlag = (c != '\n' && c != EOF && (c == ';' || commentFlag ));

		if(isTokenDelimeter(c) && tokenStrBufferLength && !commentFlag){
			// Add Token to TokenArray
			ulog(TRACE,"Adding token string: %s", tokenStrBuffer);
			error = addNewToken(tokenList, tokenStrBuffer, determineTokenType(tokenStrBuffer), filename, lineNumber);
			if(error){
				break;
			}

			// Clear token string and index
			memset(tokenStrBuffer,'\0',MAX_TOKEN_LENGTH);
			tokenStrBufferLength = 0;
		}

		if(isModifier(c) && !commentFlag){
			tokenStrBuffer[0] = c;
			tokenStrBuffer[1] = '\0';
			// Add Token to TokenArray
			ulog(TRACE,"Adding token string: %s", tokenStrBuffer);
			error = addNewToken(tokenList, tokenStrBuffer, determineTokenType(tokenStrBuffer), filename, lineNumber);
			if(error){
				break;
			}
			
			// Clear token string and index
			memset(tokenStrBuffer,'\0',MAX_TOKEN_LENGTH);
			tokenStrBufferLength = 0;
		}

		if(isStatementDelimeter(c) && !commentFlag){
			// Add Token to TokenArray
			ulog(TRACE,"Adding token string: %s", "\\n");
			error = addNewToken(tokenList, "\\n", determineTokenType("\\n"), filename, lineNumber);
			lineNumber++;
			if(error){
				break;
			}
			
			// Check for Include Statement
			if(isIncludeStatement(tokenList) && !error){  
				char* includedFilename = tokenList->list[tokenList->numTokens-2].stringValue;
				error = addFileToFileTable(tokenizerConfig, &tokenizerConfig->fileTable, &includedFilename, lineNumber, fileIdx);
				if(error){
					break;
				}
				
				error = tokenizer(tokenizerConfig, tokenList);
				if(error){
					break;
				}
				tokenizerConfig->includedFileDepth--;
			}
		} 
		
		if(!isTokenDelimeter(c) && !isModifier(c) && !isStatementDelimeter(c) && !commentFlag) {
			tokenStrBuffer[tokenStrBufferLength++] = c;
			if(tokenStrBufferLength >= MAX_TOKEN_LENGTH){
				ulog(ERROR,"Size of the Token String excedes its maximum length: %i in File %s:%i", \
							tokenStrBufferLength,filename,lineNumber);	
				error = 1;
				break;
			}
		}
	}
	
	//close file
	ulog(DEBUG,"Closing File: %s  File Depth: %i",filename,tokenizerConfig->includedFileDepth);
	fclose(sourceFile);
	
    return error;
}

int tokenizeFile(char* filename, struct TOKEN_LIST* tokenList, struct TOKENIZER_CONFIG* tokenizerConfig){
	// Initialize Tokenizer Config 
	tokenizerConfig->includedFileDepth = 0;
	tokenizerConfig->fileTable.length = 0;
	tokenList->numTokens = 0;
	tokenList->nextToken = tokenList->list;

	// Add the first file to the filetable
	addFileToFileTable(tokenizerConfig, &tokenizerConfig->fileTable, &filename, 0, 0);
	
	ulog(INFO,"Starting Tokenizer");
	tokenizer(tokenizerConfig, tokenList);
	
	ulog(INFO,"Generated %i Tokens", tokenList->numTokens);
	return tokenList->numTokens;
}
