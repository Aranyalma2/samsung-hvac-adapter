#include "LocalWebServer.h"
#include "../config.h"
#include <SPIFFS.h>

// Initialize static member variables
AsyncWebServer LocalWebServer::server(80);

void LocalWebServer::start() {
    // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    
    Serial.println("SPIFFS mounted successfully");
    
    // Serve static files from SPIFFS root without authentication
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    
    // Optional: Add a catch-all route for debugging
    server.onNotFound([](AsyncWebServerRequest *request){
        Serial.printf("Not found: %s\n", request->url().c_str());
        request->send(404, "text/plain", "Not found");
    });
    
    // Start the server
    server.begin();
    Serial.println("Web server started on port 80");
}
