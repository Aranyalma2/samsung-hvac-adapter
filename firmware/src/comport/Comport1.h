#include <ModbusServerRTU.h>

#define COMPORT1_RX 5
#define COMPORT1_TX 17
//#define COMPORT1_TX_EN 4
#define COMPORT1_TX_EN 33

// SLAVE MODE

class Comport1 {
public:
    Comport1() : _COM1(1), _modbus(20000, COMPORT1_TX_EN) {};
    void setup(uint32_t baudrate, SerialConfig config, uint8_t slaveAddress);
private:
    HardwareSerial _COM1;
    ModbusServerRTU _modbus;

    // Slave mode request handler
    ModbusMessage slaveHandlerFC03(ModbusMessage request);
    ModbusMessage slaveHandlerFC06(ModbusMessage request);
};