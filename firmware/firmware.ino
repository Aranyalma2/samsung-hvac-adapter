#include "src/config.h"
#include "src/network/NetworkHandler.h"
#include "src/webserver/LocalWebServer.h"
#include "src/display/DisplayHandler.h"
#include "src/comport/Comport1.h"
#include "src/utils.h"

Comport1 c1;

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=================================");
    Serial.println("Samsung HVAC Adapter");
    Serial.println("Firmware Version: " FIRMWARE_VERSION);
    Serial.println("=================================\n");
    
    // Initialize display if enabled
    Serial.println("Initializing display...");
    displayHandler.setup();
    displayHandler.clear();
    displayHandler.write("HVAC Adapter");
    displayHandler.write("v" FIRMWARE_VERSION);
    displayHandler.addNewLine();
    
    setupNetwork();

    LocalWebServer::start();

    c1.setup(9600, SERIAL_8N1, 1); // Example: 9600 baud, 8 data bits, no parity, 1 stop bit, slave address 1
}

void loop() {
    
    delay(1);
}
