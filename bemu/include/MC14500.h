#ifndef MC14500_H
    #define MC14500_H 1

#include <inttypes.h>

/**
 * @brief Potential Status of the ICU:CREATED,STOPPED,SINGLE,RUNNING,DESTROYED.
*/
enum chipStatuses {CREATED,STOPPED,SINGLE,RUNNING,DESTROYED};

/** 
 * @struct MC14500
 * @brief This structure contains all the availble registers and pins of the ICU. 
 */
struct MC14500 {
    // ICU Status
    uint8_t  status;
    
    // Internal Registers
    uint8_t  resultsRegister;
    uint8_t  ienRegister:1;
    uint8_t  oenRegister:1;
    
    // Instruction Register
    uint8_t  instruction:4;

    // Hidden Regsiters
    uint8_t  skipRegister:1;
    uint8_t  logicUnit:1;

    // ICU Operation Pins
    uint8_t  resetPin:1;
    uint8_t  writePin:1;
    uint8_t  dataPin;

    // ICU User Pins
    uint8_t  flagOPin;
    uint8_t  flagFPin;
    uint8_t  jmpPin;
    uint8_t  rtnPin;
    uint8_t* resultsRegisterPin;  
};

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

    uint8_t enableDebugger;
};

/**
 * @brief Initializes the referenced MC14500 struct. A resetICU is performed and
 * the ICU is set to the STOPPED status. The ResultsRegisterPin is tied to the
 * ResultsRegister so their values are always equal. Pin Handler Pointers are
 * tied to their respective jump pins if a configured Pin Handler is set to 
 * modify the Program Counter.
 * @param icu A pointer to a MC14500 struct to initialize
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 */
void initICU(struct MC14500 *icu, struct OPTIONS* sOptions);

/**
 * @brief Sets the referenced MC14500 struct to the RUNNING status.
 * @param icu A pointer to a MC14500 struct to start.
 */
void startICU(struct MC14500 *icu);

/**
 * @brief Sets the referenced MC14500 struct to the STOPPED status.
 * @param icu A pointer to a MC14500 struct to stop.
 */
void stopICU(struct MC14500 *icu);

/**
 * @brief Resets the referenced MC14500 struct to its initial values. All 
 *        registers and pins are set to 0.
 * @param icu A pointer to a MC14500 struct to reset.
 */
void resetICU(struct MC14500 *icu);

/**
 * @brief "Latches" the Logic Unit data into the ResultsRegister, Clears the 
 *        Write Pin, and retrieves, decodes, and "latches" the instruction
 *        to be performed to the ICU.
 * @param icu A Pointer to the ICU to latch the instruction.
 * @param programROMValue The value read from ROM that contains the combination
 *        Instruction and Address Data to be decoded.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 */
void fetch(struct MC14500* icu, uint32_t programROMValue, struct OPTIONS* sOptions);

/**
 * @brief Performs the instruction that is "latched" to the referenced MC14500 
 *        struct's instruction register and clears all the user pins.
 * @param icu Pointer to the ICU to execute it's instruction.
 */
void execute(struct MC14500* icu);

/**
 * @brief Decodes the Instruction based on configuration provided by the
 *        OPTIONS struct from the programROMValue.
 * @param programROMValue The value read from ROM that contains the combination
 *        Instruction and Address Data to be decoded.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 * @returns uint32_t The value of the decoded Instruction.
 */
uint32_t decodeInstruction(uint32_t programROMValue, struct OPTIONS* sOptions);

/**
 * @brief Decodes the Address based on configuration provided by the
 *        OPTIONS struct from the programROMValue.
 * @param programROMValue The value read from ROM that contains the combination
 *        Instruction and Address Data to be decoded.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 * @returns uint32_t The value of the decoded Address.
 */
uint32_t decodeAddress(uint32_t programROMValue, struct OPTIONS* sOptions);

/**
 * @brief Assigns the Value of the device at the specified address to the 
 *        Data Pin of the icu.
 * @param dataPin A Pointer to the data Pin if the ICU to revcieve the data.
 * @param deviceListValue A Pointer to the Device Value at the address we want
 *        to latch.
 */
void latchIODeviceValueToDataPin(uint8_t* dataPin, uint8_t* deviceListValue);

/**
 * @brief Assigns the Value of the Data Pin of the icu to the device at the 
 *        specified address only if the Write Pin is High.
 * @param deviceListValue A Pointer to the Device Value at the address we want
 *        to latch.
 * @param dataPin A Pointer to the Data Pin if the ICU to recieve the data.
 * @param writePin The value of the Write Pin if the ICU.
 * 
 */
void latchDataPinToIODevice(uint8_t* deviceListValue, uint8_t* dataPin, uint8_t writePin);

/**
 * @brief Determines if a User Pin is high and then executes the defined 
 *        function if there is one.
 * @param icu A pointer to the ICU for configuration.
 * @param stack A Pointer to the Stack,
 * @param sp Current Stack Pointer value.
 * @param pc Current Program Counter value.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 * @param address The address operand of currently executing instruction.
 * @return uint8_t Returns a 0 is successful or a 1 if any failure occurs.
 */
uint8_t pinHandler(struct MC14500* icu,uint32_t* stack, uint8_t* sp, uint16_t* pc, \
                        struct OPTIONS* sOptions, uint32_t address);

/**
 * @brief Determines the amount to increment to Program Counter by based on the
 *        actions configured to be executed by each User Pin. The Program 
 *        Counter should not be incrmented if a User Pin executes a function
 *        that modifies the Program Counter.
 * @param pinHandles A pointer to PIN_HANDLES struct that contains all the User
 *        Pin pointers.
 * @return uint8_t The value to increment the Program Counter by.
 */
uint8_t getPCIncrement(struct PIN_HANDLES* pinHandles);

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

#endif