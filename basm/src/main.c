#include <stdio.h>

#include "basm.h"

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
        printf("ERROR: Filename to open must be specified.\n");
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
		char binaryArr[32768];
		// Assemble the file
		int binSize = assemble(&options, options.filename, binaryArr);
		if (binSize == -1){
			return 1;
		}
		if(options.onlyTokenize){
			return 0;
		}

        /*******************************************************************************************/

		/*******************************************************************************************/
		//open file[s] | write binary to file[s] | close file[s]
		if(options.splitFile){
			// todo
		} else {
			writeFile("bin/test.asm.bin",binaryArr,binSize);
			printf("Wrote %i bytes",binSize);
		}
		/*******************************************************************************************/
		
		if(options.prettyPrint){
			prettyPrintBytes(binaryArr,binSize-1);
		}
		printf("Wrote file to %s.\n","bin/test.asm.bin");
		return 0;
    }
}