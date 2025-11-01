#ifndef STATUS_CONTROLLER_H
#define STATUS_CONTROLLER_H

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "../services/StatusService.h"

/**
 * StatusController handles /api/status endpoints
 */
class StatusController {
public:
    /**
     * Register routes
     */
    static void registerRoutes(AsyncWebServer& server) {
        // GET /api/status - Get current system status
        server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
            handleGetStatus(request);
        });
    }
    
private:
    /**
     * GET /api/status
     * Returns current system status and statistics
     */
    static void handleGetStatus(AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        
        const auto& status = StatusService::getStatus();
        JsonObject obj = response->getRoot().as<JsonObject>();
        status.toJson(obj);

        response->setLength();
        request->send(response);
    }
};

#endif // STATUS_CONTROLLER_H
