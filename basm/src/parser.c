#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "utils.h"
#include "token.h"
#include "ulog.h"

const char *mnenomicStrings2[] = {
	"NOPO","LD","LDC","AND","ANDC","OR","ORC","XNOR","STO","STOC","IEN","OEN","JMP","RTN","SKZ","NOPF","NULL"};

struct TOKEN blankToken = {-1,"", -1, -1, -1, -1, ""};

void checkForRemapping(struct TOKEN** statement,struct LABEL* labelTable,int labelTableLen){
	if (statement[0]->type == LABEL){
		int label = getLabelIdx(labelTable,labelTableLen,statement[0]->stringValue,0);
		if (label != -1 && labelTable[label].isRemapped){
			statement[0]->type = MNENOMIC;
		}
	}
}

int isMnenomicStatement(struct TOKEN** statement){
	if ((statement[0]->type == MNENOMIC  &&  statement[1]->type == NUMBER  && statement[2]->type == NEWLINE)  || \
			    (statement[0]->type == MNENOMIC  &&  statement[1]->type == LABEL   && statement[2]->type == NEWLINE)  || \
				(statement[0]->type == MNENOMIC  &&  statement[1]->type == NEWLINE)  						    || \
				(statement[0]->type == MNENOMIC  && (statement[1]->type == NUMBER || statement[1]->type == LABEL)     && \
				 statement[2]->type == LABEL_MOD &&  (statement[3]->type == NUMBER  || statement[3]->type == LABEL)   && \
				 statement[4]->type == NEWLINE)){ 
		return 1;
	}
	return 0;
}

int isDirectiveStatement(struct TOKEN** statement){
	if (statement[0]->type == DIRECTIVE && \
					 ((statement[1]->type == LABEL    && statement[2]->type == NEWLINE) 							|| \
					  (statement[1]->type == NUMBER   && statement[2]->type == NEWLINE) 							|| \
					  (statement[1]->type == MNENOMIC && statement[2]->type == LABEL && statement[3]->type == NEWLINE) 	|| \
					  (statement[1]->type == NEWLINE))){
		return 1;
	}
	return 0;
}

int isLabelStatement(struct TOKEN** statement){
	if ((statement[0]->type == LABEL && statement[1]->type == NEWLINE) || \
			(statement[0]->type == LABEL && statement[1]->type == ASSIGNMENT && statement[2]->type == NUMBER && statement[3]->type == NEWLINE) || \
			(statement[0]->type == LABEL && statement[1]->type == ASSIGNMENT && statement[2]->type == LABEL && statement[3]->type == NEWLINE)  || \
			(statement[0]->type == LABEL && statement[1]->type == ASSIGNMENT && (statement[2]->type == NUMBER || statement[2]->type == LABEL)  && \
			 statement[3]->type == LABEL_MOD && (statement[4]->type == LABEL || statement[4]->type == NUMBER) && statement[5]->type == NEWLINE)){
		return 1;
	}
	return 0;
}

int isInclude(struct TOKEN** statement){
	if(statement[0]->type == INCLUDE && statement[1]->type == LABEL && statement[2]->type == NEWLINE){
		return 1;
	}
	return 0;
}

int isBlankLine(struct TOKEN** statement){
	if(statement[0]->type == NEWLINE){
		return 1;
	}
	return 0;
}

int getNextStatement(struct TOKEN** statement, struct TOKEN_LIST* tokenList, int* tokenIdx){
	// Clear Statement Buffer
	for(int i=0; i < MAX_STATEMENT_LENGTH; i++){
		statement[i] = &blankToken; 
	}
	
	// Get Next Statement and Store in Buffer
	int statementLength = 0;
	while(tokenList->list[*tokenIdx].type != NEWLINE && statementLength < MAX_STATEMENT_LENGTH){
		statement[statementLength++] = &tokenList->list[*tokenIdx]; 
		(*tokenIdx)++;
	}
	statement[statementLength] = &tokenList->list[*tokenIdx];

	return *tokenIdx;
}

