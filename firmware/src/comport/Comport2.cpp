#include "Comport2.h"
#include "../services/ModbusService.h"
#include "../services/StatusService.h"
#include "../services/ModbusPollingService.h"

void Comport2::setup(uint32_t baudrate, SerialConfig config) {
    RTUutils::prepareHardwareSerial(_COM);
    _COM.begin(baudrate, config, COMPORT2_RX, COMPORT2_TX);

    _modbus.onDataHandler([this](ModbusMessage response, uint32_t token) {this->handleData(response, token);});
    _modbus.onErrorHandler([this](Error error, uint32_t token) {this->handleError(error, token);});

    _modbus.setTimeout(500);

    _modbus.begin(_COM);
}

boolean Comport2::addRequest(uint32_t token, uint8_t slaveAddress, FunctionCode functionCode, uint16_t registerAddress, uint16_t value_or_count) {

    Error err = _modbus.addRequest(token, slaveAddress, functionCode, registerAddress, value_or_count);
    if (err!=SUCCESS) {
        ModbusError e(err);
        Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        return false;
    }
    StatusService::addUart2Sent(1);
    return true;
}


void Comport2::handleData(ModbusMessage response, uint32_t token) {
    // Decrement ongoing request counter
    ModbusPollingService::onResponseReceived();
    
    // Decode token to get group ID, slave ID, and register ID
    uint8_t groupId = (token >> 24) & 0xFF;
    uint8_t slaveId = (token >> 16) & 0xFF;
    uint16_t registerId = token & 0xFFFF;
    
    // Extract register value from response
    // For holding register read (function code 0x03), data starts at byte 2
    if (response.size() >= 4) {  // At least: slave + fc + byte_count + 2 bytes data
        uint16_t value = 0;
        response.get(3, value);
        
        // Update the register value in ModbusService
        bool updated = false;
        if (slaveId == 0) {
            // Group-level register
            updated = ModbusService::updateRegisterValue(groupId, registerId, value);
            Serial.printf("[Response] Group %d, Register %d : %d\n", 
                         groupId, registerId, value);
        } else {
            // Slave register
            updated = ModbusService::updateSlaveRegisterValue(groupId, slaveId, registerId, value);
            Serial.printf("[Response] Group %d, Slave %d, Register %d : %d\n", 
                         groupId, slaveId, registerId, value);
        }
        
        // Update UART statistics
        StatusService::addUart2Received(1);
    }
}

void Comport2::handleError(Error error, uint32_t token) {
    // Decrement ongoing request counter (even on error)
    ModbusPollingService::onResponseReceived();
    
    uint8_t groupId = (token >> 24) & 0xFF;
    uint8_t slaveId = (token >> 16) & 0xFF;
    uint16_t registerId = token & 0xFFFF;
    
    ModbusError e(error);
    if (slaveId == 0) {
        Serial.printf("[Error] Group %d, Register %d: %02X - %s\n", 
                     groupId, registerId, (int)e, (const char *)e);
    } else {
        Serial.printf("[Error] Group %d, Slave %d, Register %d: %02X - %s\n", 
                     groupId, slaveId, registerId, (int)e, (const char *)e);
    }
    StatusService::addUart2Received(1);
}