#include "Comport1.h"

#define MAX_REGISTERS 256

void Comport1::setup(uint32_t baudrate, SerialConfig config, uint8_t slaveAddress) {
    RTUutils::prepareHardwareSerial(_COM1);
    _COM1.begin(baudrate, config, COMPORT1_RX, COMPORT1_TX);

    _modbus.registerWorker(
        slaveAddress,
        READ_HOLD_REGISTER,
        [this](ModbusMessage request) {
            return this->slaveHandlerFC03(request);
        });

    _modbus.registerWorker(
        slaveAddress,
        WRITE_HOLD_REGISTER,
        [this](ModbusMessage request) {
            return this->slaveHandlerFC06(request);
        });

    _modbus.begin(_COM1);
}

// FC03: worker do serve Modbus function code 0x03 (READ_HOLD_REGISTER)
ModbusMessage Comport1::slaveHandlerFC03(ModbusMessage request) {

  uint16_t address;           // requested register address
  uint16_t words;             // requested number of registers
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, words);

  // Address and words validation
  if (address >= 0 && words > 0 && words <= 125 && (address + words) <= MAX_REGISTERS) {
    // Looks okay. Set up message with serverID, FC and length of data
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
    // Fill response with requested data
    for (uint16_t i = address; i < address + words; i++) {
        uint16_t value = 0;
        // HERE fill the value from your internal data source
        response.add(value);
      }
  } else {
    // No, either address or words are outside the limits. Set up error response.
    Serial.printf("Comport1: FC03 request with illegal address %d or words %d\n", address, words);
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}

// FC06: worker to serve Modbus function code 0x06 (WRITE_HOLD_REGISTER)
ModbusMessage Comport1::slaveHandlerFC06(ModbusMessage request) {

  uint16_t address;           // requested register address
  uint16_t value;             // value to write
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, value);

  // Address validation
  if (address >= 0 && address < MAX_REGISTERS) {
    // Looks okay. Set up message with serverID, FC and length of data
    response.add(request.getServerID(), request.getFunctionCode());
    // Write value to the requested register

    // HERE write the value to your internal data source

    response.add(address); // Echo back the written address
    response.add(value); // Echo back the written value
  } else {
    // No, address is outside the limits. Set up error response.
    Serial.printf("Comport1: FC06 request with illegal address %d\n", address);
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}