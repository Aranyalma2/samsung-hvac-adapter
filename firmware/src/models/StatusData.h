#ifndef STATUS_DATA_H
#define STATUS_DATA_H

#include <ArduinoJson.h>

/**
 * StatusData represents current system status and statistics
 */
class StatusData {
public:
    uint32_t uptime;            // System uptime in seconds
    uint32_t uart1_sent;        // UART1 bytes sent
    uint32_t uart1_received;    // UART1 bytes received
    uint32_t uart2_sent;        // UART2 bytes sent
    uint32_t uart2_received;    // UART2 bytes received
    String eth_status;          // Ethernet status (connected/disconnected)
    String eth_ip;              // Ethernet IP address
    size_t ongoing_requests;    // Number of ongoing Modbus requests
    uint32_t poll_delay_ms;     // Current adaptive polling delay
    
    StatusData() 
        : uptime(0), uart1_sent(0), uart1_received(0), 
          uart2_sent(0), uart2_received(0),
          eth_status("unknown"), eth_ip("0.0.0.0"),
          ongoing_requests(0), poll_delay_ms(0) {}
    
    // Serialize to JSON
    void toJson(JsonObject& obj) const {
        obj["uptime"] = uptime;
        obj["uart1_sent"] = uart1_sent;
        obj["uart1_recived"] = uart1_received;  // Note: API uses "recived"
        obj["uart2_sent"] = uart2_sent;
        obj["uart2_recived"] = uart2_received;
        obj["eth_status"] = eth_status;
        obj["eth_ip"] = eth_ip;
        obj["ongoing_requests"] = ongoing_requests;
        obj["poll_delay_ms"] = poll_delay_ms;
    }
};

#endif // STATUS_DATA_H
