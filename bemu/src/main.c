#include "MCUtils.h"
#include "MCSystem.h"
#include "MC14500.h"
#include "IODevice.h"
#include "debugger.h"
#include "../../ulog/include/ulog.h"

int main(int argc, char* argv[]){
    uint8_t error = 0;
    if (argc == 1){
        printUsage();
        ulog(ERROR,"Filename to open must be specified. Exiting now");
        return 1;
	} else {
        /*********************************************************************/
        /************************* Init Program ******************************/
        struct OPTIONS sOptions;
        error = parseCommandLineOptions(&sOptions,argc,argv);
        /*********************************************************************/
        
        /*********************************************************************/
        /********************** Setup Program Counter ************************/
        // Setup Program Counter 
        uint16_t pc = sOptions.pcInitAddress-sOptions.wordWidth;
        /*********************************************************************/

        /*********************************************************************/
        /*************************** Setup ICU *******************************/
        struct MC14500 icu;
        initICU(&icu);
        setPinHandlers(&sOptions.pinHandles, &icu.jmpPin, &icu.rtnPin, \
                                                &icu.flagOPin,&icu.flagFPin);
        /*********************************************************************/

        /*********************************************************************/
        /*************************** Setup ROM *******************************/
        // Define the Read Only Memory to be used by the ICU
        uint32_t programROM[sOptions.romSize];
        // Program Rom
        error += !error * programROMFromFile(programROM, &sOptions);
        
        /*********************************************************************/

        /*********************************************************************/
        /************************* Setup Stack *******************************/
        uint32_t stack[sOptions.stackSize];
        uint8_t  sp  = !sOptions.stackDir*(sOptions.stackSize-1);
        /*********************************************************************/

        /*********************************************************************/
        /************************** Setup I/O ********************************/
        // Array to store input and output devices and their addresses
        struct IODevice deviceList[sOptions.ioDeviceCount];
        initIODeviceList(deviceList,sOptions.ioDeviceCount);

        if (sOptions.bindResultsRegister){
            error += !error * \
                bindResultRegisterPinToIOAddress(deviceList,sOptions.rrDeviceAddress,icu.resultsRegisterPin,sOptions.ioDeviceCount);
        }
        
        // struct IODevice ioDeviceList[0xF];
        // struct IODevice inputDeviceList[0xF];
        // struct IODevice outputDeviceList[0xF];
        /*********************************************************************/
        
        if(sOptions.enableDebugger && !error){
            error += !error * startDebugger(&sOptions);
        }

        uint32_t address;
        uint8_t instruction;
        uint32_t programROMValue;
        startICU(&icu);
        while(icu.status == RUNNING && !error){
            // Fetch - Clock Up
            pc += getPCIncrement(&sOptions.pinHandles, sOptions.wordWidth); // 0 if we modify the PC outside of this line
            programROMValue = readWordFromROM(programROM,pc,&sOptions);
            address = decodeAddress(programROMValue,&sOptions);
            instruction = decodeInstruction(programROMValue, &sOptions);
            fetch(&icu, instruction);               // icu "fetch"
              
            // Execute - Clock Down
            latchIODeviceValueToDataPin(deviceList,address,&icu.dataPin,sOptions.ioDeviceCount); // Latch device at address to Data Pin
            execute(&icu); // icu "execute"
            latchDataPinToIODevice(deviceList,address,&icu.dataPin,icu.writePin,sOptions.ioDeviceCount);// Latch Data Pin out to device
            
            if(sOptions.enableDebugger){
                drawScreen(&sOptions,pc, address, &icu,deviceList,stack,&sp);
            } else if(sOptions.printState){
                printSystemInfo(pc, address, &icu);
            }
            
            // User Defined Pin Handling
            error += !error * pinHandler(&icu,stack,&sp,&pc,&sOptions,address);

            (pc >= sOptions.romSize) ? pc = 0 : 0; // Reset PC if we overflow the "ROM"
        }

        if(sOptions.enableDebugger){
            stopDebugger();
        }
        return error;
    }
}