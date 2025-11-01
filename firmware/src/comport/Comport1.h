#ifndef __COMPORT1_H__
#define __COMPORT1_H__

#include <ModbusServerRTU.h>

#define COMPORT1_RX 5
#define COMPORT1_TX 17
#define COMPORT1_TX_EN 4

//#define COMPORT1_RX 32
//#define COMPORT1_TX 33
//#define COMPORT1_TX_EN 12

// Forward declaration
class Comport2;

// SLAVE MODE

class Comport1 {
public:
    Comport1() : _COM(1), _modbus(20000, COMPORT1_TX_EN), _comport2(nullptr) {};
    void setup(uint32_t baudrate, SerialConfig config, Comport2* com2 = nullptr);
private:
    HardwareSerial _COM;
    ModbusServerRTU _modbus;
    Comport2* _comport2;  // Reference to COM2 for write requests

    // Slave mode request handler
    ModbusMessage slaveHandlerFC03(ModbusMessage request);
    ModbusMessage slaveHandlerFC06(ModbusMessage request);
};

#endif // __COMPORT1_H__