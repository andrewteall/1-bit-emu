#include "MC14500.h"
#include "ulog.h"

const char* mnenomicStrings[] = {"NOPO","LD","LDC","AND","ANDC","OR","ORC","XNOR","STO","STOC","IEN","OEN","JMP","RTN","SKZ","NOPF"};

// void nopo(struct MC14500* icu);
// void   ld(struct MC14500* icu);
// void  ldc(struct MC14500* icu);
// void  and(struct MC14500* icu);
// void andc(struct MC14500* icu);
// void   or(struct MC14500* icu);
// void  orc(struct MC14500* icu);
// void xnor(struct MC14500* icu);
// void  sto(struct MC14500* icu);
// void stoc(struct MC14500* icu);
// void  ien(struct MC14500* icu);
// void  oen(struct MC14500* icu);
// void  jmp(struct MC14500* icu);
// void  rtn(struct MC14500* icu);
// void  skz(struct MC14500* icu);
// void nopf(struct MC14500* icu);

void nopo(struct MC14500 *icu){
    icu->logicUnit = icu->resultsRegister;
    icu->flagOPin = !icu->skipRegister;
    icu->skipRegister = 0;
}

void ld(struct MC14500 *icu){
    icu->logicUnit = icu->dataPin & icu->ienRegister;
}

void ldc(struct MC14500 *icu){
    icu->logicUnit = !(icu->dataPin & icu->ienRegister);
}

void and(struct MC14500 *icu){
    icu->logicUnit = icu->resultsRegister & (icu->dataPin & icu->ienRegister);
}

void andc(struct MC14500 *icu){
    icu->logicUnit = icu->resultsRegister & (!(icu->dataPin & icu->ienRegister));
}

void or(struct MC14500 *icu){
    icu->logicUnit = icu->resultsRegister | (icu->dataPin & icu->ienRegister);
}

void orc(struct MC14500 *icu){
    icu->logicUnit = icu->resultsRegister | (!(icu->dataPin & icu->ienRegister));
}

void xnor(struct MC14500 *icu){
    icu->logicUnit = !(icu->resultsRegister^icu->dataPin);
}

void sto(struct MC14500 *icu){
    icu->dataPin = icu->resultsRegister & icu->oenRegister;
    icu->writePin = 1;
}

void stoc(struct MC14500 *icu){
    icu->dataPin = (!icu->resultsRegister) & icu->oenRegister;
    icu->writePin = 1;
}

void ien(struct MC14500 *icu){
    icu->ienRegister = icu->dataPin;
}

void oen(struct MC14500 *icu){
    icu->oenRegister = icu->dataPin;
}

void jmp(struct MC14500 *icu){
    icu->jmpPin = 1;
}

void rtn(struct MC14500 *icu){
    icu->skipRegister = 1;
    icu->rtnPin = 1;
}

void skz(struct MC14500 *icu){
    icu->skipRegister = !icu->resultsRegister;
}

void nopf(struct MC14500 *icu){
	icu->logicUnit = icu->resultsRegister;
	icu->flagFPin = 1;
}

void (*instructionList[])(struct MC14500* icu) = {nopo,ld,ldc,and,andc,or,orc,xnor,sto,stoc,ien,oen,jmp,rtn,skz,nopf};

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
