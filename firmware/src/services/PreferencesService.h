#ifndef PREFERENCES_SERVICE_H
#define PREFERENCES_SERVICE_H

#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "../models/Group.h"
#include "../models/InterfacesData.h"

/**
 * PreferencesService handles persistent storage of configuration to SPIFFS
 * - Modbus groups configuration
 * - Interface settings
 */
class PreferencesService {
private:
    static const char* MODBUS_FILE;
    static const char* INTERFACES_FILE;
    static const size_t JSON_CAPACITY;
    
    // Initialize SPIFFS if not already done
    static bool initSPIFFS() {
        if (!SPIFFS.begin(true)) {
            Serial.println("[PreferencesService] Error: Could not mount SPIFFS");
            return false;
        }
        return true;
    }
    
public:
    /**
     * Load all groups from persistent storage
     */
    static bool loadGroups(std::vector<Group>& groups) {
        if (!initSPIFFS()) return false;
        
        if (!SPIFFS.exists(MODBUS_FILE)) {
            Serial.println("[PreferencesService] Modbus config file not found, starting fresh");
            return true;  // Not an error, just starting fresh
        }
        
        File file = SPIFFS.open(MODBUS_FILE, "r");
        if (!file) {
            Serial.println("[PreferencesService] Error: Could not open modbus config file");
            return false;
        }
        
        DynamicJsonDocument doc(JSON_CAPACITY);
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        
        if (error) {
            Serial.print("[PreferencesService] JSON deserialization error: ");
            Serial.println(error.c_str());
            return false;
        }
        
        groups.clear();
        if (doc.containsKey("groups")) {
            auto groupsArray = doc["groups"].as<JsonArray>();
            for (const auto& groupObj : groupsArray) {
                groups.push_back(Group::fromJson(groupObj));
            }
        }
        
        Serial.print("[PreferencesService] Loaded ");
        Serial.print(groups.size());
        Serial.println(" groups from SPIFFS");
        return true;
    }
    
    /**
     * Save all groups to persistent storage (without register values)
     */
    static bool saveGroups(const std::vector<Group>& groups) {
        if (!initSPIFFS()) return false;
        
        DynamicJsonDocument doc(JSON_CAPACITY);
        auto groupsArray = doc.createNestedArray("groups");
        
        for (const auto& group : groups) {
            auto groupObj = groupsArray.createNestedObject();
            // Save without values - values are only in memory
            group.toJson(groupObj, false);
        }
        
        File file = SPIFFS.open(MODBUS_FILE, "w");
        if (!file) {
            Serial.println("[PreferencesService] Error: Could not open modbus config file for writing");
            return false;
        }
        
        if (serializeJson(doc, file) == 0) {
            Serial.println("[PreferencesService] Error: Could not serialize groups to JSON");
            file.close();
            return false;
        }
        
        file.close();
        
        Serial.print("[PreferencesService] Saved ");
        Serial.print(groups.size());
        Serial.println(" groups to SPIFFS (without values)");
        return true;
    }
    
    /**
     * Load interface configuration from persistent storage
     */
    static bool loadInterfaces(InterfacesData& data) {
        if (!initSPIFFS()) return false;
        
        if (!SPIFFS.exists(INTERFACES_FILE)) {
            Serial.println("[PreferencesService] Interfaces config file not found, using defaults");
            return true;  // Not an error, use defaults
        }
        
        File file = SPIFFS.open(INTERFACES_FILE, "r");
        if (!file) {
            Serial.println("[PreferencesService] Error: Could not open interfaces config file");
            return false;
        }
        
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        
        if (error) {
            Serial.print("[PreferencesService] JSON deserialization error: ");
            Serial.println(error.c_str());
            return false;
        }
        
        JsonObject obj = doc.as<JsonObject>();
        data = InterfacesData::fromJson(obj);
        
        Serial.println("[PreferencesService] Loaded interface configuration from SPIFFS");
        return true;
    }
    
    /**
     * Save interface configuration to persistent storage
     */
    static bool saveInterfaces(const InterfacesData& data) {
        if (!initSPIFFS()) return false;
        
        DynamicJsonDocument doc(256);
        JsonObject obj = doc.to<JsonObject>();
        data.toJson(obj);
        
        File file = SPIFFS.open(INTERFACES_FILE, "w");
        if (!file) {
            Serial.println("[PreferencesService] Error: Could not open interfaces config file for writing");
            return false;
        }
        
        if (serializeJson(doc, file) == 0) {
            Serial.println("[PreferencesService] Error: Could not serialize interfaces to JSON");
            file.close();
            return false;
        }
        
        file.close();
        
        Serial.println("[PreferencesService] Saved interface configuration to SPIFFS");
        return true;
    }
    
    /**
     * Get available space on SPIFFS
     */
    static void printStorageInfo() {
        if (!initSPIFFS()) return;
        
        size_t total = SPIFFS.totalBytes();
        size_t used = SPIFFS.usedBytes();
        size_t free = total - used;
        
        Serial.print("[PreferencesService] SPIFFS Total: ");
        Serial.print(total);
        Serial.print(" bytes, Used: ");
        Serial.print(used);
        Serial.print(" bytes, Free: ");
        Serial.print(free);
        Serial.println(" bytes");
    }
};

#endif // PREFERENCES_SERVICE_H
