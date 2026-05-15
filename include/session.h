#pragma once

#include <Arduino.h>
#include "config.h"

// the session entry struct for storing completed session info in history
struct SessionEntry {
  String time;
  int    duration;
  int    breaks;
  bool   completed;
};


// session state variables
extern SessionEntry sessionHistory[5];
extern int historyCount;
extern int historyIndex;

extern State currentState;

extern unsigned long studyDurationMs;
extern unsigned long studyStartMs;
extern unsigned long studyElapsedMs;
extern unsigned long pauseStartMs;
extern unsigned long pauseDurationMs;
extern unsigned long totalPauseMs;

extern int rfidUsesLeft;
extern int rfidUsesTotal;

extern String enteredNum;

extern int currentQuote;
extern unsigned long lastQuoteChange;




void initSession();
void addSession(int durationMin, int breaks, bool completed);