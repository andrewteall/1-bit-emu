#include <stdio.h>
#include <string.h>
#include <limits.h> 
#include <unistd.h>  // getcwd
// #include <ctype.h>

#include "parser.h"
#include "utils.h"
#include "token.h"
#include "../../ulog/include/ulog.h"

const char *mnenomicStrings2[] = {
	"NOPO","LD","LDC","AND","ANDC","OR","ORC","XNOR","STO","STOC","IEN","OEN","JMP","RTN","SKZ","NOPF","NULL"};

struct LABEL {
	char* name;
	int   value;
	int   pass;
	int   lineNumber;
	char* filename;
	int   isRemapped;
	int   subroutineID;
};

void checkForRemapping(int* statement,struct LABEL* labelTable,int labelTableLen,char* tokenString){
	if (statement[0] == LABEL){
		int label = getLabelIdx(labelTable,labelTableLen,tokenString,0);
		if (label != -1 && labelTable[label].isRemapped){
			statement[0] = MNENOMIC;
		}
	}
}

int parseTokens(struct OPTIONS* sOptions, struct TOKEN* tokenArr, int tokenizedArraySize){
	ulog(INFO,"Starting Parser");
	int error = 0;
	int labelTableLen = 0;
	int maxAddress = 0;
	struct LABEL labelTable[MAX_LABEL_COUNT];
	
	for (int passCounter = 0; passCounter < 2; passCounter++){
		int currentAddress = 0;
		int statementLength = 0;
		int inSubroutine = 0;
		int subroutineCounter = 0;

		ulog(INFO,"Starting Pass %i",passCounter+1);
		for (int i = 0; i<tokenizedArraySize;i++){
			int statement[MAX_STATEMENT_LENGTH] = {0};
			while(tokenArr[i].type != NEWLINE){
				statement[statementLength++] = tokenArr[i].type; 
				i++;
			}
			statement[statementLength] = tokenArr[i].type;

			int tokenIdx = i - statementLength;

			checkForRemapping(statement,labelTable,labelTableLen,tokenArr[tokenIdx].stringValue);
			
			if ((statement[0] == MNENOMIC  &&  statement[1] == NUMBER  && statement[2] == NEWLINE)  || \
			    (statement[0] == MNENOMIC  &&  statement[1] == LABEL   && statement[2] == NEWLINE)  || \
				(statement[0] == MNENOMIC  &&  statement[1] == NEWLINE)  						    || \

				(statement[0] == MNENOMIC  && (statement[1] == NUMBER || statement[1] == LABEL)     && \
				 statement[2] == LABEL_MOD &&  (statement[3] == NUMBER  || statement[3] == LABEL)   && \
				 statement[4] == NEWLINE)){ 
				
				int  subroutineID = 0;
				int  statementError = 0;
				int  operand = -1;
				int  opcode = -1;
				char errorMsg[255] = {'\0'};
				int  errorToken = 0;
				
				if(tokenArr[tokenIdx].type == LABEL){
					opcode = getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx].stringValue,0);
				} else {
					opcode = getMnenomicOpCode(tokenArr[tokenIdx].stringValue);
				}
				
				if(opcode == -1){
					// Should never get here becasue we already to checked to see if this label exists
					strcpy(errorMsg,"Invalid Opcode");
					errorToken = 0;
					statementError = 1;
				}
				
				switch (statement[1]){
				case NUMBER:
					operand = str2num(tokenArr[tokenIdx+1].stringValue);
					if(passCounter && operand == -1){
						strcpy(errorMsg,"Not a Number");
						errorToken = 1;
						statementError = 1;
					}
					break;
				case LABEL:
					subroutineID = inSubroutine * subroutineCounter;
					operand = getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+1].stringValue,subroutineID);
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

				switch (statement[2] + statement[3]){
				case LABEL_MOD+NUMBER:
					if(!strcmp(tokenArr[tokenIdx+2].stringValue,"+")){
						operand += str2num(tokenArr[tokenIdx+3].stringValue);
					} else if(!strcmp(tokenArr[tokenIdx+2].stringValue,"-")){
						operand -= str2num(tokenArr[tokenIdx+3].stringValue);
					} else if(!strcmp(tokenArr[tokenIdx+2].stringValue,"*")){
						operand *= str2num(tokenArr[tokenIdx+3].stringValue);
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
					if(!strcmp(tokenArr[tokenIdx+2].stringValue,"+")){
						operand += getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+3].stringValue,subroutineID);
					} else if(!strcmp(tokenArr[tokenIdx+2].stringValue,"-")){
						operand -= getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+3].stringValue,subroutineID);
					} else if(!strcmp(tokenArr[tokenIdx+2].stringValue,"*")){
						operand *= getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+3].stringValue,subroutineID);
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
					ulog(ERROR,"%s: %s in file %s:%i",errorMsg, tokenArr[tokenIdx+errorToken].stringValue, \
					tokenArr[tokenIdx+errorToken].filename, tokenArr[tokenIdx+errorToken].lineNumber);
					error = 1;
				}

				currentAddress = processMnemonic(sOptions,&tokenArr[tokenIdx],currentAddress,opcode,operand,passCounter);

			} else if (statement[0] == DIRECTIVE && \
					 ((statement[1] == LABEL    && statement[2] == NEWLINE) 							|| \
					  (statement[1] == NUMBER   && statement[2] == NEWLINE) 							|| \
					  (statement[1] == MNENOMIC && statement[2] == LABEL && statement[3] == NEWLINE) 	|| \
					  (statement[1] == NEWLINE))){
				
				int directiveValue = 0;
				int subroutineID;
				char* directiveString = tokenArr[tokenIdx].stringValue;

				switch (statement[1]){
				case LABEL:
					subroutineID = inSubroutine * subroutineCounter;
					directiveValue = getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+1].stringValue,subroutineID);
					break;
				case NUMBER:
					directiveValue = str2num(tokenArr[tokenIdx+1].stringValue);
					break;
				case MNENOMIC:
					directiveValue = getMnenomicOpCode(tokenArr[tokenIdx+1].stringValue);
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
					ulog(ERROR,"Label does not exist: %s in file %s:%i", tokenArr[tokenIdx+1].stringValue, \
										tokenArr[tokenIdx+1].filename ,tokenArr[tokenIdx+1].lineNumber);
					error = 1;

				}
				currentAddress = processDirective(sOptions,tokenArr,labelTable,&labelTableLen,tokenIdx, \
													currentAddress,directiveValue,passCounter);

				

			} else if ((statement[0] == LABEL && statement[1] == NEWLINE) || \
			(statement[0] == LABEL && statement[1] == ASSIGNMENT && statement[2] == NUMBER && statement[3] == NEWLINE) || \
			(statement[0] == LABEL && statement[1] == ASSIGNMENT && statement[2] == LABEL && statement[3] == NEWLINE)  || \
			(statement[0] == LABEL && statement[1] == ASSIGNMENT && (statement[2] == NUMBER || statement[2] == LABEL)  && \
			 statement[3] == LABEL_MOD && (statement[4] == LABEL || statement[4] == NUMBER) && statement[5] == NEWLINE)){
				int subroutineID;
				int labelValue = 0;
				struct TOKEN* token;
				char* labelName = tokenArr[tokenIdx].stringValue;
				
				switch (statement[2]){
				case NUMBER:
					labelValue = str2num(tokenArr[tokenIdx+2].stringValue);
					token = &tokenArr[tokenIdx+2];
					break;
				case LABEL:
					subroutineID = inSubroutine * subroutineCounter;
					labelValue = getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+2].stringValue,subroutineID);
					token = &tokenArr[tokenIdx+2];
					break;
				default:
					labelValue = currentAddress;
					tokenArr[i].address = currentAddress;
					token = &tokenArr[tokenIdx];
					break;
				}

				switch (statement[3] + statement[4]){
				case LABEL_MOD+NUMBER:
					if(!strcmp(tokenArr[tokenIdx+3].stringValue,"+")){
						labelValue += str2num(tokenArr[tokenIdx+4].stringValue); 
					} else if(!strcmp(tokenArr[tokenIdx+3].stringValue,"-")){
						labelValue -= str2num(tokenArr[tokenIdx+4].stringValue); 
					} else if(!strcmp(tokenArr[tokenIdx+3].stringValue,"*")){
						labelValue *= str2num(tokenArr[tokenIdx+4].stringValue); 
					} else {
						ulog(ERROR,"Invalid Label Modifier: %s in file %s:%i",tokenArr[tokenIdx+3].stringValue, \
									tokenArr[tokenIdx+3].filename, tokenArr[tokenIdx+3].lineNumber);
						error = 1;
					}
					break;

				case LABEL_MOD+LABEL:
					if(!strcmp(tokenArr[tokenIdx+3].stringValue,"+")){
						labelValue += getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+4].stringValue,subroutineID);
					} else if(!strcmp(tokenArr[tokenIdx+3].stringValue,"-")){
						labelValue -= getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+4].stringValue,subroutineID);
					} else if(!strcmp(tokenArr[tokenIdx+3].stringValue,"*")){
						labelValue *= getLabelValue(labelTable,labelTableLen,tokenArr[tokenIdx+4].stringValue,subroutineID);
					} else {
						ulog(ERROR,"Invalid Label Modifier: %s in file %s:%i",tokenArr[tokenIdx+3].stringValue, \
									tokenArr[tokenIdx+3].filename, tokenArr[tokenIdx+3].lineNumber);
						error = 1;
					}
					break;
				}

				if(passCounter && checkOverflow(sOptions, token, 0, labelValue)){
					return -1;
				}
				int subroutine = inSubroutine * subroutineCounter;
				labelTableLen = addLabel(labelTable,labelName,labelValue,labelTableLen,passCounter,&tokenArr[tokenIdx],0, subroutine);

				
				if (labelTableLen == -1){
					return -1;
				}
			

			} else if (statement[0] == INCLUDE && statement[1] == LABEL && statement[2] == NEWLINE){
				//Ignore includes here
			} else if (statement[0] == NEWLINE){
				//Ignore blank lines
			} else {
				if(passCounter){
					char errorStatementStr[255];
					for(int j=0; tokenIdx+j != i;j++){
						strcat(errorStatementStr,tokenArr[tokenIdx+j].stringValue);
						strcat(errorStatementStr," ");
					}
				
					ulog(ERROR,"Invalid Statement %sin File: %s:%i ", \
												errorStatementStr,tokenArr[i].filename,tokenArr[i].lineNumber);
					memset(errorStatementStr,0,255);
					error = 1;
				}
			}
			
			if(currentAddress == -1){
				return -1;
			}

			statementLength = 0;
			if ((currentAddress+tokenArr[i].size) > maxAddress){
				maxAddress = currentAddress+tokenArr[i].size;
			}
		}
		if(sOptions->printLabelTable){
			printLabelTable(labelTable,labelTableLen);
		}
	}

	if(sOptions->parsePrint){
		printTokens(tokenArr,tokenizedArraySize);
	}
	
	if(error){
		ulog(ERROR,"Error occurred... exiting");
	} else {
		ulog(DEBUG,"Binary file length: %i bytes",maxAddress);
	}
	return (maxAddress * !error);
}

