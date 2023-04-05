#ifndef MCUTILS_H
    #define MCUTILS_H 1

#include <inttypes.h>

#include "MC14500.h"


#ifndef LITTLE_ENDIAN
    #define LITTLE_ENDIAN 0
#endif

#ifndef BIG_ENDIAN
    #define BIG_ENDIAN 1
#endif


/**
 * @struct OPTIONS
 * @brief This structure contains all the configuration parameters for a given
 *        system.
 */
struct OPTIONS {
	char *filename;
    int romSize;
    uint8_t stackSize;
    uint8_t stackDir;
    uint8_t stackWidth;
	int endianess;
	int instructionWidth;
	int addressWidth;
	int instructionPosition;
	int addressPosition;
	int splitFile;
	int wordWidth;
    uint16_t ioDeviceCount;
    uint16_t rrDeviceAddress;
    uint8_t bindResultsRegister;
    struct PIN_HANDLES pinHandles;
    uint16_t pcInitAddress;
    int printState;

    uint8_t enableDebugger;
    int stepMode;
};


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

/**
 * @brief 
 * @param programROM A pointer to a uint32_t array to store the progam file 
 *        contents.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 * @return uint8_t Return 0 for success or 1 if any part fails.
 */
uint8_t programROMFromFile(uint32_t* programROM,struct OPTIONS* sOptions);


long expo(int base, int power);
#endif