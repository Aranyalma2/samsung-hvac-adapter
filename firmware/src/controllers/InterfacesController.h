#ifndef INTERFACES_CONTROLLER_H
#define INTERFACES_CONTROLLER_H

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "../services/InterfacesService.h"

/**
 * InterfacesController handles /api/interfaces endpoints
 */
class InterfacesController {
public:
    /**
     * Register routes
     */
    static void registerRoutes(AsyncWebServer& server) {
        // GET /api/interfaces - Get current interface settings
        server.on("/api/interfaces", HTTP_GET, [](AsyncWebServerRequest *request) {
            handleGetInterfaces(request);
        });
        
        // POST /api/interfaces - Update interface settings
        server.on("/api/interfaces", HTTP_POST, [](AsyncWebServerRequest *request) {
            handlePostInterfaces(request);
        }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            handlePostInterfacesBody(request, data, len, index, total);
        });
    }
    
private:
    static String postData;
    
    /**
     * GET /api/interfaces
     * Returns current UART interface settings
     */
    static void handleGetInterfaces(AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject obj = response->getRoot().as<JsonObject>();
        
        const auto& config = InterfacesService::getConfig();
        config.toJson(obj);
        
        response->setLength();
        request->send(response);
    }
    
    /**
     * POST body handler for /api/interfaces
     */
    static void handlePostInterfacesBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            postData = "";
        }
        
        for (size_t i = 0; i < len; i++) {
            postData += (char)data[i];
        }
    }
    
    /**
     * POST /api/interfaces
     * Updates UART interface settings
     */
    static void handlePostInterfaces(AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, postData);
        
        if (error) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            JsonObject obj = response->getRoot().as<JsonObject>();
            obj["error"] = "Invalid JSON";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        // Parse new configuration
        JsonObject docObj = doc.as<JsonObject>();
        InterfacesData newConfig = InterfacesData::fromJson(docObj);
        
        // Validate
        if (!newConfig.uart1.isValid() || !newConfig.uart2.isValid()) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(400);
            JsonObject obj = response->getRoot().as<JsonObject>();
            obj["error"] = "Invalid interface configuration";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        // Update configuration
        if (!InterfacesService::updateConfig(newConfig)) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            response->setCode(500);
            JsonObject obj = response->getRoot().as<JsonObject>();
            obj["error"] = "Failed to save configuration";
            response->setLength();
            request->send(response);
            postData = "";
            return;
        }
        
        // Success response
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject obj = response->getRoot().as<JsonObject>();
        obj["message"] = "Settings saved successfully, device will restart to apply changes";
        
        auto dataObj = obj.createNestedObject("data");
        newConfig.toJson(dataObj);
        
        response->setLength();
        request->send(response);

        postData = "";
        
        // Restart device to apply new settings
        utils::scheduleRestart(2000, "Applying new interface settings");
    }
};

// Static member initialization
String InterfacesController::postData;

#endif // INTERFACES_CONTROLLER_H
