#ifndef TOKEN_H
#define TOKEN_H 1

#ifndef MAX_NUM_TOKENS
    #define MAX_NUM_TOKENS 16384
#endif

#ifndef MAX_TOKEN_LENGTH
    #define MAX_TOKEN_LENGTH 255
#endif

enum tokenType {MNENOMIC,ASSIGNMENT,INCLUDE,DIRECTIVE,FLAG,NUMBER,NEWLINE,LABEL_MOD,WHITESPACE,COMMENT,LABEL,};

struct TOKEN {
	int type;
	char  stringValue[MAX_TOKEN_LENGTH];	//lexemeString
	int   size;
	int   numericValue;
	int   address;
	int   lineNumber;
	char* filename;
};

struct TOKEN_LIST {
	struct TOKEN list[MAX_NUM_TOKENS];
	int numTokens;
	struct TOKEN* nextToken;
};

void printTokens(struct TOKEN_LIST* tokenList);

int addNewToken(struct TOKEN_LIST* tokenList, char* tokenStr, int type, char* filename, int* lineNumber);

#endif