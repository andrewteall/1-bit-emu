#include <stdio.h>
#include <string.h>

#include "MCSystem.h"
#include "../../ulog/include/ulog.h"

const char* pinActionsStrings[] = {"NONE","JUMP","JSR","RET","JSRS","RETS","HLT","RES","NULL"};

/*****************************************************************************/
/********************************** Stack ************************************/
/*****************************************************************************/

void push(uint32_t* stack, uint8_t* sp, uint32_t value, struct OPTIONS* sOptions, uint8_t* err){
    stack[*sp] = value;
    if(sOptions->stackDir == UP){
		if(*sp == sOptions->stackSize){
			ulog(ERROR,"Stack Overflow");
			*err = 1;
		}
        (*sp)++;
    } else {
		if(*sp == 0){
			ulog(ERROR,"Stack Overflow");
			*err = 1;
		}
        (*sp)--;
    }
}

uint32_t pull(uint32_t* stack, uint8_t* sp, struct OPTIONS* sOptions, uint8_t* err){
    if(sOptions->stackDir == UP){
		if(*sp == 0){
			ulog(ERROR,"Stack Overflow");
			*err = 1;
			return *sp;
		}
        (*sp)--;
    } else {
		if(*sp == sOptions->stackSize){
			ulog(ERROR,"Stack Overflow");
			*err = 1;
			return *sp;
		}
        (*sp)++;
    }
    return stack[*sp];
}

/*****************************************************************************/
/************************* User Defined Functions ****************************/
/*****************************************************************************/

void jump(uint16_t* pc, uint32_t address){
	*pc = address;
}

uint8_t jumpSubRoutine(uint32_t* stack, uint8_t* sp, uint16_t* pc, struct OPTIONS* sOptions, uint32_t address){
	// push programConter;
	uint8_t err = 0;
	push(stack,sp,*pc,sOptions,&err);
	// set programCounter to address
	*pc = address;
	return err;
}

uint8_t returnSubRoutine(uint32_t* stack, uint8_t* sp, uint16_t* pc, struct OPTIONS* sOptions){
	uint8_t err = 0;
	*pc = pull(stack,sp,sOptions,&err);
	return err;
}

uint8_t jumpSubRoutineShallow(uint32_t* stackRegsister, uint16_t* pc, uint32_t address){
	// Save programConter;
	*stackRegsister = *pc;
	// set programCounter to address
	*pc = address;
	return 0;
}

uint8_t returnSubRoutineShallow(uint32_t* stackRegsister, uint16_t* pc){	
	// set programCounter to stackRegister
	*stackRegsister = *pc;
	return 0;
}

void halt(struct MC14500 *icu){
	stopICU(icu);
}

uint8_t selectPinAndHandler(struct MC14500* icu, struct PIN_HANDLES* pinHandles){
	return(pinHandles->flagFPinHandler*icu->flagFPin) + (icu->rtnPin*pinHandles->rtnPinHandler) + \
        (icu->jmpPin*pinHandles->jmpPinHandler) + (icu->flagOPin*pinHandles->flagOPinHandler);
}

uint8_t pinHandler(struct MC14500* icu,uint32_t* stack, uint8_t* sp, uint16_t* pc, struct OPTIONS* sOptions, uint32_t address){
	uint8_t error = 0;
	switch (selectPinAndHandler(icu,&sOptions->pinHandles)){
		case NONE:
			break;
		case JUMP:
			jump(pc, address);
			break;
		case JSR:
			error = jumpSubRoutine(stack, sp, pc, sOptions, address);
			break;
		case RET:
			error = returnSubRoutine(stack, sp, pc, sOptions);
			break;
		case JSRS:
			error = jumpSubRoutineShallow(stack, pc, address);
			break;
		case RETS:
			error = returnSubRoutineShallow(stack, pc);
			break;
		case HLT:
			halt(icu);
			break;
		case RES:
			resetICU(icu);
			break;
		default:
			break;
	}
	return error;
}


/*****************************************************************************/
/*********************************** ROM *************************************/
/*****************************************************************************/

/**
 * @brief Determines the size of a file in bytes.
 * @param file A Pointer to the file to determine size.
 * @return long The size of the file in bytes.
 */
long getFileSize(FILE *file){
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
	return size;
}

uint8_t programROMFromFile(uint32_t* programROM,struct OPTIONS* sOptions){
	if (sOptions->filename != NULL){
		FILE* programROMFile = fopen(sOptions->filename, "r");
		if(programROMFile == NULL){
			ulog(FATAL,"Error Opening File: %s",sOptions->filename);
			return 1;
		}
		ulog(DEBUG,"Filesize: %li bytes",getFileSize(programROMFile));
		uint8_t i = 0;
		for (i=0;i < sOptions->romSize && programROM[i-1] != EOF;i++){
			programROM[i] += fgetc(programROMFile);
		}
		fclose(programROMFile);
		// Zero out remaining rom
		for (i = i; i < sOptions->romSize;i++){
			programROM[i] = 0;
		}
	} else {
		return 1;
	}
	return 0;
}

uint32_t readWordFromROM(uint32_t* programROM, uint16_t pc, struct OPTIONS* sOptions){
	uint32_t wordValue = 0;
	for(int i = 0;i<sOptions->wordWidth;i++){
		switch (sOptions->endianess){
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


uint32_t decodeInstruction(uint32_t programROMValue, struct OPTIONS* sOptions){
	uint32_t instructionShift = (sOptions->wordWidth*8)-sOptions->instructionWidth-sOptions->instructionPosition;
	uint32_t instructionMask = (expo(2,sOptions->instructionWidth)-1) << instructionShift;
	
	return (programROMValue & instructionMask) >> instructionShift;
}

uint32_t decodeAddress(uint32_t programROMValue, struct OPTIONS* sOptions){
	uint32_t addressShift = (sOptions->wordWidth*8)-sOptions->addressWidth-sOptions->addressPosition;
	uint32_t addressMask = (expo(2,sOptions->addressWidth)-1) << addressShift;
	
	return (programROMValue & addressMask) >> addressShift;
}



/*****************************************************************************/
/****************************** Program Counter ******************************/
/*****************************************************************************/

uint8_t getPCIncrement(struct PIN_HANDLES* pinHandles,int wordWidth){
	return (!(*pinHandles->jmpPinPtr)*!(*pinHandles->rtnPinPtr)*!(*pinHandles->flagFPinPtr)*!(*pinHandles->flagOPinPtr)) * wordWidth;
}

