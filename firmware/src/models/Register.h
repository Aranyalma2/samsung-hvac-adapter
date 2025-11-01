#ifndef REGISTER_H
#define REGISTER_H

#include <ArduinoJson.h>

/**
 * Register represents a single Modbus register
 * Can be used at group level or slave level
 */
class Register {
public:
    uint16_t id;        // Register address (0-65535)
    String name;        // Register name (user-defined)
    uint16_t value;     // Current register value (read from Modbus)
    
    Register() : id(0), name(""), value(0) {}
    
    Register(uint16_t id, const String& name, uint16_t value = 0) 
        : id(id), name(name), value(value) {}
    
    // Serialize to JSON
    void toJson(JsonObject& obj) const {
        obj["id"] = id;
        obj["name"] = name;
        obj["value"] = value;
    }
    
    // Serialize to JSON (without value)
    void toJsonWithoutValue(JsonObject& obj) const {
        obj["id"] = id;
        obj["name"] = name;
    }
    
    // Deserialize from JSON (value is initialized to 0, will be updated from Modbus)
    static Register fromJson(const JsonObject& obj) {
        Register reg;
        reg.id = obj["id"] | 0;
        reg.name = obj["name"].as<String>();
        reg.value = 0;  // Always start at 0, will be updated from Modbus polling
        return reg;
    }
};

#endif // REGISTER_H
