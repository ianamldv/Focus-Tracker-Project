#pragma once

#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

void initDisplay();
void showSetup(String entered);
void showStudying(unsigned long remainingMs, unsigned long totalMs);
void showPauseInput(String entered);
void showPaused(unsigned long remainingMs);
void showRFIDDisabled();
void showSummary();