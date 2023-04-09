#ifndef MCSYSTEM_H
    #define MCSYSTEM_H 1
    
#include <inttypes.h>

#include "MCUtils.h"

#ifndef UP
    #define UP 1
#endif

#ifndef DOWN
    #define DOWN 0
#endif

enum pinActions {NONE,JUMP,JSR,RET,JSRS,RETS,HLT,RES,};
const char* pinActionsStrings[9];

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
uint8_t pinHandler(struct MC14500* icu,uint32_t* stack, uint8_t* sp, uint16_t* pc, struct OPTIONS* sOptions, uint32_t address);

void setPinHandlers(struct PIN_HANDLES* pinHandles,uint8_t* jmpPin,uint8_t* rtnPin,uint8_t* flagOPin,uint8_t* flagFPin);

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
uint8_t getPCIncrement(struct PIN_HANDLES* pinHandles, int wordWidth);


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

void initStack(uint32_t* stack, uint8_t* sp, struct OPTIONS* sOptions);

uint32_t readWordFromROM(uint32_t* programROM, uint16_t pc, struct OPTIONS* sOptions);

#endif