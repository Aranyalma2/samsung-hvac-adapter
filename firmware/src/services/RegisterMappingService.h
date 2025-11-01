#ifndef REGISTER_MAPPING_SERVICE_H
#define REGISTER_MAPPING_SERVICE_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "ModbusService.h"

/**
 * RegisterMappingService creates fast pointer-based mappings 
 * from group ID + register address to register values
 * 
 * Group ID = Modbus Server ID on COM1
 * Registers are mapped sequentially: group registers, then slave registers
 */
class RegisterMappingService {
private:
    // Map structure: [groupId][registerAddress] -> pointer to value
    static std::map<uint8_t, std::map<uint16_t, uint16_t*>> registerMap;
    static bool initialized;
    
public:
    /**
     * Build the register mapping from current ModbusService groups
     * Call this after loading groups or when groups change
     */
    static void buildMapping() {
        registerMap.clear();
        
        auto& groups = ModbusService::getGroupsMutable();
        
        for (auto& group : groups) {
            uint16_t address = 0;  // Start at address 0 for each group
            
            // Map group-level registers first
            for (auto& reg : group.registers) {
                registerMap[group.id][address] = &reg.value;
                Serial.printf("[Mapping] Group %d: Address %d -> Group Register %d\n", 
                             group.id, address, reg.id);
                address++;
            }
            
            // Then map slave registers sequentially
            for (auto& slave : group.slaves) {
                for (auto& reg : slave.registers) {
                    registerMap[group.id][address] = &reg.value;
                    Serial.printf("[Mapping] Group %d: Address %d -> Slave %d Register %d\n", 
                                 group.id, address, slave.id, reg.id);
                    address++;
                }
            }
            
            Serial.printf("[Mapping] Group %d: Total %d registers mapped\n", group.id, address);
        }
        
        initialized = true;
        Serial.printf("[Mapping] Complete: %d groups mapped\n", groups.size());
    }
    
    /**
     * Get pointer to register value by group ID and address
     * Returns nullptr if not found
     */
    static uint16_t* getRegisterPointer(uint8_t groupId, uint16_t address) {
        auto groupIt = registerMap.find(groupId);
        if (groupIt == registerMap.end()) {
            return nullptr;  // Group not found
        }
        
        auto regIt = groupIt->second.find(address);
        if (regIt == groupIt->second.end()) {
            return nullptr;  // Register not found
        }
        
        return regIt->second;
    }
    
    /**
     * Read register value by group ID and address
     * Returns true if found, value is set in output parameter
     */
    static bool readRegister(uint8_t groupId, uint16_t address, uint16_t& value) {
        uint16_t* ptr = getRegisterPointer(groupId, address);
        if (ptr) {
            value = *ptr;
            return true;
        }
        return false;
    }
    
    /**
     * Write register value by group ID and address
     * Returns true if found and written
     * Note: This writes to the local cache, use with caution
     */
    static bool writeRegister(uint8_t groupId, uint16_t address, uint16_t value) {
        uint16_t* ptr = getRegisterPointer(groupId, address);
        if (ptr) {
            *ptr = value;
            return true;
        }
        return false;
    }
    
    /**
     * Get total register count for a group
     */
    static size_t getRegisterCount(uint8_t groupId) {
        auto groupIt = registerMap.find(groupId);
        if (groupIt == registerMap.end()) {
            return 0;
        }
        return groupIt->second.size();
    }
    
    /**
     * Check if group exists
     */
    static bool groupExists(uint8_t groupId) {
        return registerMap.find(groupId) != registerMap.end();
    }
    
    /**
     * Get original register info (group ID, slave ID, register ID) from mapped address
     * Used for COM2 write requests
     */
    static bool getRegisterInfo(uint8_t groupId, uint16_t address, 
                                uint8_t& outSlaveId, uint16_t& outRegId) {
        const auto& groups = ModbusService::getGroups();
        
        for (const auto& group : groups) {
            if (group.id != groupId) continue;
            
            uint16_t currentAddr = 0;
            
            // Check group-level registers
            for (const auto& reg : group.registers) {
                if (currentAddr == address) {
                    outSlaveId = 0;  // 0 means group-level
                    outRegId = reg.id;
                    return true;
                }
                currentAddr++;
            }
            
            // Check slave registers
            for (const auto& slave : group.slaves) {
                for (const auto& reg : slave.registers) {
                    if (currentAddr == address) {
                        outSlaveId = slave.id;
                        outRegId = reg.id;
                        return true;
                    }
                    currentAddr++;
                }
            }
            
            return false;  // Address not found in this group
        }
        
        return false;  // Group not found
    }
    
    /**
     * Check if initialized
     */
    static bool isInitialized() {
        return initialized;
    }
};

#endif // REGISTER_MAPPING_SERVICE_H
