#include "MC14500.h"

#include "../../ulog/include/ulog.h"

void (*instructionList[16])(struct MC14500*);

const char* mnenomicStrings[] = { "NOPO","LD","LDC","AND","ANDC","OR","ORC","XNOR","STO","STOC","IEN","OEN","JMP","RTN","SKZ","NOPF"};

/*****************************************************************************/
/*********************************** ICU *************************************/
/*****************************************************************************/

void clearUserPins(struct MC14500 *icu){
    icu->flagFPin = 0;
    icu->flagOPin = 0;
    icu->jmpPin = 0;
    icu->rtnPin = 0;
}

void initICU(struct MC14500 *icu){
	resetICU(icu);
	icu->status = STOPPED;
    icu->resultsRegisterPin = &icu->resultsRegister;
}

void startICU(struct MC14500* icu){
	ulog(DEBUG,"Starting ICU");
	icu->status = RUNNING;
}

void stopICU(struct MC14500* icu){
	ulog(DEBUG,"Stopping ICU");
	icu->status = STOPPED;
}

void resetICU(struct MC14500 *icu){
    icu->flagFPin = 0;
    icu->flagOPin = 0;
    icu->jmpPin = 0;
    icu->rtnPin = 0;
    icu->writePin = 0;
    icu->dataPin = 0;

    icu->resultsRegister = 0;
    icu->ienRegister = 0;
    icu->oenRegister = 0;
    icu->skipRegister = 0;
    icu->logicUnit = 0;
}

void fetch(struct MC14500* icu, enum instructions instruction){
	icu->instruction = instruction;   	 	// Latch instruction to ICU
	icu->resultsRegister = icu->logicUnit;  // Latch Results Register
	icu->writePin = 0;                      // Half Cycle Clear write pin					
}

void execute(struct MC14500 *icu){
    clearUserPins(icu);
	icu->instruction *= !icu->skipRegister;
    (*instructionList[icu->instruction])(icu);
}
