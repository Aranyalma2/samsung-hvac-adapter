#ifndef INTERFACES_DATA_H
#define INTERFACES_DATA_H

#include <ArduinoJson.h>

/**
 * InterfaceConfig represents UART interface configuration
 */
class InterfaceConfig {
public:
    uint32_t baudrate;      // Baud rate
    uint8_t dataBits;       // Data bits (5, 6, 7, 8)
    uint8_t stopBits;       // Stop bits (1, 2)
    uint8_t parity;         // Parity (0=None, 1=Odd, 2=Even)
    
    InterfaceConfig(uint32_t baud = 19200, uint8_t data = 8, 
                    uint8_t stop = 1, uint8_t par = 0)
        : baudrate(baud), dataBits(data), stopBits(stop), parity(par) {}
    
    // Serialize to JSON
    void toJson(JsonObject& obj, const String& prefix) const {
        obj[prefix + "_baud"] = baudrate;
        obj[prefix + "_data"] = dataBits;
        obj[prefix + "_stop"] = stopBits;
        obj[prefix + "_parity"] = parity;
    }
    
    // Validate configuration
    bool isValid() const {
        if (dataBits < 5 || dataBits > 8) return false;
        if (stopBits < 1 || stopBits > 2) return false;
        if (parity > 2) return false;
        
        switch (baudrate) {
            case 9600:
            case 19200:
            case 38400:
            case 57600:
            case 115200:
                return true;
            default:
                return false;
        }
    }
};

/**
 * InterfacesData represents all UART interface configurations
 */
class InterfacesData {
public:
    InterfaceConfig uart1;
    InterfaceConfig uart2;
    
    InterfacesData() 
        : uart1(19200, 8, 1, 0), uart2(19200, 8, 1, 0) {}
    
    // Serialize to JSON
    void toJson(JsonObject& obj) const {
        uart1.toJson(obj, "uart1");
        uart2.toJson(obj, "uart2");
    }
    
    // Deserialize from JSON
    static InterfacesData fromJson(const JsonObject& obj) {
        InterfacesData data;
        
        if (!obj.isNull()) {
            if (obj.containsKey("uart1_baud")) {
                data.uart1 = InterfaceConfig(
                    obj["uart1_baud"],
                    obj["uart1_data"],
                    obj["uart1_stop"],
                    obj["uart1_parity"]
                );
            }
            
            if (obj.containsKey("uart2_baud")) {
                data.uart2 = InterfaceConfig(
                    obj["uart2_baud"],
                    obj["uart2_data"],
                    obj["uart2_stop"],
                    obj["uart2_parity"]
                );
            }
        }
        
        return data;
    }
};

#endif // INTERFACES_DATA_H
