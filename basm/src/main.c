#include <limits.h> 

#include "basm.h"
#include "parser.h"
#include "lexer.h"
#include "../../ulog/include/ulog.h"

// TODO: Handle Split file option

// FEATURE: Support subroutine directive after label
// FEATURE: Output formats: Raw or encode emu parameters
// FEATURE: Support MACROS
// FEATURE: Support REPEAT
// FEATURE: Support . and * for current address
// FEATURE: Support echo for assemble debug info
// FEATURE: MAYBE Support Address LABELS on lines
// FEATURE: Maybe add width and position in assembly file
// FEATURE: Maybe add endianess in assembly file

// FEATURE: Disassembler

// FEATURE: Symbol File
// FEATURE: Listing File

#define initBinaryArr(binaryArr,binSize) clearTokenStringBuffer(binaryArr,binSize) // don't do this

int main(int argc, char* argv[]){
    if (argc == 1){
		printHelp();
        ulog(ERROR,"Filename to open must be specified. Exiting now");
		return 1;
	} else {
		struct OPTIONS sOptions;
		struct TOKEN sTokenArray[MAX_NUM_TOKENS];
		struct FILE_TABLE sFileTable = {{{0}},{0},0};
		
		int tokenArrayLength 	= 0;
		int binSize 			= 0;


		/*******************************************************************************************/
		// Parse Command Line Options and set Defaults
		if(parseCommandLineAndInitOptions(&sOptions,argc,argv)){
			return 1;
		}

		/*******************************************************************************************/
		
		/*******************************************************************************************/
		// Tokenize the file
		if (tokenizeFile(&sOptions, sTokenArray, &sFileTable, &tokenArrayLength)){
			return 1;
		}
		if(sOptions.onlyTokenize){
			printFileTable(&sFileTable);
			printTokens(sTokenArray,tokenArrayLength);
			return 0;
		}

        /*******************************************************************************************/
		
		/*******************************************************************************************/
		// Parse the Tokens generated
		binSize = parseTokens(&sOptions, sTokenArray, tokenArrayLength);
		if (!binSize){
			return 1;
		}
		/*******************************************************************************************/
		
		/*******************************************************************************************/
		// Write buffer to Binary Array in order
		if(sOptions.align && binSize < sOptions.alignValue){
				binSize = sOptions.alignValue;
		}
		char binaryArr[binSize];
		initBinaryArr(binaryArr,binSize);
		int byteWriteCounter = 0;
		for (int i=0;i<tokenArrayLength;i++){
			if (sTokenArray[i].size){
				writeByteToArray(binaryArr,&sOptions,sTokenArray[i].address,sTokenArray[i].numericValue);
				byteWriteCounter += sOptions.wordWidth;
			}
		}

		/*******************************************************************************************/

		/*******************************************************************************************/
		//open file[s] | write binary to file[s] | close file[s]
		if(sOptions.splitFile){
			// todo
		} else {
			writeFile(sOptions.outFilePath,binaryArr,binSize);
			ulog(INFO,"Wrote %i bytes",byteWriteCounter);
		}
		/*******************************************************************************************/
		
		if(sOptions.prettyPrint){
			prettyPrintBytes(binaryArr,binSize-1);
		}
		printf("Wrote file to %s.\n",sOptions.outFilePath);
		return 0;
    }
}