int checkOverflow(struct OPTIONS* sOptions, struct TOKEN* token,int value,int address){
	int overflow = 0;
	if(getCountBytesInNum(value) > sOptions->wordWidth){
		ulog(ERROR,"Overflow word width: %i in file: %s:%i",value,token->filename,token->lineNumber);
		overflow = 1;
	}
	if(getCountBitsInNum(address) > sOptions->addressWidth){
		ulog(ERROR,"Overflow address width: %i in file %s:%i",address,token->filename,token->lineNumber);
		overflow = 1;
	}
	return overflow;
}

int processMnemonic(struct OPTIONS* sOptions, struct TOKEN* token, int currentAddress,int opcode,int address, int pass){
	int opcodePositionVal = opcode << (((sOptions->wordWidth*8)-sOptions->instructionWidth)-sOptions->instructionPosition);
	int addressPositionVal = address << (((sOptions->wordWidth*8)-sOptions->addressWidth)-sOptions->addressPosition);

	int value = (opcodePositionVal | addressPositionVal);
	
	if(pass && address != -1 && checkOverflow(sOptions, token, value, address)){
		return -1;
	}
	
	token->numericValue = value;
	token->address = currentAddress;
	token->size = sOptions->wordWidth;

	currentAddress += sOptions->wordWidth;
	return currentAddress;

}

int processDirective(struct OPTIONS* sOptions,struct TOKEN* tokenArr, struct LABEL *labelTable, int* labelIdx, \
						int tokenIdx, int currentAddress,int value,int passCounter){
	if (!strcmp(tokenArr[tokenIdx].stringValue,"ORG")){
		if(passCounter && checkOverflow(sOptions, &tokenArr[tokenIdx], value, value)){
			ulog(ERROR,"ORG value in %s:%i is out of specified address range", \
									tokenArr[tokenIdx].filename,tokenArr[tokenIdx].lineNumber);
			return -1;
		}
		ulog(INFO,"Setting ORG to %i",value);
		currentAddress = value;
	} else if (!strcmp(tokenArr[tokenIdx].stringValue,"REMAP")){
		char* labelName = tokenArr[tokenIdx+2].stringValue;
		*labelIdx = addLabel(labelTable,labelName,value,*labelIdx,passCounter,&tokenArr[tokenIdx],1, 0);
		ulog(INFO,"Remapping %s to %s",labelName, tokenArr[tokenIdx+1].stringValue);
		if(passCounter && *labelIdx == -1){
			ulog(ERROR,"Unable to remap %s to %s",labelName,tokenArr[tokenIdx+1].stringValue); 
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
