#include <stdio.h>
#include <string.h>

#include "basm.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"
#include "utils.h"
#include "ulog.h"

int assemble(struct OPTIONS* options, char* filename, char* binaryArr){
	struct TOKEN_LIST tokenList;
	struct TOKENIZER_CONFIG tokenizerConfig;
	tokenizerConfig.maxFileDepth = options->maxFileDepth;

	int numTokens = tokenizeFile(filename, &tokenList, &tokenizerConfig);
	if(options->onlyTokenize){
		printFileTable(&tokenizerConfig.fileTable);
		printTokens(&tokenList);
		return 0;
	}

/******************************************************************************/
	/* Parse the Tokens generated */
	struct PARSER_CONFIG parserConfig;
	parserConfig.instructionWidth = options->instructionWidth;
	parserConfig.addressWidth = options->addressWidth;
	parserConfig.instructionPosition = options->instructionPosition;
	parserConfig.addressPosition = options->addressPosition;
	parserConfig.wordWidth = options->wordWidth;
	parserConfig.printLabelTable = options->printLabelTable;
	
	int binSize = parseTokens(&parserConfig, &tokenList);
	if (!binSize){
		return -1;
	}
	if(options->parsePrint){
		printTokens(&tokenList);
	}
/******************************************************************************/

/*******************************************************************************************/
	// Write buffer to Binary Array in order
	if(options->align && binSize < options->alignValue){
			binSize = options->alignValue;
	}
	
	int byteWriteCounter = 0;
	for (int i=0;i<numTokens;i++){
		if (tokenList.list[i].size){
			writeByteToArray(binaryArr,options,tokenList.list[i].address,tokenList.list[i].numericValue);
			byteWriteCounter += options->wordWidth;
		}
	}

/*******************************************************************************************/

	return binSize;
}

int writeByteToArray(char* array, struct OPTIONS* options, int position, int value){
	while(value >= 0){
		if(value < 256){
			switch (options->endianess){
			case LITTLE_ENDIAN:
				array[position] = value;
				position++;
				break;
			case BIG_ENDIAN:
				array[position+options->wordWidth-1] = value;
				position--;
				break;
			}
			break;
		} else {
			position = writeByteToArray(array,options,position,(value & 0b11111111));
			value >>= 8;
		}
	}

	return position;
}

char writeFile(char *path, char *bytes, size_t size){
    FILE *file = fopen(path, "wb");
    if (file == NULL){
        return 1;
	}

    fwrite(bytes, 1, size, file);
    fclose(file);

    return 0;
}

void prettyPrintBytes(char* array, int size){
	printf("\n       00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F\n");
	printf("       ===============================================================\n");
	for (int i=0;i <=size;i++){
		printf("%04x | ",i);
		int j=0;
		while(j<16 && i<=size){
			printf("%02x  ",array[i++]);
			j++;
		}
		i--;
		printf("\n");
	}
}


/******************************************************************************/
/******************************** Init & Help *********************************/
/******************************************************************************/
/* Set Default Options */
void setDefaultOptions(struct OPTIONS* options){
	options->instructionWidth  		= 4;
	options->addressWidth     		= 4;
	options->instructionPosition    = 0;
	options->addressPosition    	= 4;
	options->wordWidth				= 8;

	options->endianess          	= LITTLE_ENDIAN;
	options->splitFile       		= 0;
	options->interlaceFile  		= 0;
	options->maxFileDepth			= 10;

	options->format					= 0;
	options->onlyTokenize			= 0;
	options->prettyPrint			= 0;
	options->printLabelTable		= 0;
	options->parsePrint				= 0;

	options->align					= 0;
	options->alignValue				= 0;
	
	// strcpy(options->outFilePath,options->filename);
	// strcat(options->outFilePath,".bin");

}

/* Set the Word Width in Bits*/
int setWordWidth(struct OPTIONS* options){
	float instructionWidth =(float)options->instructionWidth;
	float addressWidth =(float)options->addressWidth;
	float instructionPosition =(float)options->instructionPosition;
	float addressPosition =(float)options->addressPosition;
	int wordWidth = 0;

	if(instructionPosition < addressPosition){
		if (instructionWidth+instructionPosition > addressPosition){
			ulog(ERROR,"Instruction overlaps Address");
			return 1;
		}
		wordWidth = roundUp((addressWidth+addressPosition) / 8.0);

	} else if (instructionPosition > addressPosition){
		if (addressWidth+addressPosition > instructionPosition){
			ulog(ERROR,"Address overlaps Instruction");
			return 1;
		}
		wordWidth = roundUp((instructionWidth+instructionPosition) / 8.0);
	} else {
		ulog(ERROR,"Address and Instruction Position cannot be equal");
		return 1;
	}
	
	options->wordWidth = wordWidth;
	ulog(INFO,"Setting Word Width to: %i",options->wordWidth);
	return 0;
}

