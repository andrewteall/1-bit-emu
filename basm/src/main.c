#include <stdio.h>

#include "basm.h"
#include "parser.h"
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

int main(int argc, char* argv[]){
    if (argc == 1){
		printHelp();
        ulog(ERROR,"Filename to open must be specified. Exiting now");
		return 1;
	} else {
		struct OPTIONS options;
		
		/*******************************************************************************************/
		// Set Defaults and Parse Command Line Options
		setDefaultOptions(&options);
		if(parseCommandLine(&options,argc,argv)){
			return 1;
		}

		/*******************************************************************************************/
		
		/*******************************************************************************************/
		// Assemble the file
		int binSize = assemble(&options, options.filename);
		if (binSize == -1){
			return 1;
		}
		if(options.onlyTokenize){
			return 0;
		}

        /*******************************************************************************************/
		
		
		
		/*******************************************************************************************/
		// Write buffer to Binary Array in order
		if(options.align && binSize < options.alignValue){
				binSize = options.alignValue;
		}
		char binaryArr[binSize];
		
		int byteWriteCounter = 0;
		// for (int i=0;i<tokenArrayLength;i++){
		// 	if (sTokenArray[i].size){
		// 		writeByteToArray(binaryArr,&options,sTokenArray[i].address,sTokenArray[i].numericValue);
		// 		byteWriteCounter += options.wordWidth;
		// 	}
		// }

		/*******************************************************************************************/

		/*******************************************************************************************/
		//open file[s] | write binary to file[s] | close file[s]
		if(options.splitFile){
			// todo
		} else {
			writeFile(options.outFilePath,binaryArr,binSize);
			ulog(INFO,"Wrote %i bytes",byteWriteCounter);
		}
		/*******************************************************************************************/
		
		if(options.prettyPrint){
			prettyPrintBytes(binaryArr,binSize-1);
		}
		printf("Wrote file to %s.\n",options.outFilePath);
		return 0;
    }
}