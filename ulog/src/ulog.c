/******************************************************************************
					uLog - C micro logging framwork
*******************************************************************************
*******************************************************************************
MIT No Attribution License

Copyright (c) 2023

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal 
in the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so. 

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************
******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <ncurses.h>

#include "ulog.h"

/* Strings that correlate to LOGLEVEL enum so that LOGLEVEL can be printed */
const char *LOGLEVELSTRINGS[] = {"OFF","FATAL", "ERROR", "WARNING", "INFO", "DEBUG",};

const char *ONOFFTRINGS[] = {"OFF","ON",};

/* Static global Logging Level to track verbosity across the program */
static int loggingLevel = WARNING;

/* Static global Curses Mode to track mode across the program */
static int cursesMode = 0;

static int cursesLogXPos = 0;
static int cursesLogYPos = 0;

/* Logging method to support filtering out logs by verbosity */
void ulog(int verbosity, const char* logMessage,...) {
	if (verbosity <= loggingLevel){
		char logBuf[1024];
		va_list args;
		va_start(args, logMessage);
		vsnprintf(logBuf,sizeof(logBuf),logMessage, args);
		va_end(args);
		if(cursesMode){
			if(verbosity == OFF){
				// mvprintw(cursesLogYPos,cursesLogXPos,"%s\n", logBuf);
			} else {
				// mvprintw(cursesLogYPos,cursesLogXPos,"%s: %s\n", LOGLEVELSTRINGS[verbosity],logBuf);
			}
		} else {
    		printf("%s:\t%s\n", LOGLEVELSTRINGS[verbosity],logBuf);
		}
	}
}

/* Sets the LoggingLevel to the specified newLogLevel. Fails and returns 1 if 
   an invalid newLogLevel is passed. Otherwise, returns 0. */
int setLoggingLevel(int newLogLevel){
	if( newLogLevel < DEBUG || newLogLevel > OFF){
		loggingLevel = newLogLevel;
		ulog(INFO,"Setting Logging Level to [%i] %s",newLogLevel,LOGLEVELSTRINGS[newLogLevel]);
		return 0;
	} else {
		ulog(ERROR,"Invalid Logging Level. Log Level could not be set.");
		return 1;
	}
}

/* Sets CursesMode to the specified mode. */
int setCursesMode(int mode){
		if(mode > 1){
			mode = 1;
		}
		cursesMode = mode;
		ulog(INFO,"Setting Curses Mode to %s",ONOFFTRINGS[mode]);
		return 0;	
}

/* Sets CursesMode to the specified mode. */
int setCursesLogPos(int xPos,int yPos){
		cursesLogXPos = xPos;
		cursesLogYPos = yPos;
		return 0;	
}

/* Returns the currently set LoggingLevel */
int getLoggingLevel(){
	return loggingLevel;
}