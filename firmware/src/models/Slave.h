#ifndef SLAVE_H
#define SLAVE_H

#include <ArduinoJson.h>
#include <vector>
#include "Register.h"

/**
 * Slave represents an indoor device
 * Contains slave-level registers
 */
class Slave {
public:
    uint8_t id;                         // Slave ID (1-255)
    std::vector<Register> registers;    // List of slave-level registers
    
    Slave() : id(0) {}
    
    explicit Slave(uint8_t id) : id(id) {}
    
    // Add a new register
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
    
    // Get register by ID
    Register* getRegister(uint16_t regId) {
        for (auto& r : registers) {
            if (r.id == regId) {
                return &r;
            }
        }
        return nullptr;
    }
    
    // Update register
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
    
    // Delete register
    bool deleteRegister(uint16_t regId) {
        for (size_t i = 0; i < registers.size(); i++) {
            if (registers[i].id == regId) {
                registers.erase(registers.begin() + i);
                return true;
            }
        }
        return false;
    }
    
    // Serialize to JSON
    void toJson(JsonObject& obj, bool withValues = true) const {
        obj["id"] = id;
        
        auto regArray = obj.createNestedArray("registers");
        for (const auto& reg : registers) {
            auto regObj = regArray.createNestedObject();
            if (withValues) {
                reg.toJson(regObj);
            } else {
                reg.toJsonWithoutValue(regObj);
            }
        }
    }
    
    // Deserialize from JSON
    static Slave fromJson(const JsonObject& obj) {
        Slave slave(obj["id"] | 0);
        
        if (obj.containsKey("registers")) {
            auto regsArray = obj["registers"].as<JsonArray>();
            for (const auto& regObj : regsArray) {
                slave.registers.push_back(Register::fromJson(regObj));
            }
        }
        
        return slave;
    }
};

#endif // SLAVE_H
