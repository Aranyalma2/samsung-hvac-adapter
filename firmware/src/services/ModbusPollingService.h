#ifndef MODBUS_POLLING_SERVICE_H
#define MODBUS_POLLING_SERVICE_H

#include <Arduino.h>
#include "../comport/Comport2.h"
#include "ModbusService.h"
#include "StatusService.h"

/**
 * ModbusPollingService manages periodic polling of Modbus registers
 * defined in the group configuration using COM2
 * Simple sequential polling with delay between requests
 */
class ModbusPollingService {
private:
    static Comport2* comport;
    static uint32_t pollDelayMs;              // Delay between individual requests (adaptive)
    static uint32_t minPollDelayMs;           // Minimum delay
    static uint32_t maxPollDelayMs;           // Maximum delay
    
    static size_t currentGroupIndex;
    static size_t currentRegisterIndex;
    static bool isPollingGroupRegisters;
    static size_t currentSlaveIndex;
    
    static bool initialized;
    static unsigned long lastRequestTime;     // Time when last request was sent
    static size_t ongoingRequests;            // Number of requests sent but not yet responded
    
public:
    /**
     * Initialize the polling service with COM2
     */
    static void init(Comport2* com2, uint32_t delayMs = 10, uint32_t maxDelay = 1000) {
        if (initialized) return;
        
        comport = com2;
        minPollDelayMs = delayMs;
        maxPollDelayMs = maxDelay;
        pollDelayMs = delayMs;
        currentGroupIndex = 0;
        currentRegisterIndex = 0;
        currentSlaveIndex = 0;
        isPollingGroupRegisters = true;
        initialized = true;
        lastRequestTime = 0;
        ongoingRequests = 0;
        
        Serial.println("ModbusPollingService initialized");
        Serial.printf("Delay range: %d-%d ms\n", minPollDelayMs, maxPollDelayMs);
    }
    
    /**
     * Called when a request is sent - increments ongoing counter
     */
    static void onRequestSent() {
        ongoingRequests++;
        adjustPollDelay();
    }
    
    /**
     * Called when a response is received - decrements ongoing counter
     */
    static void onResponseReceived() {
        if (ongoingRequests > 0) {
            ongoingRequests--;
            adjustPollDelay();
        }
    }
    
    /**
     * Get current number of ongoing requests
     */
    static size_t getOngoingRequests() {
        return ongoingRequests;
    }
    
    /**
     * Get current poll delay
     */
    static uint32_t getPollDelayMs() {
        return pollDelayMs;
    }
    
