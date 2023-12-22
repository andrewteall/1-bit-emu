#ifndef PARSER_H
#define PARSER_H 1

#include "token.h"

#ifndef MAX_STATEMENT_LENGTH
	#define MAX_STATEMENT_LENGTH 6
#endif

#ifndef MAX_LABEL_LENGTH
	#define MAX_LABEL_LENGTH 255
#endif

#ifndef MAX_LABEL_COUNT
	#define MAX_LABEL_COUNT 1024
#endif

struct LABEL {
	char* name;
	int   value;
	int   pass;
	int   lineNumber;
	char* filename;
	int   isRemapped;
	int   subroutineID;
};

struct PARSER_CONFIG {
	int   instructionWidth;
	int   addressWidth;
	int   instructionPosition;
	int   addressPosition;
	int   wordWidth;
	int   printLabelTable;
};

void printLabelTable(struct LABEL*, int);
int parseTokens(struct PARSER_CONFIG* parserConfig,struct TOKEN_LIST* tokenList);

int checkOverflow(struct PARSER_CONFIG* parserConfig, struct TOKEN* token,int value,int address);
int getMnenomicOpCode(char* string);

int getCountBytesInNum(unsigned int num);
int getCountBitsInNum(unsigned int num);
int getLabelValue(struct LABEL *labelTable, int labelTableLen, char* labelName, int subroutineID);
int getLabelIdx(struct LABEL *labelTable, int labelTableLen, char* labelName, int subroutineID);
int addLabel(struct LABEL *labelTable, char* str,int value,int labelTableLen,int pass,struct TOKEN* token, \
				int isRemapped,int inSubroutine);


int processMnemonic(struct PARSER_CONFIG* parserConfig,struct TOKEN* tokenArr,int currentAddress,int opcode, \
						int address,int pass);
int processDirective(struct PARSER_CONFIG* parserConfig,struct TOKEN** statement, struct LABEL *labelTable, int* labelIdx, \
						int tokenIdx, int currentAddress,int value,int passCounter);
#endif
