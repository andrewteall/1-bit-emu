#ifndef BASM_H
#define BASM_H 1

#define MAJOR "0"
#define MINOR "9"
#define PATCH "0"
#define VERSION  MAJOR "." MINOR "." PATCH

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

enum mnenomic {NOPO,LD,LDC,AND,ANDC,OR,ORC,XNOR,STO,STOC,IEN,OEN,JMP,RTN,SKZ,NOPF,};

struct OPTIONS {
	char* filename;
	char*  outFilePath;
	int   endianess;
	int   instructionWidth;
	int   addressWidth;
	int   instructionPosition;
	int   addressPosition;
	int   interlaceFile;
	int   splitFile;
	int   wordWidth;
	int   maxFileDepth;
	int   format;
	int   onlyTokenize;
	int   prettyPrint;
	int   printLabelTable;
	int   parsePrint;
	int   align;
	int   alignValue;
};


char writeFile(char*, char*, size_t );
int  writeByteToArray(char* , struct OPTIONS* , int , int );
void prettyPrintBytes(char* , int );


int assemble(struct OPTIONS* sOptions, char* filename, char* binArr);

void printHelp(void);
void setDefaultOptions(struct OPTIONS* options);
int parseCommandLine(struct OPTIONS* sOptions,int argc, char* argv[]);

#endif