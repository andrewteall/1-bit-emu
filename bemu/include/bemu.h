#ifndef BEMU_H
    #define BEMU_H 1
#include <inttypes.h>

#include "MC14500.h"

#define MAJOR "0"
#define MINOR "9"
#define PATCH "0"
#define VERSION  MAJOR "." MINOR "." PATCH

enum pinActions {NONE,JUMP,JSR,RET,JSRS,RETS,HLT,RES,};

/**
 * @struct PIN_HANDLES
 * @brief This structure contains all User Pin Handler selectors and the User 
 *        Pin pointers.
 */
struct PIN_HANDLES {
	uint8_t pinSink;
    uint8_t* jmpPinPtr;
    uint8_t* rtnPinPtr;
    uint8_t* flagFPinPtr;
    uint8_t* flagOPinPtr;

    uint8_t jmpPinHandler;
    uint8_t rtnPinHandler;
    uint8_t flagFPinHandler;
    uint8_t flagOPinHandler;
};

/**
 * @struct OPTIONS
 * @brief This structure contains all the configuration parameters for a given
 *        system.
 */
struct OPTIONS {
	char *filename;
    uint32_t romSize;
    uint8_t stackSize;
    uint8_t stackDir;
    uint8_t stackWidth;
	uint8_t endianess;
	uint8_t instructionWidth;
	uint8_t addressWidth;
	uint8_t instructionPosition;
	uint8_t addressPosition;
	int splitFile;
	uint8_t wordWidth;
    uint16_t ioDeviceCount;
    uint16_t rrDeviceAddress;
    uint8_t bindResultsRegister;
    struct PIN_HANDLES pinHandles;
    uint16_t pcInitAddress;
    int printState;

    uint8_t enableDebugger;
    int stepMode;
};


int run(struct OPTIONS* sOptions);

/**
 * @brief Prints the command line usage menu.
 */
void printUsage(void);

/**
 * @brief Prints System Program Counter and current Address as well as Register
 *        and Pin information of the ICU. 
 * @param pc Current Program Counter value.
 * @param address Current operating address.
 * @param icu A pointer to a MC14500 struct to obtain information.
 */
void printSystemInfo(uint16_t pc, uint32_t address, struct MC14500* icu);

/**
 * @brief Parses all the command line Arguments and sets the appropriate options
 *        in the OPTIONS struct.
 * @param sOptions A pointer to the OPTIONS struct to store all the configured
 *        options.
 * @param argc Count of command line arguments.
 * @param argv A pointer to the Array of command line arguments.
 * @return 
 */
int parseCommandLineOptions(struct OPTIONS* sOptions,int argc, char* argv[]);

#endif