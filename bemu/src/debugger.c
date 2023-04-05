#include <ncurses.h>

#include "MC14500.h"
#include "MCUtils.h"
#include "IODevice.h"
#include "../../ulog/include/ulog.h"

int startDebugger(struct OPTIONS* sOptions){
    int err = 0;
    setLoggingLevel(OFF);
    initscr();
    cbreak();
    noecho();
    nonl(); 
    intrflush(stdscr, FALSE); 
    keypad(stdscr, TRUE);
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

void stopDebugger(void){
    endwin();
}

void checkForUserKeyPress(struct OPTIONS* sOptions,uint16_t pc, uint32_t address, struct MC14500* icu){
    int ch = getch();
    switch (ch){
        // case KEY_UP:
        //     printw ("\nUp Arrow");
        //     break;
        // case KEY_DOWN:
        //     printw ("\nDown Arrow");
        //     break;
        // case KEY_RIGHT:
        //     printw ("\nLeft Arrow");
        //     break;
        // case KEY_LEFT:
        //     printw ("\nRight Arrow");
        //     break;
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

void drawScreen(struct OPTIONS* sOptions,uint16_t pc, uint32_t address, struct MC14500* icu,struct IODevice* deviceList, \
                    uint32_t* stack, uint8_t* sp){
    move(0,0);
    printw("-------------------------------| Bemu Debugger  |-------------------------------\n");
    move(1,0);
    printw("\n       00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F\n");
	printw("       ===============================================================\n");
	for (int i = 0; i < sOptions->ioDeviceCount; i++){
		printw("%04x | ",i);
		int j=0;
		while(j<16 && i<sOptions->ioDeviceCount){
			printw("%02x  ",*deviceList[i++].value);
			j++;
		}
		i--;
		printw("\n");
	}
    move(10, 0);
    printw("PC = 0x%02x  Inst = %4s(0x%02x)  Addr = 0x%02x  LU = %i  RR = %i  IEN/OEN = %i/%i  " \
					"\n\t\tWrite: %i  Data: %i  Skip Next: %i  JROF: %i%i%i%i\n",
                    pc,mnenomicStrings[icu->instruction],icu->instruction,address,icu->logicUnit,icu->resultsRegister,icu->ienRegister, \
                    icu->oenRegister,icu->writePin,icu->dataPin,icu->skipRegister,icu->jmpPin,icu->rtnPin,icu->flagOPin,icu->flagFPin);
    refresh();
    
    checkForUserKeyPress(sOptions, pc, address, icu);
}