/* Parse Command Lin Options */
int parseCommandLine(struct OPTIONS* sOptions,int argc, char* argv[]){
	sOptions->filename = argv[argc-1];
	for(int i=argc-1;i>0;i--){
		// -h --help
		if (!(strcmp(argv[i],"-h")) || (!strcmp(argv[i],"--help"))){
			printHelp();
			return 1;
		}

		//-v --vvvv
		if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"--v") || !strcmp(argv[i],"--vv") || \
			!strcmp(argv[i],"--vvv") || !strcmp(argv[i],"--vvvv") || !strcmp(argv[i],"--vvvvv") || \
			!strcmp(argv[i],"--vvvvvv")){
			int verbosity = 0;
			if (!strcmp(argv[i],"-v")){
				verbosity = str2num(argv[i+1]);
			} else {
				verbosity = strlen(argv[i])-2;
			}
			if (verbosity >= OFF && verbosity <= TRACE){
				setLoggingLevel(verbosity);
			} else {
				ulog(ERROR,"Unsupported verbosity value");
				return 1;
			}
		}

		//-f --filename
		if (!(strcmp(argv[i],"-f")) || (!strcmp(argv[i],"--filename"))){
			sOptions->filename = argv[i+1];
		}
	}

	for(int i=argc-1;i>0;i--){
		// -aw --address-width
		if (!strcmp(argv[i],"-aw") || !strcmp(argv[i],"--address-width")){
			sOptions->addressWidth = str2num(argv[i+1]);
				if ( sOptions->addressWidth == -1){
				ulog(FATAL,"Unsupported Address Width value");
				return 1;
				} else {
				ulog(INFO,"Setting Address Width to %i",str2num(argv[i+1]));
				}
		}

		// -iw --instruction-width
		if (!strcmp(argv[i],"-iw") || !strcmp(argv[i],"--instruction-width")){
			sOptions->instructionWidth = str2num(argv[i+1]);
			if ( sOptions->instructionWidth == -1 || sOptions->instructionWidth < 4){
				ulog(FATAL,"Unsupported Instruction Width value. Minimum value is 4");
				return 1;
			} else {
				ulog(INFO,"Setting Instruction Width to %i",str2num(argv[i+1]));
			}
		}

		// -e --endianess
		if (!strcmp(argv[i],"-e") || !strcmp(argv[i],"--endianess")){
			if (!strcmp(argv[i+1],"L") || !strcmp(argv[i+1],"l")){
				sOptions->endianess = LITTLE_ENDIAN;
			} else if (!strcmp(argv[i+1],"B") || !strcmp(argv[i+1],"b")){
				sOptions->endianess = BIG_ENDIAN;
			} else{
				ulog(FATAL,"Unsupported Endianess Width value. L or B");
				return 1;
			}
			ulog(INFO,"Setting Endianess to %s",argv[i+1]);
		}

		// -f --format
		if (!strcmp(argv[i],"-f") || !strcmp(argv[i],"--format")){
			sOptions->format = str2num(argv[i+1]);
			if ( sOptions->format == -1 || sOptions->format > 1){
				ulog(FATAL,"Unsupported Format value. Must be a number and between 0 and 1.");
				return 1;
			}
			ulog(INFO,"Setting output format to %i",sOptions->format);
		}

		// -i --interlace
		// May not be needed
		// if (!strcmp(argv[i],"-i") || !strcmp(argv[i],"--interlace")){
		// 	sOptions->interlaceFile = 1;
		// 	ulog(INFO,"Interlacing Output File");
		// }

		// -o --outfile
		if (!strcmp(argv[i],"-o") || !strcmp(argv[i],"--outfile")){
			sOptions->outFilePath = argv[i+1];
			ulog(INFO,"Setting Output File name to %s",argv[i+1]);
		}

		// -ip --instruction-position
		if (!strcmp(argv[i],"-ip") || !strcmp(argv[i],"--instruction-position")){
			sOptions->instructionPosition = str2num(argv[i+1]);
			if ( sOptions->instructionPosition == -1){
				ulog(FATAL,"Unsupported Instruction Position value. Must be a number.");
				return 1;
			}
			ulog(INFO,"Setting Instruction Position to %i",str2num(argv[i+1]));
		}

		// -ap --address-position
		if (!strcmp(argv[i],"-ap") || !strcmp(argv[i],"--address-position")){
			sOptions->addressPosition = str2num(argv[i+1]);
			if ( sOptions->addressPosition == -1){
				ulog(FATAL,"Unsupported Address Position value. Must be a number.");
				return 1;
			}
			ulog(INFO,"Setting Address Position to %i",str2num(argv[i+1]));
		}

		// -s --split-file
		if (!strcmp(argv[i],"-s") || !strcmp(argv[i],"--split-file")){
			sOptions->interlaceFile = 0;
			sOptions->splitFile = 0;
			ulog(INFO,"Splitting Output File");
		}

		// -m --max-include-depth
		if (!strcmp(argv[i],"-m") || !strcmp(argv[i],"--max-include-depth")){
			sOptions->maxFileDepth = str2num(argv[i+1]);
			if ( sOptions->maxFileDepth == -1){
				ulog(FATAL,"Unsupported Max Include Depth value. Must be a number.");
				return 1;
			}
			ulog(INFO,"Setting Max Include Depth to %i",sOptions->maxFileDepth);
		}

		// -t --tokenize-only
		if (!strcmp(argv[i],"-t") || !strcmp(argv[i],"--tokenize-only")){
			sOptions->onlyTokenize = 1;
			ulog(INFO,"Only reading file and generating tokens.");
		}

		// -p --pretty-print
		if (!strcmp(argv[i],"-p") || !strcmp(argv[i],"--pretty-print")){
			sOptions->prettyPrint = 1;
			ulog(INFO,"Pretty Printing binary to Std Out");
		}

		// --print-parsed
		if (!strcmp(argv[i],"--print-parsed")){
			sOptions->parsePrint = 1;
			ulog(INFO,"Print tokens after parsing");
		}

		// -l --label-table
		if (!strcmp(argv[i],"-l") || !strcmp(argv[i],"--label-table")){
			sOptions->printLabelTable = 1;
			ulog(INFO,"Printing Label Table to Std Out");
		}

		// -a --align
		if (!strcmp(argv[i],"-a") || !strcmp(argv[i],"--align")){
			sOptions->align = 1;
			sOptions->alignValue = str2num(argv[i+1]);
			if ( sOptions->alignValue == -1){
				ulog(FATAL,"Unsupported alignment value. Must be a number.");
				return 1;
			}
			ulog(INFO,"Aligning binary to %i bytes",str2num(argv[i+1]));
		}
	}


	// Setup Word width
	if (setWordWidth(sOptions)){
		return 1;
	}

	return 0;
}

