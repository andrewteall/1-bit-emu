#ifndef MCSYSTEM_H
    #define MCSYSTEM_H 

#ifndef UP
    #define UP 1
#endif

#ifndef DOWN
    #define DOWN 0
#endif

#ifndef LITTLE_ENDIAN
    #define LITTLE_ENDIAN 0
#endif

#ifndef BIG_ENDIAN
    #define BIG_ENDIAN 1
#endif

enum pinActions {NONE,JUMP,JSR,RET,JSRS,RETS,HLT,RES,};
extern const char* pinActionsStrings[];

/**
 * @struct PIN_HANDLES
 * @brief This structure contains all User Pin Handler selectors and the User 
 *        Pin pointers.
 */
struct PIN_HANDLES {
	// uint8_t pinSink;
    // uint8_t* jmpPinPtr;
    // uint8_t* rtnPinPtr;
    // uint8_t* flagFPinPtr;
    // uint8_t* flagOPinPtr;

    uint8_t jmpPinHandler;
    uint8_t rtnPinHandler;
    uint8_t flagFPinHandler;
    uint8_t flagOPinHandler;
};

struct SIGNALS {
    uint8_t stopSignal;
    uint8_t resetSignal;
};

struct STACK {
    uint32_t* data;
    uint32_t sp;
    uint32_t stackSize;
    uint8_t stackDir;
    uint8_t stackWidth;
};

void initStack(struct STACK* stack, uint32_t* data, uint32_t stackSize, uint8_t stackDir, uint8_t stackWidth);

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
// uint8_t pinHandler(struct MC14500* icu,struct STACK* stack, uint16_t* pc, uint32_t address, struct PIN_HANDLES* pinHandles);
// uint8_t getActivePinAndHandler(struct MC14500* icu, struct PIN_HANDLES* pinHandles);
// void setPinHandlers(struct PIN_HANDLES* pinHandles,uint8_t* jmpPin,uint8_t* rtnPin,uint8_t* flagOPin,uint8_t* flagFPin);
uint8_t getActivePinAndHandler(uint8_t jmpPin, uint8_t rtnPin, uint8_t flagOPin, uint8_t flagFPin, struct PIN_HANDLES* pinHandles);
uint8_t pinHandler(uint8_t handlerForPin, struct STACK* stack, uint16_t* pc, uint32_t address, struct SIGNALS* signals);
void clearSignals(struct SIGNALS* signals);


void jump(uint16_t* pc, uint32_t address);
uint8_t jumpSubRoutine(struct STACK* stack, uint16_t* pc, uint32_t address);
uint8_t returnSubRoutine(struct STACK* stack, uint16_t* pc);
uint8_t jumpSubRoutineShallow(struct STACK* stack, uint16_t* pc, uint32_t address);
uint8_t returnSubRoutineShallow(struct STACK* stack, uint16_t* pc);


/**
 * @brief Determines the amount to increment to Program Counter by based on the
 *        actions configured to be executed by each User Pin. The Program 
 *        Counter should not be incrmented if a User Pin executes a function
 *        that modifies the Program Counter.
 * @param pinHandles A pointer to PIN_HANDLES struct that contains all the User
 *        Pin pointers.
 * @param wordWidth width of word to know how much to increment the PC
 * @return uint8_t The value to increment the Program Counter by.
 */
// uint8_t getPCIncrement(struct PIN_HANDLES* pinHandles, int wordWidth);
uint8_t getPCIncrement(uint8_t jmpPin, uint8_t rtnPin, uint8_t flagFPin, uint8_t flagOPin, int wordWidth);


/**
 * @brief Decodes the Instruction based on configuration provided by the
 *        OPTIONS struct from the programROMValue.
 * @param programROMValue The value read from ROM that contains the combination
 *        Instruction and Address Data to be decoded.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 * @returns uint32_t The value of the decoded Instruction.
 */
uint32_t decodeInstruction(uint32_t programROMValue, uint8_t wordWidth, uint8_t instructionWidth, uint8_t instructionPosition);

/**
 * @brief Decodes the Address based on configuration provided by the
 *        OPTIONS struct from the programROMValue.
 * @param programROMValue The value read from ROM that contains the combination
 *        Instruction and Address Data to be decoded.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 * @returns uint32_t The value of the decoded Address.
 */
uint32_t decodeAddress(uint32_t programROMValue, uint8_t wordWidth, uint8_t addressWidth, uint8_t addressPosition);

uint32_t readWordFromROM(uint32_t* programROM, uint16_t pc, uint8_t wordWidth, uint8_t endianess);


/**
 * @brief 
 * @param programROM A pointer to a uint32_t array to store the progam file 
 *        contents.
 * @param sOptions A pointer to an OPTIONS struct that contains all the 
 *        configuration parameters for the system.
 * @return uint8_t Return 0 for success or 1 if any part fails.
 */
uint8_t programROMFromFile(uint32_t* programROM, char* filename, uint32_t romSize);

#endif