#include "IODevice.h"

void initIODeviceList(struct IODevice* deviceList,uint16_t deviceListSize){
	for(int i=0;i<deviceListSize;i++){
		deviceList[i].valueContainer = 0;
        deviceList[i].value = &deviceList[i].valueContainer;
	}
}

void setIODeviceValue(struct IODevice* deviceList, uint16_t device, uint8_t value){
	deviceList[device].valueContainer = value;
}

void bindResultRegisterPinToIOAddress(struct IODevice* deviceList, uint16_t address, uint8_t* resultsRegisterPin){
	deviceList[address].value = resultsRegisterPin;
}