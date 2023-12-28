#include <ncurses.h>
#include <stdint.h>

#include "bemu.h"
#include "debugger.h"
#include "IODevice.h"
#include "MC14500.h"
#include "MCSystem.h"
#include "ulog.h"
#include "utils.h"

const char* pinActionsStrings4[] = {"NONE","JUMP","JSR","RET","JSRS","RETS","HLT","RES","NULL"};

int startDebugger(struct OPTIONS* sOptions){
    int err = 0;
    setCursesMode(1);
    setCursesLogPos(0,22);
    initscr();
    cbreak();
    noecho();
    nonl(); 
    // intrflush(stdscr, FALSE); 
    keypad(stdscr, TRUE);
    if (has_colors() == FALSE) {
        endwin();
        ulog(ERROR,"Your terminal does not support color\n");
        err = 1;    
    } else {        
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, COLOR_RED);
        init_pair(2, COLOR_WHITE, COLOR_BLUE);
    }
    if(sOptions->stepMode){
        timeout(-1);
    } else {
        timeout(10);
    }
    
    // if ((LINES < 24) || (COLS < 80)) {
    //     // err = 1;
    //     endwin();
    //     setLoggingLevel(WARNING);
    //     ulog(ERROR,"Your terminal needs to be at least 80x24");
    // }
    return err;
}

void stopDebugger(int error){
    if(error){
        getch(); // Allow last log to be written to screen
    }
    setCursesMode(0);
    endwin();
}

void checkForUserKeyPress(struct OPTIONS* sOptions,uint16_t pc, uint32_t address, struct MC14500* icu){
    int ch = getch();
    switch (ch){
        case KEY_UP:
            // printw ("\nUp Arrow");
            break;
        case KEY_DOWN:
            // printw ("\nDown Arrow");
            break;
        case KEY_RIGHT:
            // printw ("\nLeft Arrow");
            break;
        case KEY_LEFT:
            // printw ("\nRight Arrow");
            break;
        case 's':
            sOptions->stepMode = !sOptions->stepMode;
            if(sOptions->stepMode){
                timeout(-1);
            } else {
                timeout(10);
            }
            break;
        case 'q':
            icu->status = STOPPED;
            break;
        case ERR:
            break;
        default:
            break;
    }
}

void printStack(struct OPTIONS* sOptions,uint8_t xpos,uint8_t ypos,uint32_t* stack, uint32_t* sp){
    // -> 0xNN 0xNNNN
    // 11 x 7
    int viewContext = *sp;
    mvprintw(ypos++,xpos,"       Stack      ");
    mvprintw(ypos++,xpos," ================ ");
    if (viewContext == sOptions->stackSize || viewContext == sOptions->stackSize-1){
        viewContext = 0xfd;
    }
    if (viewContext == 0 || viewContext == 1){
        viewContext = 0x02;
    }
    if(sOptions->stackDir == UP){
        for(int i = viewContext-2; i < viewContext+3; i--){
            if(i == *sp){
                mvprintw(ypos++,xpos,"  -> 0x%02x 0x%04x ",i,stack[i]);
            } else {
                mvprintw(ypos++,xpos,"     0x%02x 0x%04x ",i,stack[i]);
            }
        }
    }
    if(sOptions->stackDir == DOWN){
        for(int i = viewContext+2; i > viewContext-3; i--){
            if(i == *sp){
                mvprintw(ypos++,xpos,"  -> 0x%02x 0x%04x ",i,stack[i]);
            } else {
                mvprintw(ypos++,xpos,"     0x%02x 0x%04x ",i,stack[i]);
            }
        }
    }
    
}

void clearLogMessage(){
    ulog(OFF,"                                                                                ");
}

void drawScreen(struct OPTIONS* sOptions,uint16_t pc, uint32_t address, struct MC14500* icu,struct IODevice* deviceList, \
                    uint32_t* stack, uint32_t* sp,  uint32_t* programROM, struct PIN_HANDLES* pinHandles){
    move(0,0);
    printw("-------------------------------| Bemu Debugger  |-------------------------------\n");
    move(1,0);
    printw("       00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F\n");
	printw("Device ===============================================================\n");
	for (int i = 0; i < sOptions->ioDeviceCount && i < 48; i++){
		printw("%04x | ",i);
		int j=0;
		while(j<16 && i<sOptions->ioDeviceCount){
            if(address == i){
                attrset(COLOR_PAIR(1));
                printw("%02x",*deviceList[i++].value);
                attroff(COLOR_PAIR(1));
                printw("  ");
            } else{
                if(sOptions->bindResultsRegister && (i == sOptions->rrDeviceAddress)){
                    attrset(COLOR_PAIR(2));
                    printw("%02x",*deviceList[i++].value);
                    attroff(COLOR_PAIR(2));
                    printw("  ");
                } else {
			        printw("%02x  ",*deviceList[i++].value);
                }
            }
			j++;
		}
		i--;
		printw("\n");
	}
    
    move(7, 0);
    printw("                 Registers and Pins          \n");
    printw("=====================================================\n");
    printw("PC: 0x%02x  Inst: %4s(0x%02x)  Addr: 0x%02x    SP: 0x%02x\n\nLU: %i     RR: %i          IEN/OEN = %i/%i " \
					"   Write: %i\n\nData: %i   Skip Next: %i      JROF: %i%i%i%i    Map: \n",
                    pc,mnenomicStrings[icu->instruction],icu->instruction,address,*sp, icu->logicUnit,icu->resultsRegister, \
                    icu->ienRegister,icu->oenRegister,icu->writePin,icu->dataPin,icu->skipRegister,icu->jmpPin,icu->rtnPin, \
                    icu->flagOPin,icu->flagFPin/*, pinActionsStrings4[getActivePinAndHandler(icu,pinHandles)]*/);


    printStack(sOptions,53,7,stack,sp);

    move(15, 0);
    printw("       00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F\n");
	printw(" ROM   ===============================================================\n");
	for (int i = 0; i < sOptions->romSize && i < 64; i++){
		printw("%04x | ",i);
		int j=0;
		while(j<16 && i<sOptions->romSize){
            if(pc <= i && pc >= (i-(sOptions->wordWidth-1))){
                attrset(COLOR_PAIR(1));
                printw("%02x",programROM[i++]);
                attroff(COLOR_PAIR(1));
                printw("  ");
            } else{
			    printw("%02x  ",programROM[i++]);
            }
			j++;
		}
		i--;
		printw("\n");
	}
    


    int execX = 70;
    int execY = 2;
    int idx = 0;

    for(int i = 0; i < 19;i++){
        uint32_t programROMValue = readWordFromROM(programROM,idx,sOptions->wordWidth, sOptions->endianess);
        uint32_t romAddress = decodeAddress(programROMValue, sOptions->wordWidth, sOptions->instructionWidth, sOptions->instructionPosition);
        uint32_t  instruction = decodeInstruction(programROMValue, sOptions->wordWidth, sOptions->instructionWidth, sOptions->instructionPosition);
         mvprintw(execY,execX," ");
        if(pc == idx){
            attrset(COLOR_PAIR(1));
            printw("%4s %4x",mnenomicStrings[instruction],romAddress);
            attroff(COLOR_PAIR(1));
        }else{
            printw("%4s %4x",mnenomicStrings[instruction],romAddress);
        }
        idx += sOptions->wordWidth;
        execY++;
    }

    move(23,0);
    checkForUserKeyPress(sOptions, pc, address, icu);
    clearLogMessage();
    
    refresh();
}
