#ifndef GROUP_H
#define GROUP_H

#include <ArduinoJson.h>
#include <vector>
#include "Register.h"
#include "Slave.h"

/**
 * Group represents an outdoor device/unit
 * Contains group-level registers and a list of slaves (indoor devices)
 */
class Group {
public:
    uint8_t id;                         // Group ID / Local Modbus address on COM1 (0-255)
    uint8_t remoteAddress;              // Remote Modbus address on COM2 (0-255)
    String name;                        // Group name (e.g., "Outdoor Device 1")
    std::vector<Register> registers;    // Group-level registers
    std::vector<Slave> slaves;          // List of slaves (indoor devices)
    
    Group() : id(0), remoteAddress(0), name("") {}
    
    explicit Group(uint8_t id) : id(id), remoteAddress(id) {
        name = "Outdoor Device " + String(id);
    }
    
    Group(uint8_t id, const String& name) : id(id), remoteAddress(id), name(name) {}
    
    Group(uint8_t id, uint8_t remote, const String& name) : id(id), remoteAddress(remote), name(name) {}
    
    // Add a new group-level register
    bool addRegister(const Register& reg) {
        // Check for duplicate ID
        for (const auto& r : registers) {
            if (r.id == reg.id) {
                return false;  // Duplicate
            }
        }
        registers.push_back(reg);
        return true;
    }
    
    // Get group-level register by ID
    Register* getRegister(uint16_t regId) {
        for (auto& r : registers) {
            if (r.id == regId) {
                return &r;
            }
        }
        return nullptr;
    }
    
    // Update group-level register
    bool updateRegister(uint16_t regId, const String& newName, uint16_t newId) {
        // Find current register
        auto* reg = getRegister(regId);
        if (!reg) return false;
        
        // Check if new ID already exists
        if (newId != regId) {
            for (const auto& r : registers) {
                if (r.id == newId) {
                    return false;  // Duplicate
                }
            }
        }
        
        reg->id = newId;
        reg->name = newName;
        return true;
    }
    
    // Delete group-level register
    bool deleteRegister(uint16_t regId) {
        for (size_t i = 0; i < registers.size(); i++) {
            if (registers[i].id == regId) {
                registers.erase(registers.begin() + i);
                return true;
            }
        }
        return false;
    }
    
    // Add a new slave
    bool addSlave(const Slave& slave) {
        // Check for duplicate ID
        for (const auto& s : slaves) {
            if (s.id == slave.id) {
                return false;  // Duplicate
            }
        }
        slaves.push_back(slave);
        return true;
    }
    
    // Get slave by ID
    Slave* getSlave(uint8_t slaveId) {
        for (auto& s : slaves) {
            if (s.id == slaveId) {
                return &s;
            }
        }
        return nullptr;
    }
    
    // Delete slave
    bool deleteSlave(uint8_t slaveId) {
        for (size_t i = 0; i < slaves.size(); i++) {
            if (slaves[i].id == slaveId) {
                slaves.erase(slaves.begin() + i);
                return true;
            }
        }
        return false;
    }
    
    // Update number of slaves (add or remove)
    void updateSlaveCount(uint8_t newCount) {
        uint8_t currentCount = slaves.size();
        
        if (newCount > currentCount) {
            // Add new slaves
            for (uint8_t i = currentCount + 1; i <= newCount; i++) {
                slaves.push_back(Slave(i));
            }
        } else if (newCount < currentCount) {
            // Remove slaves
            while (slaves.size() > newCount) {
                slaves.pop_back();
            }
        }
    }
    
    // Serialize to JSON
    void toJson(JsonObject& obj, bool withValues = true) const {
        obj["id"] = id;
        obj["remote_address"] = remoteAddress;
        obj["name"] = name;
        
        auto regArray = obj.createNestedArray("registers");
        for (const auto& reg : registers) {
            auto regObj = regArray.createNestedObject();
            if (withValues) {
                reg.toJson(regObj);
            } else {
                reg.toJsonWithoutValue(regObj);
            }
        }
        
        auto slavesArray = obj.createNestedArray("slaves");
        for (const auto& slave : slaves) {
            auto slaveObj = slavesArray.createNestedObject();
            slave.toJson(slaveObj, withValues);
        }
    }
    
    // Deserialize from JSON
    static Group fromJson(const JsonObject& obj) {
        uint8_t localId = obj["id"] | 0;
        uint8_t remoteAddr = obj["remote_address"] | localId; // Default to local ID if not specified
        Group group(localId, remoteAddr, obj["name"].as<String>());
        
        // Load registers
        if (obj.containsKey("registers")) {
            auto regsArray = obj["registers"].as<JsonArray>();
            for (const auto& regObj : regsArray) {
                group.registers.push_back(Register::fromJson(regObj));
            }
        }
        
        // Load slaves
        if (obj.containsKey("slaves")) {
            auto slavesArray = obj["slaves"].as<JsonArray>();
            for (const auto& slaveObj : slavesArray) {
                group.slaves.push_back(Slave::fromJson(slaveObj));
            }
        }
        
        return group;
    }
};

#endif // GROUP_H
