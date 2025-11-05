#include "Comport1.h"
#include "Comport2.h"
#include "../services/StatusService.h"
#include "../services/RegisterMappingService.h"
#include "../services/ModbusService.h"

#define MAX_REGISTERS 65535

void Comport1::setup(uint32_t baudrate, SerialConfig config, Comport2* com2) {
    _comport2 = com2;
    RTUutils::prepareHardwareSerial(_COM);
    _COM.begin(baudrate, config, COMPORT1_RX, COMPORT1_TX);

    _modbus.registerWorker(
        ANY_SERVER,
        READ_HOLD_REGISTER,
        [this](ModbusMessage request) {
            return this->slaveHandlerFC03(request);
        });

    _modbus.registerWorker(
        ANY_SERVER,
        WRITE_HOLD_REGISTER,
        [this](ModbusMessage request) {
            return this->slaveHandlerFC06(request);
        });

    _modbus.begin(_COM);
}

// FC03: worker do serve Modbus function code 0x03 (READ_HOLD_REGISTER)
ModbusMessage Comport1::slaveHandlerFC03(ModbusMessage request) {

  StatusService::addUart1Received(1);

  uint8_t serverID = request.getServerID();
  uint16_t address;           // requested register address
  uint16_t words;             // requested number of registers
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, words);

  // Check if group exists in mapping
  if (!RegisterMappingService::groupExists(serverID)) {
    Serial.printf("[COM1] FC03: Unknown group/server ID %d\n", serverID);
    response.setError(serverID, request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    StatusService::addUart1Sent(1);
    return response;
  }

  // Get total register count for this group
  size_t maxRegs = RegisterMappingService::getRegisterCount(serverID);

  // Address and words validation
  if (words > 0 && words <= 125 && (address + words) <= maxRegs) {
    // Looks okay. Set up message with serverID, FC and length of data
    response.add(serverID, request.getFunctionCode(), (uint8_t)(words * 2));
    
    // Fill response with requested data from mapped registers
    bool allFound = true;
    for (uint16_t i = address; i < address + words; i++) {
        uint16_t value = 0;
        if (RegisterMappingService::readRegister(serverID, i, value)) {
          response.add(value);
        } else {
          allFound = false;
          break;
        }
    }
    
    if (!allFound) {
      Serial.printf("[COM1] FC03: Register not found in mapping at address %d\n", address);
      response.setError(serverID, request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    }
  } else {
    // No, either address or words are outside the limits. Set up error response.
    Serial.printf("[COM1] FC03: Illegal address %d or words %d (max: %d)\n", address, words, maxRegs);
    response.setError(serverID, request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  
  StatusService::addUart1Sent(1);

  return response;
}

// FC06: worker to serve Modbus function code 0x06 (WRITE_HOLD_REGISTER)
ModbusMessage Comport1::slaveHandlerFC06(ModbusMessage request) {

  StatusService::addUart1Received(1);

  uint8_t serverID = request.getServerID();
  uint16_t address;           // requested register address
  uint16_t value;             // value to write
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, value);

  // Check if group exists in mapping
  if (!RegisterMappingService::groupExists(serverID)) {
    Serial.printf("[COM1] FC06: Unknown group/server ID %d\n", serverID);
    response.setError(serverID, request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    StatusService::addUart1Sent(1);
    return response;
  }

  // Get register info (slave ID and actual register ID)
  uint8_t slaveId = 0;
  uint16_t actualRegId = 0;
  
  if (!RegisterMappingService::getRegisterInfo(serverID, address, slaveId, actualRegId)) {
    Serial.printf("[COM1] FC06: Address %d not found in mapping\n", address);
    response.setError(serverID, request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
    StatusService::addUart1Sent(1);
    return response;
  }

  // Get remote address for this group
  auto* group = ModbusService::getGroup(serverID);
  uint8_t remoteAddress = group ? group->remoteAddress : serverID;
  
  // Trigger write on COM2 (pass value directly, don't use global data)
  if (_comport2) {
    // Create token for tracking
    uint32_t token = ((uint32_t)serverID << 24) | ((uint32_t)slaveId << 16) | actualRegId;
    
    // Send write request to COM2 (FC06: WRITE_HOLD_REGISTER)
    bool success = _comport2->addRequest(
      token,
      remoteAddress,         // Use remote Modbus address on COM2
      WRITE_HOLD_REGISTER,   // FC06
      actualRegId,           // Actual register address
      value                  // Value to write
    );
    
    if (success) {
      Serial.printf("[COM1] FC06 -> COM2: Write Group %d, Slave %d, Reg %d = %d\n", 
                   serverID, slaveId, actualRegId, value);
    } else {
      Serial.printf("[COM1] FC06: Failed to queue write request\n");
      response.setError(serverID, request.getFunctionCode(), REQUEST_QUEUE_FULL);
      StatusService::addUart1Sent(1);
      return response;
    }
  } else {
    Serial.printf("[COM1] FC06: COM2 not available for write\n");
    response.setError(serverID, request.getFunctionCode(), REQUEST_QUEUE_FULL);
    StatusService::addUart1Sent(1);
    return response;
  }

  // Send success response (echo back the address and value)
  response.add(serverID, request.getFunctionCode());
  response.add(address);
  response.add(value);

  StatusService::addUart1Sent(1);

  return response;
}