/* Prints help message */
void printHelp(){
    printf("Usage: basm [options] file\n");
    printf("Options:\n");
    printf(" -a N,         --align N                    Align or pad the binary with 0 to a specified size N.\n");
    printf("-ap N,         --address-position N         Sets the position of the address within the memory from the MSB. Default: 4\n");
    printf("-aw N,         --address-width N            Set the width in bits of the address. Default: 4\n");
    printf(" -e [L|B],     --endianess [L|B]            Set endianess for output file. Default: Little Endian\n");
    printf(" -f FILE,      --filename FILE              Specify to the file to be assembled. Otherwise the last argument is used.\n");
    printf(" -h,           --help                       Print this message and exit.\n");
    printf("-ip N,         --instruction-position N     Sets the position of the instruction within the memory from the MSB. Default: 0\n");
    printf("-iw N,         --instruction-width N        Set the width in bits of the instruction. Default: 4\n");
    printf(" -l,           --lable-table                Print Label Table.\n");
    printf(" -m N,         --max-include-depth N        Maximum depth of files that can be included. Default: 10\n");
    printf(" -o FILE,      --outfile FILE               Specify output file. Default: $file.bin\n");
    printf("               --print-parsed               Prints tokens after parsing.\n");
    printf(" -p,           --pretty-print               Pretty print binary file at completion.\n");
    printf(" -s,           --split-file                 Makes two seperate files one with Opcodes and one with Operands.\n");
    printf(" -t,           --tokenize-only              Only read the file and generate tokens.\n");
    printf(" -v N,         --v[vvvv]                    Set the log verbosity to N, 0=OFF, 1=FATAL, 2=ERROR, 3=WARNING, 4=INFO, 5=DEBUG.\n");
    printf("\n");
}
