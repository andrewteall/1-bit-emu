#ifndef BASM_H
#define BASM_H 1

#include <stdio.h>

#ifndef MSB
    #define MSB 0
#endif

#ifndef LSB
    #define LSB 0
#endif

#ifndef MAX_STATEMENT_LENGTH
	#define MAX_STATEMENT_LENGTH 6
#endif

#ifndef MAX_TOKEN_LENGTH
	#define MAX_TOKEN_LENGTH 255
#endif

#ifndef MAX_LABEL_LENGTH
	#define MAX_LABEL_LENGTH 255
#endif

#ifndef MAX_LABEL_COUNT
	#define MAX_LABEL_COUNT 1024
#endif

#ifndef PATH_MAX
	#define PATH_MAX 4096
#endif

#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN 0
#endif

#ifndef BIG_ENDIAN
	#define BIG_ENDIAN 1
#endif

// Statements:
// MNENOMIC [NUMBER|LABEL[LABEL_MOD NUMBER]] NEWLINE
// DIRECTIVE NUMBER|LABEL NEWLINE
// LABEL NEWLINE
// LABEL [ASSIGNMENT NUMBER|LABEL] NEWLINE
// INCLUDE LABEL NEWLINE

enum tokenType {MNENOMIC,ASSIGNMENT,INCLUDE,DIRECTIVE,FLAG,NUMBER,NEWLINE,LABEL_MOD,LABEL,};
enum mnenomic {NOPO,LD,LDC,AND,ANDC,OR,ORC,XNOR,STO,STOC,IEN,OEN,JMP,RTN,SKZ,NOPF,};

struct TOKEN {
	int   type;
	char  stringValue[MAX_TOKEN_LENGTH];	//lexemeString
	int   size;
	int   numericValue;
	int   address;
	int   lineNumber;
	char* filename;
};

struct OPTIONS {
	char* filename;
	char  outFilePath[PATH_MAX];
	int   endianess;
	int   instructionWidth;
	int   addressWidth;
	int   instructionPosition;
	int   addressPosition;
	int   interlaceFile;
	int   splitFile;
	int   wordWidth;
	int   maxFileDepth;
	int   includedFileDepth;
	int   format;
	int   onlyTokenize;
	int   prettyPrint;
	int   printLabelTable;
	int   parsePrint;
	int   align;
	int   alignValue;
};


const char *mnenomicStrings[17];
const char *tokenTypeStrings[9];
const char *assignmentStrings[3];
const char *includeStrings[2];
const char *directiveStrings[10];
const char *labelModStrings[4];


int  setWordWidth(struct OPTIONS*);
void setDefaultOptions(struct OPTIONS*);
char writeFile(char*, char*, size_t );
int  writeByteToArray(char* , struct OPTIONS* , int , int );
void prettyPrintBytes(char* , int );

/*
 * Destructively makes a string uppercase.
 *
 * @param string Pointer to the string to make upper case.
 * @returns char* Pointer to the uppercase string.
 */
char* toUpperString(char* string);

int roundUp(float);
int str2num(char*);
long expo(int, int);

void printHelp(void);
void printTokens(struct TOKEN* tokenArray,int tokenArrayIndex);
int parseCommandLineAndInitOptions(struct OPTIONS* sOptions,int argc, char* argv[]);

/**
 * write bytes to given file.
 *
 * @param path file to write.
 * @param bytes bytes to write into file.
 * @param size byte size.
 * @returns boolean value.
 */
// char writeFile(char *path, unsigned char *bytes, size_t size);

#endif