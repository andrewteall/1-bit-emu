#ifndef DEBUGGER_H
    #define DEBUGGER_H 1
#include "IODevice.h"


int  startDebugger(struct OPTIONS* sOptions);
void stopDebugger(int error);
void drawScreen(struct OPTIONS* sOptions,uint16_t pc, uint32_t address, struct MC14500* icu,struct IODevice* deviceList, \
                    uint32_t* stack, uint32_t* sp,  uint32_t* programROM);

#endif