#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "MCSystem.h"
#include "ulog.h"

const char* pinActionsStrings[] = {"NONE","JUMP","JSR","RET","JSRS","RETS","HLT","RES","NULL"};

/*****************************************************************************/
/********************************** Stack ************************************/
/*****************************************************************************/
uint8_t push(struct STACK* stack, uint32_t value){
	uint8_t error = 0;
    stack->data[stack->sp] = value;
    if(stack->stackDir == UP){
		if(stack->sp == stack->stackSize){
			error = 1;
		}
        stack->sp++;
    } else {
		if(stack->sp == 0){
			error = 1;
		}
        stack->sp--;
    }
	return error;
}

uint32_t pull(struct STACK* stack){
    if(stack->stackDir == UP){
		if(stack->sp == 0){
			return -1;
		}
        stack->sp--;
    } else {
		if(stack->sp == stack->stackSize){
			return -1;
		}
        stack->sp++;
    }
    return stack->data[stack->sp];
}

void initStack(struct STACK* stack, uint32_t* data, uint32_t stackSize, uint8_t stackDir, uint8_t stackWidth){
	stack->data = data;
	stack->sp = !stackDir * (stackSize);
	stack->stackSize = stackSize;
	stack->stackDir = stackDir;
	stack->stackWidth = stackWidth;
}

/*****************************************************************************/
/************************* User Defined Functions ****************************/
/*****************************************************************************/
void jump(uint16_t* pc, uint32_t address){
	*pc = address;
}

uint8_t jumpSubRoutine(struct STACK* stack, uint16_t* pc, uint32_t address){
	// push programConter;
	uint8_t err = 0;
	err = push(stack, *pc);
	if(err){
		ulog(ERROR,"Stack Overflow at address: 0x%02x",*pc);
	}
	// set programCounter to address
	*pc = address;
	return err;
}

uint8_t returnSubRoutine(struct STACK* stack, uint16_t* pc){
	uint8_t err = 0;
	*pc = pull(stack);
	if((signed)*pc == -1){
		ulog(ERROR,"Stack Underflow at address: 0x%02x",pc);
	}
	return err;
}

uint8_t jumpSubRoutineShallow(struct STACK* stack, uint16_t* pc, uint32_t address){
	// Save programConter;
	stack->data[0] = *pc;
	// set programCounter to address
	*pc = address;
	return 0;
}

uint8_t returnSubRoutineShallow(struct STACK* stack, uint16_t* pc){	
	// set programCounter to stackRegister
	stack->data[0] = *pc;
	return 0;
}

uint8_t getActivePinAndHandler(uint8_t jmpPin, uint8_t rtnPin, uint8_t flagOPin, uint8_t flagFPin, struct PIN_HANDLES* pinHandles){
	return (pinHandles->flagFPinHandler*flagFPin) + (rtnPin*pinHandles->rtnPinHandler) + \
        (jmpPin*pinHandles->jmpPinHandler) + (flagOPin*pinHandles->flagOPinHandler);
}

uint8_t pinHandler(uint8_t handlerForPin, struct STACK* stack, uint16_t* pc, uint32_t address, struct SIGNALS* signals){
	uint8_t error = 0;
	switch (handlerForPin){
		case NONE:
			break;
		case JUMP:
			jump(pc, address);
			break;
		case JSR:
			error = jumpSubRoutine(stack, pc, address);
			break;
		case RET:
			error = returnSubRoutine(stack, pc);
			break;
		case JSRS:
			error = jumpSubRoutineShallow(stack, pc, address);
			break;
		case RETS:
			error = returnSubRoutineShallow(stack, pc);
			break;
		case HLT:
			signals->stopSignal = 1;
			break;
		case RES:
			signals->resetSignal = 1;
			break;
		default:
			break;
	}
	return error;
}

void clearSignals(struct SIGNALS* signals){
	signals->stopSignal = 0;
	signals->resetSignal = 0;
}

/*****************************************************************************/
/*********************************** ROM *************************************/
/*****************************************************************************/
long getFileSize(FILE *file){
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
	return size;
}

uint8_t programROMFromFile(uint32_t* programROM, char* filename, uint32_t romSize){
	if (filename != NULL){
		FILE* programROMFile = fopen(filename, "r");
		if(programROMFile == NULL){
			ulog(FATAL,"Error Opening File: %s",filename);
			return 1;
		}
		ulog(DEBUG,"Filesize: %li bytes",getFileSize(programROMFile));
		uint8_t i = 0;
		for (i=0;i < romSize && programROM[i-1] != EOF;i++){
			programROM[i] += fgetc(programROMFile);
		}
		fclose(programROMFile);
		// Zero out remaining rom
		for (i = i; i < romSize;i++){
			programROM[i] = 0;
		}
	} else {
		return 1;
	}
	return 0;
}

uint32_t readWordFromROM(uint32_t* programROM, uint16_t pc, uint8_t wordWidth, uint8_t endianess){
	uint32_t wordValue = 0;
	for(int i = 0;i<wordWidth;i++){
		switch (endianess){
		case LITTLE_ENDIAN:
			wordValue += (programROM[i+pc] << (i*8));
			break;
		
		case BIG_ENDIAN:
			wordValue <<= 8;
			wordValue += programROM[i+pc];
			break;
		}
	}
	return wordValue;
}

uint32_t decodeInstruction(uint32_t programROMValue, uint8_t wordWidth, uint8_t instructionWidth, uint8_t instructionPosition){
	uint32_t instructionShift = (wordWidth*8)-instructionWidth-instructionPosition;
	uint32_t instructionMask = (uint32_t)(pow(2,instructionWidth)-1) << instructionShift;
	
	return (programROMValue & instructionMask) >> instructionShift;
}

uint32_t decodeAddress(uint32_t programROMValue, uint8_t wordWidth, uint8_t addressWidth, uint8_t addressPosition){
	uint32_t addressShift = (wordWidth*8)-addressWidth-addressPosition;
	uint32_t addressMask = (uint32_t)(pow(2,addressWidth)-1) << addressShift;
	
	return (programROMValue & addressMask) >> addressShift;
}

/*****************************************************************************/
/****************************** Program Counter ******************************/
/*****************************************************************************/
uint8_t getPCIncrement(uint8_t jmpPin, uint8_t rtnPin, uint8_t flagFPin, uint8_t flagOPin, int wordWidth){
	return (!(jmpPin)*!(rtnPin)*!(flagFPin)*!(flagOPin)) * wordWidth;
}