int parseTokens(struct PARSER_CONFIG* parserConfig, struct TOKEN_LIST* tokenList){
	ulog(INFO,"Starting Parser");
	int error = 0;
	int labelTableLen = 0;
	int maxAddress = 0;
	struct LABEL labelTable[MAX_LABEL_COUNT];
	
	for (int passCounter = 0; passCounter < 2; passCounter++){
		int currentAddress = 0;
		int inSubroutine = 0;
		int subroutineCounter = 0;

		ulog(INFO,"Starting Pass %i",passCounter+1);
		for (int i=0; i < tokenList->numTokens; i++){
			struct TOKEN* statement[MAX_STATEMENT_LENGTH];

			getNextStatement(statement,tokenList,&i);

			int tokenIdx = 0;
			checkForRemapping(statement, labelTable, labelTableLen);
			
			if(isMnenomicStatement(statement)){ 
				int  subroutineID = 0;
				int  statementError = 0;
				int  operand = -1;
				int  opcode = -1;
				char errorMsg[255] = {'\0'};
				int  errorToken = 0;
				
				if(statement[tokenIdx]->type == LABEL){
					opcode = getLabelValue(labelTable,labelTableLen,statement[tokenIdx]->stringValue,0);
				} else {
					opcode = getMnenomicOpCode(statement[tokenIdx]->stringValue);
				}
				
				if(opcode == -1){
					// Should never get here becasue we already to checked to see if this label exists
					strcpy(errorMsg,"Invalid Opcode");
					errorToken = 0;
					statementError = 1;
				}
				switch (statement[1]->type){
				case NUMBER:
					operand = str2num(statement[tokenIdx+1]->stringValue);
					if(passCounter && operand == -1){
						strcpy(errorMsg,"Not a Number");
						errorToken = 1;
						statementError = 1;
					}
					break;
				case LABEL:
					subroutineID = inSubroutine * subroutineCounter;
					operand = getLabelValue(labelTable,labelTableLen,statement[tokenIdx+1]->stringValue,subroutineID);
					if(passCounter && operand == -1){
						strcpy(errorMsg,"Label does not exist");
						errorToken = 1;
						statementError = 1;
					}
					break;
				case NEWLINE:
					operand = 0;
					break;
				}
				
				switch (statement[2]->type + statement[3]->type){
				case LABEL_MOD+NUMBER:
					if(!strcmp(statement[tokenIdx+2]->stringValue,"+")){
						operand += str2num(statement[tokenIdx+3]->stringValue);
					} else if(!strcmp(statement[tokenIdx+2]->stringValue,"-")){
						operand -= str2num(statement[tokenIdx+3]->stringValue);
					} else if(!strcmp(statement[tokenIdx+2]->stringValue,"*")){
						operand *= str2num(statement[tokenIdx+3]->stringValue);
					} else {						
						strcpy(errorMsg,"Invalid Modifier");
						errorToken = 2;
						statementError = 1;
					}
					if(passCounter && operand == -1){
						strcpy(errorMsg,"Not a Number");
						errorToken = 3;
						statementError = 1;
					}
					break;
					
				case LABEL_MOD+LABEL:
					if(!strcmp(statement[tokenIdx+2]->stringValue,"+")){
						operand += getLabelValue(labelTable,labelTableLen,statement[tokenIdx+3]->stringValue,subroutineID);
					} else if(!strcmp(statement[tokenIdx+2]->stringValue,"-")){
						operand -= getLabelValue(labelTable,labelTableLen,statement[tokenIdx+3]->stringValue,subroutineID);
					} else if(!strcmp(statement[tokenIdx+2]->stringValue,"*")){
						operand *= getLabelValue(labelTable,labelTableLen,statement[tokenIdx+3]->stringValue,subroutineID);
					} else {
						strcpy(errorMsg,"Invalid Modifier");
						errorToken = 2;
						statementError = 1;
					}

					if(passCounter && operand == -1){
						strcpy(errorMsg,"Label does not exist");
						errorToken = 3;
						statementError = 1;
					}
					break;
				}

				if(passCounter == 1 && statementError == 1){
					ulog(ERROR,"%s: %s in file %s:%i",errorMsg, statement[tokenIdx+errorToken]->stringValue, \
					statement[tokenIdx+errorToken]->filename, statement[tokenIdx+errorToken]->lineNumber);
					error = 1;
				}
				
				currentAddress = processMnemonic(parserConfig,statement[tokenIdx],currentAddress,opcode,operand,passCounter);

			} else if(isDirectiveStatement(statement)){
				int directiveValue = 0;
				int subroutineID;
				char* directiveString = statement[tokenIdx]->stringValue;

				switch (statement[1]->type){
				case LABEL:
					subroutineID = inSubroutine * subroutineCounter;
					directiveValue = getLabelValue(labelTable,labelTableLen,statement[tokenIdx+1]->stringValue,subroutineID);
					break;
				case NUMBER:
					directiveValue = str2num(statement[tokenIdx+1]->stringValue);
					break;
				case MNENOMIC:
					directiveValue = getMnenomicOpCode(statement[tokenIdx+1]->stringValue);
					break;
				case NEWLINE:
					
					if(!strcmp("SUB",directiveString) || !strcmp("SUBROUTINE",directiveString)){
						inSubroutine = 1;
						subroutineCounter++;
					}
					
					if(!strcmp("END_S",directiveString) || !strcmp("END_SUBROUTINE",directiveString)){
						inSubroutine = 0;
					}

					break;
				}

				if(passCounter == 1 && directiveValue == -1){
					ulog(ERROR,"Label does not exist: %s in file %s:%i", statement[tokenIdx+1]->stringValue, \
										statement[tokenIdx+1]->filename ,statement[tokenIdx+1]->lineNumber);
					error = 1;

				}
				currentAddress = processDirective(parserConfig,statement,labelTable,&labelTableLen,tokenIdx, \
													currentAddress,directiveValue,passCounter);

				

			} else if(isLabelStatement(statement)){
				int subroutineID;
				int labelValue = 0;
				struct TOKEN* token;
				char* labelName = statement[tokenIdx]->stringValue;
				
				switch (statement[2]->type){
				case NUMBER:
					labelValue = str2num(statement[tokenIdx+2]->stringValue);
					token = &tokenList->list[tokenIdx+2];
					break;
				case LABEL:
					subroutineID = inSubroutine * subroutineCounter;
					labelValue = getLabelValue(labelTable,labelTableLen,statement[tokenIdx+2]->stringValue,subroutineID);
					token = &tokenList->list[tokenIdx+2];
					break;
				default:
					labelValue = currentAddress;
					tokenList->list[i].address = currentAddress;
					token = &tokenList->list[tokenIdx];
					break;
				}

				switch (statement[3]->type + statement[4]->type){
				case LABEL_MOD+NUMBER:
					if(!strcmp(statement[tokenIdx+3]->stringValue,"+")){
						labelValue += str2num(statement[tokenIdx+4]->stringValue); 
					} else if(!strcmp(statement[tokenIdx+3]->stringValue,"-")){
						labelValue -= str2num(statement[tokenIdx+4]->stringValue); 
					} else if(!strcmp(statement[tokenIdx+3]->stringValue,"*")){
						labelValue *= str2num(statement[tokenIdx+4]->stringValue); 
					} else {
						ulog(ERROR,"Invalid Label Modifier: %s in file %s:%i",statement[tokenIdx+3]->stringValue, \
									statement[tokenIdx+3]->filename, statement[tokenIdx+3]->lineNumber);
						error = 1;
					}
					break;

				case LABEL_MOD+LABEL:
					if(!strcmp(statement[tokenIdx+3]->stringValue,"+")){
						labelValue += getLabelValue(labelTable,labelTableLen,statement[tokenIdx+4]->stringValue,subroutineID);
					} else if(!strcmp(statement[tokenIdx+3]->stringValue,"-")){
						labelValue -= getLabelValue(labelTable,labelTableLen,statement[tokenIdx+4]->stringValue,subroutineID);
					} else if(!strcmp(statement[tokenIdx+3]->stringValue,"*")){
						labelValue *= getLabelValue(labelTable,labelTableLen,statement[tokenIdx+4]->stringValue,subroutineID);
					} else {
						ulog(ERROR,"Invalid Label Modifier: %s in file %s:%i",statement[tokenIdx+3]->stringValue, \
									statement[tokenIdx+3]->filename, statement[tokenIdx+3]->lineNumber);
						error = 1;
					}
					break;
				}

				if(passCounter && checkOverflow(parserConfig, token, 0, labelValue)){
					return -1;
				}
				int subroutine = inSubroutine * subroutineCounter;
				labelTableLen = addLabel(labelTable,labelName,labelValue,labelTableLen,passCounter,&tokenList->list[tokenIdx],0, subroutine);

				
				if (labelTableLen == -1){
					return -1;
				}
			

			} else if(isInclude(statement)){
				//Ignore includes here
			} else if(isBlankLine(statement)){
				//Ignore blank lines
			} else {
				if(passCounter){
					char errorStatementStr[255];
					for(int j=0; tokenIdx+j != i;j++){
						strcat(errorStatementStr,tokenList->list[tokenIdx+j].stringValue);
						strcat(errorStatementStr," ");
					}
				
					ulog(ERROR,"Invalid Statement %sin File: %s:%i ", \
												errorStatementStr,tokenList->list[i].filename,tokenList->list[i].lineNumber);
					memset(errorStatementStr,0,255);
					error = 1;
				}
			}
			
			if(currentAddress == -1){
				return -1;
			}


			if ((currentAddress+tokenList->list[i].size) > maxAddress){
				maxAddress = currentAddress+tokenList->list[i].size;
			}
		}
		
		if(parserConfig->printLabelTable){
			printLabelTable(labelTable,labelTableLen);
		}
	}
	
	if(error){
		ulog(ERROR,"Error occurred... exiting");
	} else {
		ulog(DEBUG,"Binary file length: %i bytes",maxAddress);
	}
	return (maxAddress * !error);
}

