#include "session.h"
#include <time.h>

// Global session state variables
SessionEntry sessionHistory[5];
int historyCount = 0;
int historyIndex = 0;

// Current session state
State currentState = SETUP;

unsigned long studyDurationMs = 0;
unsigned long studyStartMs    = 0;
unsigned long studyElapsedMs  = 0;
unsigned long pauseStartMs    = 0;
unsigned long pauseDurationMs = 0;
unsigned long totalPauseMs    = 0;

int rfidUsesLeft  = 3;
int rfidUsesTotal = 0;

String enteredNum = "";

int currentQuote          = 0;
unsigned long lastQuoteChange = 0;

// Function to initialize session state
void initSession() {
  studyElapsedMs  = 0;
  totalPauseMs    = 0;
  rfidUsesLeft    = 3;
  rfidUsesTotal   = 0;
  currentQuote    = 0;
  lastQuoteChange = millis();
  enteredNum      = "";
}

// Function to add a completed session to history
void addSession(int durationMin, int breaks, bool completed) {
  String ts = "unknown";
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[20];
    strftime(buf, sizeof(buf), "%d/%m %H:%M", &timeinfo);
    ts = String(buf);
  }
  sessionHistory[historyIndex] = { ts, durationMin, breaks, completed };
  historyIndex = (historyIndex + 1) % 5;
  if (historyCount < 5) historyCount++;
}