#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "bemu.h"
#include "debugger.h"
#include "IODevice.h"
#include "MC14500.h"
#include "MCSystem.h"
#include "ulog.h"
#include "utils.h"

int run(struct OPTIONS* sOptions){
	int error = 0;
	/*********************************************************************/
	/********************** Setup Program Counter ************************/
	// Setup Program Counter 
	uint16_t pc = sOptions->pcInitAddress-sOptions->wordWidth;
	/*********************************************************************/

	/*********************************************************************/
	/*************************** Setup ICU *******************************/
	struct MC14500 icu;
	initICU(&icu);
	// setPinHandlers(&sOptions->pinHandles, &icu.jmpPin, &icu.rtnPin, &icu.flagOPin,&icu.flagFPin);

	struct PIN_HANDLES pinHandles;
	pinHandles.jmpPinHandler = sOptions->jmpPinHandler;
    pinHandles.rtnPinHandler = sOptions->rtnPinHandler;
    pinHandles.flagFPinHandler = sOptions->flagFPinHandler;
    pinHandles.flagOPinHandler = sOptions->flagOPinHandler;

	struct SIGNALS signals;
	/*********************************************************************/

	/*********************************************************************/
	/*************************** Setup ROM *******************************/
	// Define the Read Only Memory to be used by the ICU
	uint32_t programROM[sOptions->romSize];
	// Program Rom
	error += !error * programROMFromFile(programROM, sOptions->filename, sOptions->romSize);
	
	/*********************************************************************/

	/*********************************************************************/
	/************************* Setup Stack *******************************/
	struct STACK stack;
	uint32_t stackData[sOptions->stackSize];
	initStack(&stack, stackData, sOptions->stackSize, sOptions->stackDir, sOptions->stackWidth);
	/*********************************************************************/

	/*********************************************************************/
	/************************** Setup I/O ********************************/
	// Array to store input and output devices and their addresses
	struct IODevice deviceList[sOptions->ioDeviceCount];
	initIODeviceList(deviceList,sOptions->ioDeviceCount);

	if (sOptions->bindResultsRegister){
		error += !error * \
			bindResultRegisterPinToIOAddress(deviceList,sOptions->rrDeviceAddress,icu.resultsRegisterPin,sOptions->ioDeviceCount);
	}
	
	// struct IODevice ioDeviceList[0xF];
	// struct IODevice inputDeviceList[0xF];
	// struct IODevice outputDeviceList[0xF];
	/*********************************************************************/
	
	if(sOptions->enableDebugger && !error){
	    error += !error * startDebugger(sOptions);
	}

	uint32_t address;
	uint8_t instruction;
	uint32_t programROMValue;
	startICU(&icu);
	while(icu.status == RUNNING && !error){
		// Fetch - Clock Up
		pc += getPCIncrement(icu.jmpPin, icu.rtnPin, icu.flagFPin, icu.flagOPin, sOptions->wordWidth); // 0 if we modify the PC outside of this line
		programROMValue = readWordFromROM(programROM, pc, sOptions->wordWidth, sOptions->endianess);
		address = decodeAddress(programROMValue, sOptions->wordWidth, sOptions->instructionWidth, sOptions->instructionPosition);
		instruction = decodeInstruction(programROMValue, sOptions->wordWidth, sOptions->instructionWidth, sOptions->instructionPosition);
		// icu "fetch"
		fetch(&icu, instruction);
			
		// Execute - Clock Down
		// Latch device at address to Data Pin
		latchIODeviceValueToDataPin(deviceList,address,&icu.dataPin,sOptions->ioDeviceCount);
		// icu "execute"
		execute(&icu);
		// Latch Data Pin out to device
		latchDataPinToIODevice(deviceList,address,&icu.dataPin,icu.writePin,sOptions->ioDeviceCount);
		
		if(sOptions->enableDebugger){
			drawScreen(sOptions,pc, address, &icu,deviceList,stack.data,&stack.sp,programROM,&pinHandles);
		} else if(sOptions->printState){
			printSystemInfo(pc, address, &icu);
		}
		
		// User Defined Pin Handling
		uint8_t handlerForPin = getActivePinAndHandler(icu.jmpPin, icu.rtnPin, icu.flagOPin, icu.flagFPin, &pinHandles);
		error += !error * pinHandler(handlerForPin, &stack, &pc, address, &signals);

		// (pc >= sOptions->romSize) ? pc = 0 : 0; 
		pc *= !(pc >= sOptions->romSize); // Reset PC if we overflow the "ROM"

		if(signals.stopSignal){
			stopICU(&icu);
		}
		if(signals.resetSignal){
			resetICU(&icu);
		}
		clearSignals(&signals);
	}

	if(sOptions->enableDebugger){
	    stopDebugger(error);
	}
	
	return error;
}

