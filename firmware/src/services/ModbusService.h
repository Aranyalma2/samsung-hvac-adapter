#ifndef MODBUS_SERVICE_H
#define MODBUS_SERVICE_H

#include <vector>
#include "../models/Group.h"
#include "PreferencesService.h"

/**
 * ModbusService manages all modbus groups and their data
 * - CRUD operations for groups
 * - Register management at group and slave level
 * - Persistence through PreferencesService
 */
class ModbusService {
private:
    static std::vector<Group> groups;
    static bool initialized;
    
public:
    /**
     * Initialize ModbusService - load groups from persistent storage
     */
    static bool init() {
        if (initialized) return true;
        
        if (!PreferencesService::loadGroups(groups)) {
            Serial.println("[ModbusService] Warning: Could not load groups from storage");
            groups.clear();
        }
        
        initialized = true;
        return true;
    }
    
    /**
     * Get all groups (const reference)
     */
    static const std::vector<Group>& getGroups() {
        return groups;
    }
    
    /**
     * Get all groups (non-const for internal services like mapping)
     */
    static std::vector<Group>& getGroupsMutable() {
        return groups;
    }
    
    /**
     * Get specific group by ID
     */
    static Group* getGroup(uint8_t groupId) {
        for (auto& group : groups) {
            if (group.id == groupId) {
                return &group;
            }
        }
        return nullptr;
    }
    
    /**
     * Create a new group
     */
    static bool createGroup(uint8_t groupId, uint8_t slaveCount = 0, uint8_t remoteAddress = 0) {
        // Check if group already exists
        if (getGroup(groupId)) {
            Serial.println("[ModbusService] Group already exists");
            return false;
        }
        
        // If remoteAddress is 0, default to groupId
        if (remoteAddress == 0) {
            remoteAddress = groupId;
        }
        
        Group newGroup(groupId, remoteAddress, "Outdoor Device " + String(groupId));
        newGroup.updateSlaveCount(slaveCount);
        groups.push_back(newGroup);
        
        if (!save()) {
            groups.pop_back();  // Rollback
            return false;
        }
        
        Serial.printf("[ModbusService] Created group %d (remote: %d)\n", groupId, remoteAddress);
        return true;
    }
    
    /**
     * Update group local ID (with duplicate check)
     */
    static bool updateGroupLocalId(uint8_t oldGroupId, uint8_t newGroupId) {
        // If IDs are the same, nothing to do
        if (oldGroupId == newGroupId) {
            return true;
        }
        
        // Check if new ID already exists
        if (getGroup(newGroupId)) {
            Serial.printf("[ModbusService] Group ID %d already exists\n", newGroupId);
            return false;
        }
        
        auto* group = getGroup(oldGroupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        group->id = newGroupId;
        
        if (!save()) {
            group->id = oldGroupId; // Rollback
            return false;
        }
        
        Serial.printf("[ModbusService] Updated group local ID from %d to %d\n", oldGroupId, newGroupId);
        return true;
    }
    
    /**
     * Update group remote address
     */
    static bool updateGroupRemoteAddress(uint8_t groupId, uint8_t remoteAddress) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        group->remoteAddress = remoteAddress;
        
        if (!save()) {
            return false;
        }
        
        Serial.printf("[ModbusService] Updated group %d remote address to %d\n", groupId, remoteAddress);
        return true;
    }
    
    /**
     * Update group (number of slaves)
     */
    static bool updateGroup(uint8_t groupId, uint8_t newSlaveCount) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        group->updateSlaveCount(newSlaveCount);
        
        if (!save()) {
            // Could restore old state here if needed
            return false;
        }
        
