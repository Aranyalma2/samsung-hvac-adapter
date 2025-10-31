#include "utils.h"

namespace utils {

    SerialConfig getSerialConfigEnum(const uint8_t data_bits, const uint8_t stop_bits, const uint8_t parity) {
        switch (data_bits) {
            case 5:
                if (stop_bits == 1) {
                    switch (parity) {
                        case 0: return SERIAL_5N1;
                        case 1: return SERIAL_5E1;
                        case 2: return SERIAL_5O1;
                    }
                } else if (stop_bits == 2) {
                    switch (parity) {
                        case 0: return SERIAL_5N2;
                        case 1: return SERIAL_5E2;
                        case 2: return SERIAL_5O2;
                    }
                }
                break;
            case 6:
                if (stop_bits == 1) {
                    switch (parity) {
                        case 0: return SERIAL_6N1;
                        case 1: return SERIAL_6E1;
                        case 2: return SERIAL_6O1;
                    }
                } else if (stop_bits == 2) {
                    switch (parity) {
                        case 0: return SERIAL_6N2;
                        case 1: return SERIAL_6E2;
                        case 2: return SERIAL_6O2;
                    }
                }
                break;
            case 7:
                if (stop_bits == 1) {
                    switch (parity) {
                        case 0: return SERIAL_7N1;
                        case 1: return SERIAL_7E1;
                        case 2: return SERIAL_7O1;
                    }
                } else if (stop_bits == 2) {
                    switch (parity) {
                        case 0: return SERIAL_7N2;
                        case 1: return SERIAL_7E2;
                        case 2: return SERIAL_7O2;
                    }
                }
                break;
            case 8:
                if (stop_bits == 1) {
                    switch (parity) {
                        case 0: return SERIAL_8N1;
                        case 1: return SERIAL_8E1;
                        case 2: return SERIAL_8O1;
                    }
                } else if (stop_bits == 2) {
                    switch (parity) {
                        case 0: return SERIAL_8N2;
                        case 1: return SERIAL_8E2;
                        case 2: return SERIAL_8O2;
                    }
                }
                break;
        }
    }

    void scheduleRestart(uint32_t delayMs, String reason = "") {
        // Create timer with lambda callback
        TimerHandle_t restartTimer = xTimerCreate(
            "RestartTimer",
            pdMS_TO_TICKS(delayMs),
            pdFALSE,
            0,
            [](TimerHandle_t xTimer) {
            // Get reason from timer ID (stored as pointer)
            String* reasonPtr = static_cast<String*>(pvTimerGetTimerID(xTimer));
            
            if (reasonPtr && reasonPtr->length() > 0) {
                Serial.println("Restart reason: " + *reasonPtr);
                delete reasonPtr; // Clean up allocated memory
            }
            
            Serial.println("Restarting ESP32...");
            ESP.restart();
            }
        );
        
        if (restartTimer != NULL) {
            // Store reason in timer ID if provided
            if (reason.length() > 0) {
            String* reasonPtr = new String(reason);
            vTimerSetTimerID(restartTimer, reasonPtr);
            }
            
            xTimerStart(restartTimer, 0);
        }
    }
} // namespace utils