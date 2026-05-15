#include <Arduino.h>
#include <Keypad.h>
#include <WiFi.h>

#include "config.h"
#include "session.h"
#include "buzzer.h"
#include "servo_lock.h"
#include "rfid_reader.h"
#include "display_oled.h"
#include "web.h"

// Keypad setup
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[KEYPAD_ROWS] = {13, 12, 14, 27};
byte colPins[KEYPAD_COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);



void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);

  initServo();
  unlockBox(); // Start unlocked (at 180 degrees) so phone can be placed inside 

  initRFID();
  initDisplay();

  // Show "connecting" message while Wi-Fi comes up
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println("Connecting to WiFi...");
  display.display();

  initWeb(); 

  // Show IP on OLED for 3 seconds
  String ip = WiFi.localIP().toString();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 5);  display.println("WiFi Connected!");
  display.drawLine(0, 17, 128, 17, WHITE);
  display.setCursor(5, 25); display.println("Dashboard at:");
  display.setCursor(5, 40); display.println(ip);
  display.display();
  delay(3000);

  showSetup("");
}



void loop() {
  server.handleClient();

// state machine for handling keypad input and RFID during different session states 
  if (currentState == SETUP) {
    char key = keypad.getKey();
    if (!key) return;

    keyBeep();

    if (key == '*') {
      enteredNum = "";
      showSetup("");

    } else if (key == '#') {
      if (enteredNum.length() > 0) {
        int minutes = enteredNum.toInt();
        if (minutes > 0 && minutes <= 180) {
          studyDurationMs = (unsigned long)minutes * 60000;
          initSession();
          studyStartMs = millis();
          currentState = STUDYING;
          lockBox();
          successBeep();
          Serial.println("Session started: " + String(minutes) + " min");
        } else {
          failBeep();
          showSetup("1-180 only");
          delay(1500);
          showSetup("");
          enteredNum = "";
        }
      }

    } else if (key >= '0' && key <= '9') {
      if (enteredNum.length() < 3) {
        enteredNum += key;
        showSetup(enteredNum);
      }
    }
    return;
  }

// pause input state — after RFID break or manual pause, before break starts
  if (currentState == PAUSE_INPUT) {
    char key = keypad.getKey();
    if (!key) return;

    keyBeep();

    if (key == '*') {
      enteredNum   = "";
      currentState = STUDYING;
      studyStartMs = millis();
      lockBox();
      showStudying(studyDurationMs - studyElapsedMs, studyDurationMs);

    } else if (key == '#') {
      if (enteredNum.length() > 0) {
        int minutes = enteredNum.toInt();
        if (minutes > 0 && minutes <= 30) {
          pauseDurationMs = (unsigned long)minutes * 60000;
          pauseStartMs    = millis();
          enteredNum      = "";
          currentState    = PAUSED;
          unlockBox();
          Serial.println("Break started: " + String(minutes) + " min");
        } else {
          failBeep();
          showPauseInput("1-30 only");
          delay(1500);
          showPauseInput("");
          enteredNum = "";
        }
      }

    } else if (key >= '0' && key <= '9') {
      if (enteredNum.length() < 2) {
        enteredNum += key;
        showPauseInput(enteredNum);
      }
    }
    return;
  }

// paused state — countdown to break end
  if (currentState == PAUSED) {
    unsigned long pauseElapsed   = millis() - pauseStartMs;
    unsigned long pauseRemaining = pauseDurationMs > pauseElapsed
                                   ? pauseDurationMs - pauseElapsed : 0;

    showPaused(pauseRemaining);

    if (pauseRemaining == 0) {
      totalPauseMs += pauseDurationMs;
      studyStartMs  = millis();
      currentState  = STUDYING;
      lockBox();
      successBeep();
      Serial.println("Break over — back to studying");
    }
    return;
  }

// studying state — show elapsed time, handle RFID and manual pause
  if (currentState == STUDYING) {
    studyElapsedMs = millis() - studyStartMs;

    unsigned long remaining = studyDurationMs > studyElapsedMs
                              ? studyDurationMs - studyElapsedMs : 0;

    // Rotate quote every 30 seconds
    if (millis() - lastQuoteChange > 30000) {
      currentQuote    = (currentQuote + 1) % numQuotes;
      lastQuoteChange = millis();
    }

    showStudying(remaining, studyDurationMs);

    // Session complete
    if (remaining == 0) {
      studyElapsedMs = studyDurationMs;
      currentState   = SUMMARY;
      addSession(studyDurationMs / 60000, rfidUsesTotal, true);
      unlockBox();
      celebrationBeep();
      showSummary();
      Serial.println("Session complete!");
      return;
    }

    // RFID check
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String uid = readUID();
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      if (isAuthorizedUID(uid)) {
        if (rfidUsesLeft > 0) {
          rfidUsesLeft--;
          rfidUsesTotal++;
          studyDurationMs -= studyElapsedMs;
          studyElapsedMs   = 0;
          enteredNum       = "";
          currentState     = PAUSE_INPUT;
          showPauseInput("");
          Serial.println("RFID break! Remaining: " + String(rfidUsesLeft));
        } else {
          disabledBeep();
          showRFIDDisabled();
          showStudying(remaining, studyDurationMs);
        }
      }
    }

    // Hold * for 3 seconds to cancel
    char key = keypad.getKey();
    if (key == '*') {
      unsigned long pressStart = millis();
      Serial.println("Hold * to cancel...");
      while (millis() - pressStart < 3000) {
        if (!keypad.getKey()) break;
      }
      if (millis() - pressStart >= 3000) {
        addSession(studyDurationMs / 60000, rfidUsesTotal, false);
        currentState = SETUP;
        unlockBox();
        failBeep();
        enteredNum = "";
        showSetup("");
        Serial.println("Session cancelled.");
      }
    }
    return;
  }

// the summary 
  if (currentState == SUMMARY) {
    showSummary();

    char key = keypad.getKey();
    if (key == '*') {
      currentState = SETUP;
      enteredNum   = "";
      showSetup("");
    }
  }
}