int checkOverflow(struct PARSER_CONFIG* parserConfig, struct TOKEN* token,int value,int address){
	int overflow = 0;
	if(getCountBytesInNum(value) > parserConfig->wordWidth){
		ulog(ERROR,"Overflow word width: %i in file: %s:%i",value,token->filename,token->lineNumber);
		overflow = 1;
	}
	if(getCountBitsInNum(address) > parserConfig->addressWidth){
		ulog(ERROR,"Overflow address width: %i in file %s:%i",address,token->filename,token->lineNumber);
		overflow = 1;
	}
	return overflow;
}

int processMnemonic(struct PARSER_CONFIG* parserConfig, struct TOKEN* token, int currentAddress,int opcode,int address, int pass){
	int opcodePositionVal = opcode << (((parserConfig->wordWidth*8)-parserConfig->instructionWidth)-parserConfig->instructionPosition);
	int addressPositionVal = address << (((parserConfig->wordWidth*8)-parserConfig->addressWidth)-parserConfig->addressPosition);

	int value = (opcodePositionVal | addressPositionVal);
	
	if(pass && address != -1 && checkOverflow(parserConfig, token, value, address)){
		return -1;
	}

	token->numericValue = value;
	token->address = currentAddress;
	token->size = parserConfig->wordWidth;

	currentAddress += parserConfig->wordWidth;
	return currentAddress;

}

