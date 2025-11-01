#include "PreferencesService.h"

// Static member initialization
const char* PreferencesService::MODBUS_FILE = "/modbus_config.json";
const char* PreferencesService::INTERFACES_FILE = "/interfaces_config.json";
const size_t PreferencesService::JSON_CAPACITY = 16384;  // 16KB for modbus config
