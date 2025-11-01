#include "src/config.h"
#include "src/network/NetworkHandler.h"
#include "src/webserver/LocalWebServer.h"
#include "src/display/DisplayHandler.h"
#include "src/comport/Comport1.h"
#include "src/comport/Comport2.h"
#include "src/utils.h"
#include "src/services/StatusService.h"
#include "src/services/ModbusService.h"
#include "src/services/InterfacesService.h"
#include "src/services/PreferencesService.h"
#include "src/services/ModbusPollingService.h"
#include "src/services/RegisterMappingService.h"

Comport1 c1;
Comport2 c2;

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);

    displayHandler.setup();
    pinMode(35, INPUT);

    delay(100);
    
    Serial.println("\n\n=================================");
    Serial.println("Samsung HVAC Adapter");
    Serial.println("Firmware Version: " FIRMWARE_VERSION);
    Serial.println("=================================\n");
    
    displayHandler.clear();
    displayHandler.write("HVAC Adapter");
    displayHandler.write("v" FIRMWARE_VERSION);
    displayHandler.addNewLine();
    
    // Initialize Status Service (must be early)
    Serial.println("Initializing Status Service...");
    StatusService::init();
    
    // Initialize Interfaces Service (load from persistent storage)
    Serial.println("Initializing Interfaces Service...");
    InterfacesService::init();
    
    // Initialize Modbus Service (load from persistent storage)
    Serial.println("Initializing Modbus Service...");
    ModbusService::init();
    
    // Build register mapping for COM1
    Serial.println("Building Register Mapping...");
    RegisterMappingService::buildMapping();
    
    Serial.println("Services initialized");
    PreferencesService::printStorageInfo();
    
    // Setup network
    Serial.println("Setting up network...");
    setupNetwork();

    // Start web server with all API routes
    Serial.println("Starting web server...");
    LocalWebServer::start();

    // Setup UART interfaces
    const auto& uartCfg = InterfacesService::getConfig();
    
    Serial.println("Setting up UART2 (Comport2 - Master)...");
    c2.setup(uartCfg.uart2.baudrate, utils::getSerialConfigEnum(uartCfg.uart2.dataBits, uartCfg.uart2.stopBits, uartCfg.uart2.parity));
    
    Serial.println("Setting up UART1 (Comport1 - Slave)...");
    c1.setup(uartCfg.uart1.baudrate, utils::getSerialConfigEnum(uartCfg.uart1.dataBits, uartCfg.uart1.stopBits, uartCfg.uart1.parity), &c2);
    
    // Initialize Modbus Polling Service for COM2
    Serial.println("Initializing Modbus Polling Service...");
    ModbusPollingService::init(&c2, 100, 1000);
    
    Serial.printf("Total groups configured: %d\n", ModbusService::getGroupCount());
    
    Serial.println("=================================");
    Serial.println("Initialization complete!");
    Serial.println("=================================\n");
}

void loop() {
    // Update Modbus polling service
    ModbusPollingService::update();

    if (digitalRead(35)) {
        ESP.restart();
    }
    
    delay(1);
}
