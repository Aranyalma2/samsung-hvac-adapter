#ifndef __UTILS_H__
#define __UTILS_H__

#include <Arduino.h>

namespace utils {

// Returns the SerialConfig enumeration corresponding to the current UART settings
SerialConfig getSerialConfigEnum(const uint8_t data_bits, const uint8_t stop_bits, const uint8_t parity);

void scheduleRestart(uint32_t delayMs, String reason);

}

#endif // __UTILS_H__