/*****************************************************************************/
/***************************** Program Admin *********************************/
/*****************************************************************************/
void printSystemInfo(uint16_t pc, uint32_t address, struct MC14500* icu){
	printf("PC = 0x%02x  Inst = %4s(0x%02x)  Addr = 0x%02x  LU = %i  RR = %i  IEN/OEN = %i/%i  " \
			"Write: %i  Data: %i  Skip Next: %i  JROF: %i%i%i%i\n",
			pc,mnenomicStrings[icu->instruction],icu->instruction,address,icu->logicUnit,icu->resultsRegister,icu->ienRegister, \
			icu->oenRegister, icu->writePin,icu->dataPin,icu->skipRegister,icu->jmpPin,icu->rtnPin,icu->flagOPin,icu->flagFPin);
}

int  setWordWidth(struct OPTIONS* sOptions){
	float instructionWidth =(float)sOptions->instructionWidth;
	float addressWidth =(float)sOptions->addressWidth;
	float instructionPosition =(float)sOptions->instructionPosition;
	float addressPosition =(float)sOptions->addressPosition;
	uint8_t wordWidth = 0;

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
	
	if (wordWidth > 4){
		ulog(ERROR,"Invalid word size: %i. Word size cannot be greater than 4",wordWidth);
		return 1;
	} else {
		sOptions->wordWidth = wordWidth;
		ulog(INFO,"Setting Word Width to: %i",sOptions->wordWidth);
		return 0;
	}
}

void setDefaultOptions(struct OPTIONS* sOptions){
    sOptions->instructionWidth      = 4;
    sOptions->addressWidth          = 4;
    sOptions->instructionPosition   = 0;
    sOptions->addressPosition       = 4;

    sOptions->romSize               = 0xFF;
    // options->romSize                = expo(2,options->addressWidth);
    sOptions->stackSize             = 0xFF;
    sOptions->stackDir              = DOWN;
    sOptions->stackWidth            = 8;

    sOptions->ioDeviceCount         = 0x0F;
    sOptions->bindResultsRegister   = 0;
    sOptions->rrDeviceAddress       = 0;

    sOptions->endianess             = LITTLE_ENDIAN;
    sOptions->splitFile             = 0;

    sOptions->enableDebugger        = 0;

    // sOptions->pinHandles.pinSink = 0;
    // sOptions->pinHandles.jmpPinPtr = &sOptions->pinHandles.pinSink;
    // sOptions->pinHandles.rtnPinPtr = &sOptions->pinHandles.pinSink;
    // sOptions->pinHandles.flagFPinPtr = &sOptions->pinHandles.pinSink;
    // sOptions->pinHandles.flagOPinPtr = &sOptions->pinHandles.pinSink;

    // sOptions->pinHandles.jmpPinHandler = NONE;
    // sOptions->pinHandles.rtnPinHandler = NONE;
    // sOptions->pinHandles.flagFPinHandler = NONE;
    // sOptions->pinHandles.flagOPinHandler = NONE;

	sOptions->jmpPinHandler = NONE;
    sOptions->rtnPinHandler = NONE;
    sOptions->flagFPinHandler = NONE;
    sOptions->flagOPinHandler = NONE;

    sOptions->pcInitAddress = 0;
    sOptions->printState = 0;
    sOptions->stepMode = 0;

}

