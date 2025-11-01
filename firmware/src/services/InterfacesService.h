#ifndef INTERFACES_SERVICE_H
#define INTERFACES_SERVICE_H

#include "../models/InterfacesData.h"
#include "PreferencesService.h"

/**
 * InterfacesService manages UART interface configurations
 */
class InterfacesService {
private:
    static InterfacesData config;
    static bool initialized;
    
public:
    /**
     * Initialize InterfacesService - load configuration from persistent storage
     */
    static bool init() {
        if (initialized) return true;
        
        if (!PreferencesService::loadInterfaces(config)) {
            Serial.println("[InterfacesService] Warning: Could not load interfaces, using defaults");
            config = InterfacesData();
        }
        
        initialized = true;
        return true;
    }
    
    /**
     * Get current configuration
     */
    static const InterfacesData& getConfig() {
        return config;
    }
    
    /**
     * Get UART1 configuration
     */
    static const InterfaceConfig& getUart1() {
        return config.uart1;
    }
    
    /**
     * Get UART2 configuration
     */
    static const InterfaceConfig& getUart2() {
        return config.uart2;
    }
    
    /**
     * Update configuration
     */
    static bool updateConfig(const InterfacesData& newConfig) {
        // Validate both configs
        if (!newConfig.uart1.isValid()) {
            Serial.println("[InterfacesService] Invalid UART1 configuration");
            return false;
        }
        if (!newConfig.uart2.isValid()) {
            Serial.println("[InterfacesService] Invalid UART2 configuration");
            return false;
        }
        
        config = newConfig;
        
        if (!PreferencesService::saveInterfaces(config)) {
            Serial.println("[InterfacesService] Failed to save configuration");
            return false;
        }
        
        Serial.println("[InterfacesService] Configuration updated and saved");
        return true;
    }
    
    /**
     * Update UART1 configuration
     */
    static bool updateUart1(uint32_t baud, uint8_t data, uint8_t stop, uint8_t parity) {
        InterfaceConfig newUart1(baud, data, stop, parity);
        
        if (!newUart1.isValid()) {
            Serial.println("[InterfacesService] Invalid UART1 parameters");
            return false;
        }
        
        config.uart1 = newUart1;
        return PreferencesService::saveInterfaces(config);
    }
    
    /**
     * Update UART2 configuration
     */
    static bool updateUart2(uint32_t baud, uint8_t data, uint8_t stop, uint8_t parity) {
        InterfaceConfig newUart2(baud, data, stop, parity);
        
        if (!newUart2.isValid()) {
            Serial.println("[InterfacesService] Invalid UART2 parameters");
            return false;
        }
        
        config.uart2 = newUart2;
        return PreferencesService::saveInterfaces(config);
    }
    
    /**
     * Validate configuration
     */
    static bool isValid() {
        return config.uart1.isValid() && config.uart2.isValid();
    }
};

#endif // INTERFACES_SERVICE_H
