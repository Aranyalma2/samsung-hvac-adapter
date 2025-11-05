#ifndef MAP_CONTROLLER_H
#define MAP_CONTROLLER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "../services/RegisterMappingService.h"
#include "../services/ModbusService.h"

/**
 * MapController handles /api/map endpoint
 * Returns the register mapping organized by groups
 */
class MapController {
public:
    static void registerRoutes(AsyncWebServer& server) {
        // GET /api/map - Get register mapping
        server.on("/api/map", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse *response = new AsyncJsonResponse();
            JsonObject root = response->getRoot().as<JsonObject>();
            
            JsonArray groupsArray = root["groups"].to<JsonArray>();
            
            // Get all groups from ModbusService
            const auto& groups = ModbusService::getGroups();
            
            for (const auto& group : groups) {
                JsonObject groupObj = groupsArray.add<JsonObject>();
                groupObj["id"] = group.id;
                groupObj["remote_address"] = group.remoteAddress;
                groupObj["name"] = group.name;
                
                JsonArray registersArray = groupObj["registers"].to<JsonArray>();
                uint16_t address = 0;
                
                // Add group-level registers
                for (const auto& reg : group.registers) {
                    JsonObject regObj = registersArray.add<JsonObject>();
                    regObj["address"] = address;
                    regObj["id"] = reg.id;
                    regObj["name"] = reg.name;
                    regObj["type"] = "group";
                    regObj["slave_id"] = 0;
                    address++;
                }
                
                // Add slave registers
                for (const auto& slave : group.slaves) {
                    for (const auto& reg : slave.registers) {
                        JsonObject regObj = registersArray.add<JsonObject>();
                        regObj["address"] = address;
                        regObj["id"] = reg.id;
                        regObj["name"] = reg.name;
                        regObj["type"] = "slave";
                        regObj["slave_id"] = slave.id;
                        address++;
                    }
                }
                
                groupObj["total_registers"] = address;
            }
            
            root["initialized"] = RegisterMappingService::isInitialized();
            root["total_groups"] = groups.size();
            
            response->setLength();
            request->send(response);
        });
    }
};

#endif // MAP_CONTROLLER_H
