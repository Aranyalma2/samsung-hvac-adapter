#include "StatusService.h"
#include "ModbusPollingService.h"

// Static member initialization
StatusData StatusService::currentStatus;
unsigned long StatusService::startTime = 0;

const StatusData& StatusService::getStatus() {
    // Update uptime
    unsigned long millisElapsed = millis() - startTime;
    currentStatus.uptime = millisElapsed / 1000;
    
    // Update polling metrics
    currentStatus.ongoing_requests = ModbusPollingService::getOngoingRequests();
    currentStatus.poll_delay_ms = ModbusPollingService::getPollDelayMs();
    
    return currentStatus;
}
