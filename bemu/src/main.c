#include <stdio.h>

#include "MC14500.h"
#include "IODevice.h"
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
        uint16_t pc = sOptions.pcInitAddress;
        /*********************************************************************/

        /*********************************************************************/
        /*************************** Setup ICU *******************************/
        struct MC14500 icu;
        initICU(&icu,&sOptions);
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
            bindResultRegisterPinToIOAddress(deviceList,sOptions.rrDeviceAddress,icu.resultsRegisterPin);
        }
        
        // struct IODevice ioDeviceList[0xF];
        // struct IODevice inputDeviceList[0xF];
        // struct IODevice outputDeviceList[0xF];
        /*********************************************************************/
        
        uint32_t address;
        startICU(&icu);
        while(icu.status == RUNNING && !error){
            // Fetch - Clock Up
            pc += getPCIncrement(&sOptions.pinHandles); // 0 if we modify the PC outside of this line, 1 otherwise
            fetch(&icu, programROM[pc], &sOptions);     // icu "fetch"
            address = decodeAddress(programROM[pc], &sOptions);    // "Latch" Address for IODevice
            
            // Execute - Clock Down
            if(sOptions.ioDeviceCount > address){
                latchIODeviceValueToDataPin(&icu.dataPin,deviceList[address].value); // Latch device at address to Data Pin
            }
            execute(&icu);                                                       // icu "execute"
            if(sOptions.ioDeviceCount > address){
                latchDataPinToIODevice(deviceList[address].value,&icu.dataPin,icu.writePin); // Latch Data Pin out to device
            }
            
            if(sOptions.enableDebugger){
                printf("\033c");
            }
            if(sOptions.printState && !sOptions.enableDebugger){
                printSystemInfo(pc, address, &icu);
            }
            
            // User Defined Pin Handling
            error += !error * pinHandler(&icu,stack,&sp,&pc,&sOptions,address);

            (pc >= sOptions.romSize) ? pc = 0 : 0; // Reset PC if we overflow the "ROM"
        }
        return error;
    }
}