int  parseCommandLineOptions(struct OPTIONS* sOptions,int argc, char* argv[]){
	setDefaultOptions(sOptions);

	sOptions->filename = argv[argc-1];
	for(int i=argc-1;i>0;i--){
		// -h --help
		if (!(strcmp(argv[i],"-h")) || (!strcmp(argv[i],"--help"))){
			printUsage();
			return 1;
		}

		//-v -vvvv
		if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"--v") || !strcmp(argv[i],"--vv") \
			|| !strcmp(argv[i],"--vvv") || !strcmp(argv[i],"--vvvv")|| !strcmp(argv[i],"--vvvvv")){
			int verbosity = 0;
			if (!strcmp(argv[i],"-v")){
				verbosity = str2num(argv[i+1]);
			} else {
				verbosity = strlen(argv[i])-2;
			}
			if (verbosity >= 0 && verbosity <= 5){
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
		// if (!strcmp(argv[i],"-s") || !strcmp(argv[i],"--split-file")){
		// 	sOptions->splitFile = 0;
		// 	ulog(INFO,"Splitting Output File");
		// }

        // -r --rom-size
		if (!strcmp(argv[i],"-r") || !strcmp(argv[i],"--rom-size")){
			sOptions->romSize = str2num(argv[i+1]);
			if ( sOptions->romSize == -1){
				ulog(FATAL,"Unsupported Rom Size value. Must be a number.");
				return 1;
			}
			ulog(INFO,"Setting Rom Size to %i",str2num(argv[i+1]));
		}

		 // -m --map-pin
		 // FIXME: Checking second arg value
		if (!strcmp(argv[i],"-m") || !strcmp(argv[i],"--map-pin")){
			if (!strcmp(argv[i+1],"j") || !strcmp(argv[i+1],"J")){
				for(int j=0; strcmp(pinActionsStrings[j],"NULL");j++){
					if(!strcmp(argv[i+2],pinActionsStrings[j])){
						 sOptions->jmpPinHandler = j;
					}
				}
			} else if (!strcmp(argv[i+1],"r") || !strcmp(argv[i+1],"R")){
				for(int j=0; strcmp(pinActionsStrings[j],"NULL");j++){
					if(!strcmp(argv[i+2],pinActionsStrings[j])){
						 sOptions->rtnPinHandler = j;
					}
				}
			} else if (!strcmp(argv[i+1],"o") || !strcmp(argv[i+1],"O")){
				for(int j=0; strcmp(pinActionsStrings[j],"NULL");j++){
					if(!strcmp(argv[i+2],pinActionsStrings[j])){
						 sOptions->flagOPinHandler = j;
					}
				}
			} else if (!strcmp(argv[i+1],"f") || !strcmp(argv[i+1],"F")){
				for(int j=0; strcmp(pinActionsStrings[j],"NULL");j++){
					if(!strcmp(argv[i+2],pinActionsStrings[j])){
						 sOptions->flagFPinHandler = j;
					}
				}
			} else{
				ulog(FATAL,"Unsupported Pin Mapping Width value. J,R,O, or F and " \
								"NONE, JUMP, JSR, RET, JSRS, RETS, HLT, RES, or NULL");
				return 1;
			}
		}

		// FIXME: Handle setting io count lower than referenced addresses.
		// -io --io-count
		if (!strcmp(argv[i],"-io") || !strcmp(argv[i],"--io-count")){
			sOptions->ioDeviceCount = str2num(argv[i+1]);
			if ( sOptions->ioDeviceCount == -1 ){
				ulog(FATAL,"Unsupported Device Count value. Must be a number");
				return 1;
			} else {
				ulog(INFO,"Setting Device Count to %i",sOptions->ioDeviceCount);
			}
		}

		// -b --bind-rr-address
		if (!strcmp(argv[i],"-b") || !strcmp(argv[i],"--bind-rr-address")){
			sOptions->bindResultsRegister = 1;
			sOptions->rrDeviceAddress = str2num(argv[i+1]);
			if ( sOptions->rrDeviceAddress == -1 ){
				ulog(FATAL,"Unsupported Device Address value. Must be a number");
				return 1;
			} else {
				ulog(INFO,"Binding Results Register to Address: %i",sOptions->rrDeviceAddress);
			}
		}

		// -pc --program-counter
		if (!strcmp(argv[i],"-pc") || !strcmp(argv[i],"--program-counter")){
			sOptions->pcInitAddress = str2num(argv[i+1]);
			if ( sOptions->pcInitAddress == 65535){
				ulog(FATAL,"Unsupported Program Counter value. Must be a number.");
				return 1;
			}
			ulog(INFO,"Setting Program Counter to %i",sOptions->pcInitAddress);
		}

		// -p --print-state
		if (!strcmp(argv[i],"-p") || !strcmp(argv[i],"--print-state")){
			sOptions->printState = 1;
			ulog(INFO,"Printing System State");
		}

		// -d --debug
		if (!strcmp(argv[i],"-d") || !strcmp(argv[i],"--debug")){
			sOptions->enableDebugger = 1;
			ulog(INFO,"Enabling Debugger");
		}

		// -s --step-mode
		if (!strcmp(argv[i],"-s") || !strcmp(argv[i],"--step-mode")){
			sOptions->stepMode = 1;
			ulog(INFO,"Starting Program in Step Mode");
		}


		// TODO: -i --interlace
		// TODO: -c --combined-io / --io-overlap
		
	}
	return setWordWidth(sOptions);
}

