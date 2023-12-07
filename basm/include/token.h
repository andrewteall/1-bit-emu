#ifndef TOKEN_H
#define TOKEN_H 1

#ifndef MAX_TOKEN_LENGTH
	#define MAX_TOKEN_LENGTH 255
#endif

enum tokenType {MNENOMIC,ASSIGNMENT,INCLUDE,DIRECTIVE,FLAG,NUMBER,NEWLINE,LABEL_MOD,LABEL,};

struct TOKEN {
	enum tokenType type;
	char  stringValue[MAX_TOKEN_LENGTH];	//lexemeString
	int   size;
	int   numericValue;
	int   address;
	int   lineNumber;
	char* filename;
};

void printTokens(struct TOKEN* sTokenArray, int sTokenArrayLength);

#endif