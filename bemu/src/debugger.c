#include <ncurses.h>

#include "MC14500.h"
#include "MCUtils.h"
#include "MCSystem.h"
#include "IODevice.h"
#include "../../ulog/include/ulog.h"

// TODO: Better log redirection for debugger 

int startDebugger(struct OPTIONS* sOptions){
    int err = 0;
    setCursesMode(1);
    setCursesLogPos(0,12);
    initscr();
    cbreak();
    noecho();
    nonl(); 
    // intrflush(stdscr, FALSE); 
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

void printStack(struct OPTIONS* sOptions,uint8_t xpos,uint8_t ypos,uint32_t* stack, uint8_t* sp){
    // -> 0xNN 0xNNNN
    // 11 x 7
    int viewContext = *sp;
    mvprintw(ypos++,xpos,"       Stack      ");
    mvprintw(ypos++,xpos," ---------------- ");
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
    move(7, 0);
    printw("PC = 0x%02x  Inst = %4s(0x%02x)  Addr = 0x%02x  LU = %i  RR = %i\n  IEN/OEN = %i/%i  " \
					"Write: %i  Data: %i  Skip Next: %i  JROF: %i%i%i%i\n",
                    pc,mnenomicStrings[icu->instruction],icu->instruction,address,icu->logicUnit,icu->resultsRegister,icu->ienRegister, \
                    icu->oenRegister,icu->writePin,icu->dataPin,icu->skipRegister,icu->jmpPin,icu->rtnPin,icu->flagOPin,icu->flagFPin);

    printStack(sOptions,61,5,stack,sp);
    refresh();
    
    
    checkForUserKeyPress(sOptions, pc, address, icu);
    ulog(OFF,"                                                                                ");
    refresh();
}
