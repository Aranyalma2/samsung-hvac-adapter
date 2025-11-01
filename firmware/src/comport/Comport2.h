#ifndef __COMPORT2_H__
#define __COMPORT2_H__

#include <ModbusClientRTU.h>

#define COMPORT2_RX 32
#define COMPORT2_TX 33
#define COMPORT2_TX_EN 12

//#define COMPORT2_RX 5
//#define COMPORT2_TX 17
//#define COMPORT2_TX_EN 33

// SLAVE MODE

class Comport2 {
public:
    Comport2() : _COM(2), _modbus(COMPORT2_TX_EN) {
    };
    void setup(uint32_t baudrate, SerialConfig config);
    
    // Add a request (made public for polling service)
    boolean addRequest(uint32_t token, uint8_t slaveAddress, FunctionCode functionCode, uint16_t registerAddress, uint16_t value_or_count);

private:
    HardwareSerial _COM;
    ModbusClientRTU _modbus;

    // Handle the response
    void handleData(ModbusMessage response, uint32_t token);

    // Handle the error
    void handleError(Error error, uint32_t token);

};
#endif // __COMPORT2_H__