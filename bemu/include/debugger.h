#ifndef DEBUGGER_H
    #define DEBUGGER_H 1


int  startDebugger(struct OPTIONS* sOptions);
void stopDebugger(void);
void drawScreen(struct OPTIONS* sOptions,uint16_t pc, uint32_t address, struct MC14500* icu,struct IODevice* deviceList, \
                    uint32_t* stack, uint8_t* sp);

#endif