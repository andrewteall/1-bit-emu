/******************************************************************************
					uLog - C micro logging framwork
*******************************************************************************
******************************************************************************
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

#ifndef ULOG_H
    #define ULOG_H 1
    /**
     * @brief Defines the Logging Levels used by uLog.
     */
    enum LOGLEVEL {OFF,FATAL,ERROR,WARNING,INFO,DEBUG};

    /**
     * @brief Prints a message to stdout with the loggingLevel prepended.
     * @param verbosity The logging level of the generated log.
     * @param logMessage The formatted string to output to stdout.
     * @param ... The formatted string parameters.
     */
    void ulog(int , const char* logMessage, ...);
    
    /**
     * @brief Sets the logging level used by the program
     * @param newLogLevel The logging level to set the program to.
     * @returns uint8_t Returns 0 if loggingLevel is successfully set. 1 if not.
     */
    int setLoggingLevel(int logLevel);
    
    /**
     * @brief Returns the current logging level that is set.
     * @returns int The current logging level.
     */
    int getLoggingLevel(void);
#endif