    /**
     * Main polling loop - call this frequently from main loop()
     */
    static void update() {
        if (!initialized || !comport) return;
        
        unsigned long currentTime = millis();
        
        // Check if enough time has passed since last request
        if (currentTime - lastRequestTime < pollDelayMs) {
            return;  // Not time for next request yet
        }
        
        // Get all groups from ModbusService
        const auto& groups = ModbusService::getGroups();
        
        if (groups.empty()) {
            // No groups configured
            return;
        }
        
        // Send next request
        sendNextRequest(groups, currentTime);
    }
    
private:
    /**
     * Send next single request
     */
    static void sendNextRequest(const std::vector<Group>& groups, unsigned long currentTime) {
        bool requestSent = false;
        
        // Try to send one request
        while (!requestSent) {
            // Check if we've gone through all groups
            if (currentGroupIndex >= groups.size()) {
                currentGroupIndex = 0;
                currentRegisterIndex = 0;
                currentSlaveIndex = 0;
                isPollingGroupRegisters = true;
                Serial.println("[Poll] Completed full cycle, restarting from beginning");
                return;
            }
            
            const Group& group = groups[currentGroupIndex];
            
            // Poll group-level registers first
            if (isPollingGroupRegisters) {
                if (currentRegisterIndex < group.registers.size()) {
                    const Register& reg = group.registers[currentRegisterIndex];
                    
                    // Create token: encode group ID, register type (0 = group), and register ID
                    uint32_t token = makeToken(group.id, 0, reg.id);
                    
                    // Send holding register read request (function code 0x03)
                    bool success = comport->addRequest(
                        token,
                        group.id,                    // Slave address = group ID
                        READ_HOLD_REGISTER,          // Function code 0x03
                        reg.id,                      // Register address
                        1                            // Read 1 register
                    );
                    
                    if (success) {
                        lastRequestTime = currentTime;
                        requestSent = true;
                        onRequestSent();
                    }
                    
                    currentRegisterIndex++;
                } else {
                    // Done with group registers, move to slaves
                    isPollingGroupRegisters = false;
                    currentRegisterIndex = 0;
                    currentSlaveIndex = 0;
                }
            }
            // Poll slave registers
            else {
                if (currentSlaveIndex < group.slaves.size()) {
                    const Slave& slave = group.slaves[currentSlaveIndex];
                    
                    if (currentRegisterIndex < slave.registers.size()) {
                        const Register& reg = slave.registers[currentRegisterIndex];
                        
                        // Create token: encode group ID, slave ID, and register ID
                        uint32_t token = makeToken(group.id, slave.id, reg.id);
                        
                        // Send holding register read request
                        bool success = comport->addRequest(
                            token,
                            group.id,                // Slave address = group ID
                            READ_HOLD_REGISTER,      // Function code 0x03
                            reg.id,                  // Register address
                            1                        // Read 1 register
                        );
                        
                        if (success) {
                            lastRequestTime = currentTime;
                            requestSent = true;
                            onRequestSent();
                        }
                        
                        currentRegisterIndex++;
                    } else {
                        // Done with current slave's registers, move to next slave
                        currentSlaveIndex++;
                        currentRegisterIndex = 0;
                    }
                } else {
                    // Done with all slaves, move to next group
                    currentGroupIndex++;
                    currentRegisterIndex = 0;
                    currentSlaveIndex = 0;
                    isPollingGroupRegisters = true;
                }
            }
        }
    }
    
    /**
     * Adjust poll delay based on ongoing requests
     * More pending requests = slower polling
     * Fewer pending requests = faster polling
     */
    static void adjustPollDelay() {
        // Adaptive algorithm:
        // 0-5 pending: min delay (fast)
        // 6-10 pending: gradually increase
        // 11-20 pending: medium delay
        // 21+ pending: max delay (slow down)
        
        if (ongoingRequests <= 5) {
            pollDelayMs = minPollDelayMs;
        } else if (ongoingRequests <= 10) {
            // Linear increase from min to middle
            uint32_t range = maxPollDelayMs - minPollDelayMs;
            pollDelayMs = minPollDelayMs + (range * (ongoingRequests - 5) / 10);
        } else if (ongoingRequests <= 20) {
            // Middle range
            pollDelayMs = (minPollDelayMs + maxPollDelayMs) / 2;
        } else {
            // High queue depth - max delay
            pollDelayMs = maxPollDelayMs;
        }
        
        // Debug output on significant changes
        static size_t lastLoggedRequests = 0;
        if (ongoingRequests / 5 != lastLoggedRequests / 5) {  // Log every 5 request threshold
            Serial.printf("[Poll] Ongoing: %d, Delay: %d ms\n", ongoingRequests, pollDelayMs);
            lastLoggedRequests = ongoingRequests;
        }
    }
    
    /**
     * Create a token to identify the request
     * Format: [group_id:8][slave_id:8][register_id:16]
     */
    static uint32_t makeToken(uint8_t groupId, uint8_t slaveId, uint16_t registerId) {
        return ((uint32_t)groupId << 24) | ((uint32_t)slaveId << 16) | registerId;
    }
    
    /**
     * Decode token into group ID, slave ID, and register ID
     */
    static void decodeToken(uint32_t token, uint8_t& groupId, uint8_t& slaveId, uint16_t& registerId) {
        groupId = (token >> 24) & 0xFF;
        slaveId = (token >> 16) & 0xFF;
        registerId = token & 0xFFFF;
    }
};

#endif // MODBUS_POLLING_SERVICE_H
