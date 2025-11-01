#ifndef STATUS_SERVICE_H
#define STATUS_SERVICE_H

#include <Arduino.h>
#include "../models/StatusData.h"

/**
 * StatusService tracks system status and statistics
 */
class StatusService {
private:
    static StatusData currentStatus;
    static unsigned long startTime;
    
public:
    /**
     * Initialize StatusService
     */
    static void init() {
        startTime = millis();
        currentStatus.uptime = 0;
        currentStatus.uart1_sent = 0;
        currentStatus.uart1_received = 0;
        currentStatus.uart2_sent = 0;
        currentStatus.uart2_received = 0;
        currentStatus.eth_status = "unknown";
        currentStatus.eth_ip = "0.0.0.0";
    }
    
    /**
     * Get current status
     */
    static const StatusData& getStatus();
    
    /**
     * Update polling metrics (called from StatusController)
     */
    static void updatePollingMetrics(size_t ongoingRequests, uint32_t pollDelayMs) {
        currentStatus.ongoing_requests = ongoingRequests;
        currentStatus.poll_delay_ms = pollDelayMs;
    }
    
    /**
     * Increment UART1 sent counter
     */
    static void addUart1Sent(uint32_t count = 1) {
        currentStatus.uart1_sent += count;
    }
    
    /**
     * Increment UART1 received counter
     */
    static void addUart1Received(uint32_t count = 1) {
        currentStatus.uart1_received += count;
    }
    
    /**
     * Increment UART2 sent counter
     */
    static void addUart2Sent(uint32_t count = 1) {
        currentStatus.uart2_sent += count;
    }
    
    /**
     * Increment UART2 received counter
     */
    static void addUart2Received(uint32_t count = 1) {
        currentStatus.uart2_received += count;
    }
    
    /**
     * Set Ethernet status
     */
    static void setEthernetStatus(const String& status, const String& ip = "") {
        currentStatus.eth_status = status;
        if (ip.length() > 0) {
            currentStatus.eth_ip = ip;
        }
    }
    
    /**
     * Reset all counters
     */
    static void reset() {
        startTime = millis();
        currentStatus.uptime = 0;
        currentStatus.uart1_sent = 0;
        currentStatus.uart1_received = 0;
        currentStatus.uart2_sent = 0;
        currentStatus.uart2_received = 0;
    }
};

#endif // STATUS_SERVICE_H
