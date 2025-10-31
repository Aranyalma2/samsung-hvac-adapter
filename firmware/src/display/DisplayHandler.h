#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include "../config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayHandler {
public:
    DisplayHandler();
    void setup();
    void clear();
    void write(const char* status);
    void addNewLine();
    
private:
    Adafruit_SSD1306 display;
    int cursorY; // Track the current cursor Y position for new lines
};

extern DisplayHandler displayHandler;

#endif // DISPLAY_HANDLER_H