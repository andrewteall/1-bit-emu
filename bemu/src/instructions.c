#include "MC14500.h"

/*****************************************************************************/

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