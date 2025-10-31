#include "DisplayHandler.h"

#ifdef ENABLE_DISPLAY
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SDA_PIN       15
#define SCL_PIN       14

DisplayHandler::DisplayHandler() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), cursorY(0) {}

void DisplayHandler::setup() {
    Wire.setPins(SDA_PIN, SCL_PIN);
    !display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    clear();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, cursorY);
    display.print("Initializing...");
    display.display();
    cursorY += 8;  // Assuming text size 1, each line is 8 pixels high
}

void DisplayHandler::clear() {
    display.clearDisplay();
    display.setCursor(0, 0);
    cursorY = 0;
    display.display();
}

void DisplayHandler::write(const char* text) {
    display.setCursor(0, cursorY);
    display.print(text);
    display.display();
    cursorY += 8;  // Move cursor down for the next line
}

void DisplayHandler::addNewLine() {
    cursorY += 8;  // Move cursor down for a new line, without writing text
    display.display();
}
#endif
#ifndef ENABLE_DISPLAY
DisplayHandler::DisplayHandler() {}
void DisplayHandler::setup() {}
void DisplayHandler::clear() {}
void DisplayHandler::write(const char* text) {}
void DisplayHandler::addNewLine() {}
#endif

DisplayHandler displayHandler;
