#ifndef MC14500_H
    #define MC14500_H 1

#include <inttypes.h>

enum instructions {NOPO,LD,LDC,AND,ANDC,OR,ORC,XNOR,STO,STOC,IEN,OEN,JMP,RTN,SKZ,NOPF,};
const char* mnenomicStrings[16];

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
 * @brief Initializes the referenced MC14500 struct. A resetICU is performed and
 * the ICU is set to the STOPPED status. The ResultsRegisterPin is tied to the
 * ResultsRegister so their values are always equal. Pin Handler Pointers are
 * tied to their respective jump pins if a configured Pin Handler is set to 
 * modify the Program Counter.
 * @param icu A pointer to a MC14500 struct to initialize
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 */
void initICU(struct MC14500 *icu, struct PIN_HANDLES* pinHandles);

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
void fetch(struct MC14500* icu, enum instructions instruction, uint16_t pc);

/**
 * @brief Performs the instruction that is "latched" to the referenced MC14500 
 *        struct's instruction register and clears all the user pins.
 * @param icu Pointer to the ICU to execute it's instruction.
 */
void execute(struct MC14500* icu);


#endif