/* Prints version number */
void printVersion(){
	fprintf(stdout,"bemu v%s\n",VERSION);
}

/* Prints help message */
// Add breakpoint for debug
void printUsage(){
	printVersion();
    printf("Usage: bemu [options] file\n");
    printf("Options:\n");
    printf("-ap N,         --address-position N         Sets the position of the address within the memory from the MSB. Default: 4\n");
	printf("-aw N,         --address-width N            Set the width in bits of the address. Default: 4\n");
	printf(" -b N,         --bind-rr-address N          Binds the Result Register pin to the specified address.\n");
	printf(" -d,           --debug                      Enable the debugger.\n");
    printf(" -e L|B,       --endianess L|B              Set endianess for output file. Default: Little Endian\n");
	printf(" -m J|R|O|F S, --map-pin J|R|O|F S,         Maps a User Pin to the specified function.\n");
	printf("                                            S = NONE, JUMP, JSR, RET, JSRS, RETS, HLT, RES, or NULL\n");
    printf(" -h,           --help                       Print this message and exit.\n");
	printf("-io N,         --io-count                   Sets the number of io address available to the system. Default 15.\n");
	printf("-ip N,         --instruction-position N     Sets the position of the instruction within the memory from the MSB. Default: 0\n");
	printf("-iw N,         --instruction-width N        Set the width in bits of the instruction. Default: 4\n");
	printf(" -p,           --print-state                Prints system status after each instruction. Does not work if --debug is set.\n");
	printf("-pc N,         --program-counter N          Set the Program Counter to an initial address other than 0. Default: 0\n");
	printf(" -r N,         --rom-size N                 Set the size of the Program ROM in bytes. Default: 255\n");
    printf(" -s,           --step-mode                  Runs program in step mode. Valid when --debug flag is set.\n");
    printf(" -v N, 	       --v[vvvv]                    Set the log verbosity to N, 0=OFF, 1=FATAL, 2=ERROR, 3=WARNING, 4=INFO, 5=DEBUG.\n");
    printf("\n");
}
