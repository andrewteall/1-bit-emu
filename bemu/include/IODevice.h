#ifndef IODEVICE_H
#define IODEVICE_H 1

enum ioDeviceType {INPUT,OUTPUT,IO,NUM_IO_DEVICE_TYPES};

struct IODevice {
    uint8_t type;
    uint8_t* value;
    uint8_t valueContainer;
};

void initIODeviceList(struct IODevice* deviceList, uint16_t deviceListSize);

int setIODeviceValue(struct IODevice* deviceList, uint16_t device, uint8_t value, uint16_t maxNumDevices);

int setIODeviceType(struct IODevice* deviceList, uint16_t address, enum ioDeviceType type, uint16_t maxNumDevices);

int bindResultRegisterPinToIOAddress(struct IODevice* deviceList, uint32_t address, uint8_t* resultsRegisterPin, uint16_t maxNumDevices);

/**
 * @brief Assigns the Value of the device at the specified address to the 
 *        Data Pin of the icu.
 * @param deviceList A Pointer to the DeviceList.
 * @param dataPin A Pointer to the data Pin of the ICU to recieve the data.
 *        to latch.
 * @param maxNumDevices The number of IODevices specified in the Options
 */
void latchIODeviceValueToDataPin(struct IODevice* deviceList, uint32_t address,uint8_t* dataPin, uint16_t maxNumDevices);

/**
 * @brief Assigns the Value of the Data Pin of the icu to the device at the 
 *        specified address only if the Write Pin is High.
 * @param deviceList A Pointer to the DeviceList.
 * @param dataPin A Pointer to the Data Pin of the ICU to recieve the data.
 * @param writePin The value of the Write Pin if the ICU.
 * @param maxNumDevices The number of IODevices specified in the Options
 * 
 */
void latchDataPinToIODevice(struct IODevice* deviceList, uint32_t address, uint8_t* dataPin, uint8_t writePin, uint16_t maxNumDevices);

#endif