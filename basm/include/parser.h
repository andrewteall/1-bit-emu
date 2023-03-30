#ifndef PARSER_H
#define PARSER_H 1

#include "basm.h"

struct LABEL;

void printLabelTable(struct LABEL*, int);
int parseTokens(struct OPTIONS* options,struct TOKEN* tokenArr,int tokenizedArraySize);

int checkOverflow(struct OPTIONS* sOptions, struct TOKEN* token,int value,int address);
int getMnenomicOpCode(char* string);

int getCountBytesInNum(unsigned int num);
int getCountBitsInNum(unsigned int num);
int getLabelValue(struct LABEL *labelTable, int labelTableLen, char* labelName, int subroutineID);
int getLabelIdx(struct LABEL *labelTable, int labelTableLen, char* labelName, int subroutineID);
int addLabel(struct LABEL *labelTable, char* str,int value,int labelTableLen,int pass,struct TOKEN* token, \
				int isRemapped,int inSubroutine);


int processMnemonic(struct OPTIONS* options,struct TOKEN* tokenArr, int currentAddress,int opcode,int address, int pass);
int processDirective(struct OPTIONS* options,struct TOKEN* tokenArr, struct LABEL *labelTable, int* labelIdx, \
						int tokenIdx, int currentAddress,int value,int passCounter);
#endif
