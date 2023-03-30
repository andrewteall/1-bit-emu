#ifndef IODEVICE_H
    #define IODEVICE_H 1

#include <inttypes.h>

struct IODevice {
    uint8_t type;
    uint8_t* value;
    uint8_t* valueContainerPtr;
    uint8_t valueContainer;
};

void initIODeviceList(struct IODevice* deviceList,uint16_t deviceListSize);
void setIODeviceValue(struct IODevice* deviceList, uint16_t device, uint8_t value);
void bindResultRegisterPinToIOAddress(struct IODevice* deviceList, uint16_t address, uint8_t* resultsRegisterPin);


#endif