int processDirective(struct PARSER_CONFIG* parserConfig,struct TOKEN** statement, struct LABEL *labelTable, int* labelIdx, \
						int tokenIdx, int currentAddress,int value,int passCounter){
	if (!strcmp(statement[tokenIdx]->stringValue,"ORG")){
		if(passCounter && checkOverflow(parserConfig, statement[tokenIdx], value, value)){
			ulog(ERROR,"ORG value in %s:%i is out of specified address range", \
									statement[tokenIdx]->filename,statement[tokenIdx]->lineNumber);
			return -1;
		}
		ulog(INFO,"Setting ORG to %i",value);
		currentAddress = value;
	} else if (!strcmp(statement[tokenIdx]->stringValue,"REMAP")){
		char* labelName = statement[tokenIdx+2]->stringValue;
		*labelIdx = addLabel(labelTable,labelName,value,*labelIdx,passCounter,statement[tokenIdx],1, 0);
		ulog(INFO,"Remapping %s to %s",labelName, statement[tokenIdx+1]->stringValue);
		if(passCounter && *labelIdx == -1){
			ulog(ERROR,"Unable to remap %s to %s",labelName,statement[tokenIdx+1]->stringValue); 
		}
	}
	return currentAddress;
}


/* Prints out the specified label table */
void printLabelTable(struct LABEL *labelTable, int labelTableLen){
	printf("========================== Label Table ===========================\n");
	for(int i=0; i<labelTableLen; i++){
		printf("|%18s : 0x%04x    isRemapped: %i  SubroutineID: %i\n",labelTable[i].name,labelTable[i].value, \
										labelTable[i].isRemapped, labelTable[i].subroutineID);
	}
	printf("==================================================================\n\n");
}

