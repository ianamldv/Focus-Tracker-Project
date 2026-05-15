#pragma once

#include <Arduino.h>
#include <secrets.h>

//this file contains all the global constants and variables used across multiple files

// wifi credentials
const char* const ssid     = WIFI_SSID;
const char* const password = WIFI_PASSWORD;

// NTP server settings for timestamping sessions
const char* const ntpServer  = "pool.ntp.org";
const long  gmtOffset  = 7200; // GMT+2
const int   dstOffset  = 0;

// pin definitions
#define SS_PIN     5
#define RST_PIN    4
#define BUZZER_PIN 2
#define SERVO_PIN  15

// servo positions
#define LOCKED_POS   0
#define UNLOCKED_POS 180 // start unlocked so phone can be placed inside before locking

// OLED display settings
#define SCREEN_W  128
#define SCREEN_H   64
#define OLED_ADDR 0x3C

// Keypad settings
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

// RFID settings
const String authorizedUIDs[] = {
  RFID_KEY_1,
  RFID_KEY_2
};
const int numCards = 2;

// Motivational quotes to display during study sessions
const char* const quotes[] = {
    "Ramai concentrat!",
    "Poti face asta!",
    "Munca profunda castiga",
    "Fara distrageri",
    "Continua sa muncesti!",
    "Pas cu pas",
    "Viitorul tau iti va multumi",
    "Concentrarea e superputerea ta"
};

const int numQuotes = 8;

// session states
enum State {
  SETUP,
  STUDYING,
  PAUSE_INPUT,
  PAUSED,
  SUMMARY
};