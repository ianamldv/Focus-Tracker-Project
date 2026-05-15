#include "display_oled.h"
#include "config.h"
#include "session.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

void initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found! Check wiring.");
    while (true);
  }
  Serial.println("OLED ready");
}

void showSetup(String entered) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.println("Focus Time :)");
  display.drawLine(0, 10, 128, 10, WHITE);
  display.setCursor(0, 14);
  display.println("How long to focus?");
  display.setCursor(0, 24);
  display.println("Type minutes + #");
  display.setCursor(0, 34);
  display.println("* = clear");
  display.drawLine(0, 44, 128, 44, WHITE);
  display.setTextSize(2);
  display.setCursor(10, 48);
  if (entered.length() > 0) {
    display.print(entered);
    display.println(" min");
  } else {
    display.println("__ min");
  }
  display.display();
}

void showStudying(unsigned long remainingMs, unsigned long totalMs) {
  display.clearDisplay();

  int remMin = remainingMs / 60000;
  int remSec = (remainingMs % 60000) / 1000;

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Focus Time :)");
  display.print(rfidUsesLeft);
  display.println("/3 breaks");
  display.drawLine(0, 10, 128, 10, WHITE);

  display.setTextSize(2);
  char buf[6];
  sprintf(buf, "%02d:%02d", remMin, remSec);
  display.setCursor(15, 14);
  display.println(buf);

  int elapsed  = totalMs - remainingMs;
  int barWidth = map(elapsed, 0, totalMs, 0, 124);
  display.drawRect(2, 34, 124, 8, WHITE);
  display.fillRect(2, 34, barWidth, 8, WHITE);

  display.setTextSize(1);
  display.setCursor(0, 46);
  display.println(quotes[currentQuote]);

  display.display();
}

void showPauseInput(String entered) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(15, 0);
  display.println("BREAK TIME");
  display.drawLine(0, 10, 128, 10, WHITE);
  display.setCursor(0, 14);
  display.println("How long? (minutes)");
  display.setCursor(0, 24);
  display.println("Type + # to confirm");
  display.drawLine(0, 36, 128, 36, WHITE);
  display.setTextSize(2);
  display.setCursor(10, 42);
  if (entered.length() > 0) {
    display.print(entered);
    display.println(" min");
  } else {
    display.println("__ min");
  }
  display.display();
}

void showPaused(unsigned long remainingMs) {
  int remMin = remainingMs / 60000;
  int remSec = (remainingMs % 60000) / 1000;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println("BREAK TIME");
  display.drawLine(0, 10, 128, 10, WHITE);
  display.setCursor(0, 14);
  display.println("Enjoy your break!");
  display.setCursor(0, 24);
  display.println("Back to focus in:");
  display.setTextSize(2);
  char buf[6];
  sprintf(buf, "%02d:%02d", remMin, remSec);
  display.setCursor(20, 38);
  display.println(buf);
  display.display();
}

void showRFIDDisabled() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 15);
  display.println("No breaks left!");
  display.setCursor(5, 30);
  display.println("Stay focused!");
  display.display();
  delay(2000);
}

void showSummary() {
  int studyMin = studyElapsedMs / 60000;
  int studySec = (studyElapsedMs % 60000) / 1000;
  int breakMin = totalPauseMs / 60000;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println("SESSION DONE!");
  display.drawLine(0, 10, 128, 10, WHITE);
  display.setCursor(0, 14);
  display.print("Studied: ");
  display.print(studyMin);
  display.print("m ");
  display.print(studySec);
  display.println("s");
  display.setCursor(0, 24);
  display.print("Breaks:  ");
  display.println(rfidUsesTotal);
  display.setCursor(0, 34);
  display.print("Break time: ");
  display.print(breakMin);
  display.println(" min");
  display.drawLine(0, 46, 128, 46, WHITE);
  display.setCursor(5, 50);
  if (rfidUsesTotal == 0)      display.println("Perfect focus!");
  else if (rfidUsesTotal == 1) display.println("Keep going!");
  else                         display.println("Try your best!");
  display.display();
}