int getMnenomicOpCode(char* mnemonicString){
	for(int index = 0;index<16;index++){
		if (!strcmp(mnemonicString,mnenomicStrings2[index])){
			return index;
		}
	}
	return -1;
}

int getCountBytesInNum(unsigned int num){
	int byteSize = 1;
	while((num >>= 8) > 0){
		byteSize++;
	}
	return byteSize;
}

int getCountBitsInNum(unsigned int num){
	int bitSize = 1;
	while((num >>= 1) > 0){
		bitSize++;
	}
	return bitSize;
}



int getLabelValue(struct LABEL *labelTable, int labelTableLen, char* labelName, int subroutineID){
	int labelIdx = getLabelIdx(labelTable,labelTableLen,labelName,subroutineID);
	if(labelIdx != -1){
		ulog(DEBUG,"Getting Label: %s Value: 0x%02x",labelName,labelTable[labelIdx].value);
		return labelTable[labelIdx].value;
	} else {
		return -1;
	}
}

int getLabelIdx(struct LABEL *labelTable, int labelTableLen, char* labelName, int subroutineID){
	if(labelName[0] != '.'){
			subroutineID = 0;
	}
	for(int i = 0; i < labelTableLen; i++){
		if (!strcmp(labelTable[i].name,labelName) && labelTable[i].subroutineID == subroutineID){
			return i;
		}
	}
	return -1;
}

int addLabel(struct LABEL* labelTable, char* labelName, int labelValue, int labelTableLen, int pass, struct TOKEN* token, \
					int isRemapped, int subroutineID){

	if(labelTableLen > MAX_LABEL_COUNT){
		ulog(ERROR,"Too many Labels: %i",MAX_LABEL_COUNT);
		return -1;
	}
	if(labelName[0] != '.'){
			subroutineID = 0;
	}
	int labelIndex = getLabelIdx(labelTable,labelTableLen,labelName,subroutineID);
	if( labelIndex == -1 || labelTable[labelIndex].pass != pass){
		if( labelIndex == -1){
			labelIndex = labelTableLen;
			labelTableLen++;
		}
		labelTable[labelIndex].name = labelName;
		labelTable[labelIndex].value = labelValue;
		labelTable[labelIndex].pass = pass;
		labelTable[labelIndex].lineNumber = token->lineNumber;
		labelTable[labelIndex].filename = token->filename;
		labelTable[labelIndex].isRemapped = isRemapped;
		labelTable[labelIndex].subroutineID = subroutineID;
		
	} else {
		ulog(ERROR,"Label %s on line %i in file %s:%i is already in use on line %i in file %s:%i",\
						labelTable[labelIndex].name,token->lineNumber,token->filename,token->lineNumber, \
						labelTable[labelIndex].lineNumber,labelTable[labelIndex].filename, \
						labelTable[labelIndex].lineNumber);
		return -1;
	}
	
	return labelTableLen;
}