        Serial.print("[ModbusService] Updated group ");
        Serial.println(groupId);
        return true;
    }
    
    /**
     * Delete a group
     */
    static bool deleteGroup(uint8_t groupId) {
        for (size_t i = 0; i < groups.size(); i++) {
            if (groups[i].id == groupId) {
                groups.erase(groups.begin() + i);
                
                if (!save()) {
                    // Could restore here if needed
                    return false;
                }
                
                Serial.print("[ModbusService] Deleted group ");
                Serial.println(groupId);
                return true;
            }
        }
        
        Serial.println("[ModbusService] Group not found");
        return false;
    }
    
    /**
     * Add register to group
     */
    static bool addGroupRegister(uint8_t groupId, const Register& reg) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        if (!group->addRegister(reg)) {
            Serial.println("[ModbusService] Register ID already exists");
            return false;
        }
        
        return save();
    }
    
    /**
     * Update group register
     */
    static bool updateGroupRegister(uint8_t groupId, uint16_t regId, 
                                   const String& newName, uint16_t newId) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        if (!group->updateRegister(regId, newName, newId)) {
            Serial.println("[ModbusService] Register not found or duplicate ID");
            return false;
        }
        
        return save();
    }
    
    /**
     * Delete group register
     */
    static bool deleteGroupRegister(uint8_t groupId, uint16_t regId) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        if (!group->deleteRegister(regId)) {
            Serial.println("[ModbusService] Register not found");
            return false;
        }
        
        return save();
    }
    
    /**
     * Add register to slave
     */
    static bool addSlaveRegister(uint8_t groupId, uint8_t slaveId, const Register& reg) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        auto* slave = group->getSlave(slaveId);
        if (!slave) {
            Serial.println("[ModbusService] Slave not found");
            return false;
        }
        
        if (!slave->addRegister(reg)) {
            Serial.println("[ModbusService] Register ID already exists in slave");
            return false;
        }
        
        return save();
    }
    
    /**
     * Update slave register
     */
    static bool updateSlaveRegister(uint8_t groupId, uint8_t slaveId, 
                                   uint16_t regId, const String& newName, uint16_t newId) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        auto* slave = group->getSlave(slaveId);
        if (!slave) {
            Serial.println("[ModbusService] Slave not found");
            return false;
        }
        
        if (!slave->updateRegister(regId, newName, newId)) {
            Serial.println("[ModbusService] Register not found or duplicate ID");
            return false;
        }
        
        return save();
    }
    
    /**
     * Delete slave register
     */
    static bool deleteSlaveRegister(uint8_t groupId, uint8_t slaveId, uint16_t regId) {
        auto* group = getGroup(groupId);
        if (!group) {
            Serial.println("[ModbusService] Group not found");
            return false;
        }
        
        auto* slave = group->getSlave(slaveId);
        if (!slave) {
            Serial.println("[ModbusService] Slave not found");
            return false;
        }
        
        if (!slave->deleteRegister(regId)) {
            Serial.println("[ModbusService] Register not found");
            return false;
        }
        
        return save();
    }
    
    /**
     * Update register value (called when modbus reads new values)
     */
    static bool updateRegisterValue(uint8_t groupId, uint16_t regId, uint16_t value) {
        auto* group = getGroup(groupId);
        if (!group) return false;
        
        auto* reg = group->getRegister(regId);
        if (reg) {
            reg->value = value;
            return true;
        }
        
        return false;
    }
    
    /**
     * Update slave register value
     */
    static bool updateSlaveRegisterValue(uint8_t groupId, uint8_t slaveId, 
                                        uint16_t regId, uint16_t value) {
        auto* group = getGroup(groupId);
        if (!group) return false;
        
        auto* slave = group->getSlave(slaveId);
        if (!slave) return false;
        
        auto* reg = slave->getRegister(regId);
        if (reg) {
            reg->value = value;
            return true;
        }
        
        return false;
    }
    
    /**
     * Save all groups to persistent storage
     */
    static bool save() {
        return PreferencesService::saveGroups(groups);
    }
    
    /**
     * Get count of groups
     */
    static size_t getGroupCount() {
        return groups.size();
    }
};

#endif // MODBUS_SERVICE_H
