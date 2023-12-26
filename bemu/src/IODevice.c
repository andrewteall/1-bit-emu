#include <inttypes.h>

#include "IODevice.h"
#include "ulog.h"

void initIODeviceList(struct IODevice* deviceList, uint16_t deviceListSize){
	for(int deviceAddress = 0; deviceAddress < deviceListSize; deviceAddress++){
		deviceList[deviceAddress].type = IO;
		deviceList[deviceAddress].valueContainer = 0;
        deviceList[deviceAddress].value = &deviceList[deviceAddress].valueContainer;
	}
}

int setIODeviceValue(struct IODevice* deviceList, uint16_t address, uint8_t value, uint16_t maxNumDevices){
	int err = 0;
	if(maxNumDevices > address){
		deviceList[address].valueContainer = value;
	} else {
		ulog(ERROR,"Cannot set Device Value. Address: %i is greater than the maximum number of devices: %i",address,maxNumDevices);
		err = 1;
	}
	return err;
}

int setIODeviceType(struct IODevice* deviceList, uint16_t address, enum ioDeviceType type, uint16_t maxNumDevices){
	int err = 0;
	if(maxNumDevices > address){
		if(type < NUM_IO_DEVICE_TYPES){
			deviceList[address].type = type;
		} else {
			ulog(ERROR,"Unable to set IO Device Type: %i Type out of Range",type);
			err = 1;
		}
	} else {
		ulog(ERROR,"Cannot set Device Value. Address: %i is greater than the maximum number of devices: %i",address,maxNumDevices);
		err = 1;
	}
	return err;
}

int bindResultRegisterPinToIOAddress(struct IODevice* deviceList, uint32_t address,uint8_t* resultsRegisterPin,uint16_t maxNumDevices){
	int err = 0;
	if(address < maxNumDevices){
		deviceList[address].type  = INPUT;
		deviceList[address].value = resultsRegisterPin;
	} else {
		ulog(ERROR,"Can not bind Results Register Pin to Device. Address %i greater than number of devices",address);
		err = 1;
	}
	return err;
}

void latchIODeviceValueToDataPin(struct IODevice* deviceList, uint32_t address, uint8_t* dataPin, uint16_t maxNumDevices){
	if(deviceList[address].type != OUTPUT){
		if(maxNumDevices > address){
			*dataPin = *deviceList[address].value;
		} else {
			// TODO: Address will actually be equal to what ever the bit alignment of the address to the MaxNumDevices
			ulog(WARNING,"Can not read device at address: 0x%02x is greater than the maximum number of devices: 0x%02x",address,maxNumDevices);
			*dataPin = 0;
		}
	}
}

void latchDataPinToIODevice(struct IODevice* deviceList, uint32_t address, uint8_t* dataPin, uint8_t writePin, uint16_t maxNumDevices){
	if (writePin){
		if(deviceList[address].type != INPUT){
			if(maxNumDevices > address){
				*deviceList[address].value = (writePin & *dataPin) | ((writePin) & *deviceList[address].value);
			}  else {
				ulog(WARNING,"Can not write device at address: 0x%02x is greater than the maximum number of devices: 0x%02x",address,maxNumDevices);
				// TODO: Address will actually be equal to what ever the bit alignment of the address to the MaxNumDevices
			}
		} else {
			ulog(WARNING,"Attempting to write to an INPUT Device at address: %i", address);
		}
		
	} 
}
