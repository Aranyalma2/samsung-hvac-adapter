#include "ModbusPollingService.h"

// Static member initialization
Comport2* ModbusPollingService::comport = nullptr;
uint32_t ModbusPollingService::pollDelayMs = 10;
uint32_t ModbusPollingService::minPollDelayMs = 10;
uint32_t ModbusPollingService::maxPollDelayMs = 1000;
size_t ModbusPollingService::currentGroupIndex = 0;
size_t ModbusPollingService::currentRegisterIndex = 0;
bool ModbusPollingService::isPollingGroupRegisters = true;
size_t ModbusPollingService::currentSlaveIndex = 0;
bool ModbusPollingService::initialized = false;
unsigned long ModbusPollingService::lastRequestTime = 0;
size_t ModbusPollingService::ongoingRequests = 0;
