#include "RegisterMappingService.h"

// Static member initialization
std::map<uint8_t, std::map<uint16_t, uint16_t*>> RegisterMappingService::registerMap;
bool RegisterMappingService::initialized = false;
