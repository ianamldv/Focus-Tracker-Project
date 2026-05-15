#include "web.h"
#include "config.h"
#include "session.h"
#include "servo_lock.h"
#include "buzzer.h"
#include "display_oled.h"
#include <WiFi.h>
#include <LittleFS.h> 
#include <ArduinoJson.h>
#include <time.h>
#include <Arduino.h>

WebServer server(80);

// File handlers for static assets (HTML, CSS, JS)
static void handleRoot() {
  File f = LittleFS.open("/index.html", "r");
  if (!f) { server.send(404, "text/plain", "Not found"); return; }
  server.streamFile(f, "text/html");
  f.close();
}

static void handleCSS() {
  File f = LittleFS.open("/style.css", "r");
  if (!f) { server.send(404, "text/plain", "Not found"); return; }
  server.streamFile(f, "text/css");
  f.close();
}

static void handleJS() {
  File f = LittleFS.open("/app.js", "r");
  if (!f) { server.send(404, "text/plain", "Not found"); return; }
  server.streamFile(f, "application/javascript");
  f.close();
}

// Data handler for /data route — returns JSON with current state and session info
static void handleData() {
  JsonDocument doc;

  switch (currentState) {
    case SETUP:       doc["state"] = "SETUP";       break;
    case STUDYING:    doc["state"] = "STUDYING";    break;
    case PAUSE_INPUT: doc["state"] = "PAUSE_INPUT"; break;
    case PAUSED:      doc["state"] = "PAUSED";      break;
    case SUMMARY:     doc["state"] = "SUMMARY";     break;
  }

  doc["studyDurationMs"] = studyDurationMs;
  doc["studyElapsedMs"]  = studyElapsedMs;
  doc["totalPauseMs"]    = totalPauseMs;
  doc["pauseDurationMs"] = pauseDurationMs;
  doc["rfidUsesLeft"]    = rfidUsesLeft;
  doc["rfidUsesTotal"]   = rfidUsesTotal;

  unsigned long remaining = studyDurationMs > studyElapsedMs
                            ? studyDurationMs - studyElapsedMs : 0;
  doc["remainingMs"] = remaining;

  if (currentState == PAUSED) {
    unsigned long pauseElapsed = millis() - pauseStartMs;
    unsigned long pauseRem = pauseDurationMs > pauseElapsed
                             ? pauseDurationMs - pauseElapsed : 0;
    doc["pauseRemainingMs"] = pauseRem;
  } else {
    doc["pauseRemainingMs"] = 0;
  }

  JsonArray hist = doc["history"].to<JsonArray>();
  for (int i = 0; i < historyCount; i++) {
    int idx = (historyIndex - 1 - i + 5) % 5;
    JsonObject s = hist.add<JsonObject>();
    s["time"]      = sessionHistory[idx].time;
    s["duration"]  = sessionHistory[idx].duration;
    s["breaks"]    = sessionHistory[idx].breaks;
    s["completed"] = sessionHistory[idx].completed;
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// Command handler for /cmd route — accepts JSON with "action" and parameters
static void handleCmd() {
  String action = server.arg("action");

  if (action == "start") {
    if (currentState != SETUP && currentState != SUMMARY) {
      server.send(200, "application/json", "{\"ok\":false,\"reason\":\"session already running\"}");
      return;
    }
    int minutes = server.arg("minutes").toInt();
    if (minutes < 1 || minutes > 180) {
      server.send(200, "application/json", "{\"ok\":false,\"reason\":\"invalid duration\"}");
      return;
    }
    studyDurationMs = (unsigned long)minutes * 60000;
    initSession();
    studyStartMs = millis();
    currentState = STUDYING;
    lockBox();
    successBeep();
    Serial.println("Remote: session started " + String(minutes) + " min");
    server.send(200, "application/json", "{\"ok\":true}");

  } else if (action == "break") {
    if (currentState != STUDYING) {
      server.send(200, "application/json", "{\"ok\":false,\"reason\":\"not studying\"}");
      return;
    }
    if (rfidUsesLeft <= 0) {
      server.send(200, "application/json", "{\"ok\":false,\"reason\":\"no breaks left\"}");
      return;
    }
    int minutes = server.arg("minutes").toInt();
    if (minutes < 1 || minutes > 30) {
      server.send(200, "application/json", "{\"ok\":false,\"reason\":\"invalid break duration\"}");
      return;
    }
    rfidUsesLeft--;
    rfidUsesTotal++;
    studyDurationMs -= studyElapsedMs;
    studyElapsedMs   = 0;
    pauseDurationMs  = (unsigned long)minutes * 60000;
    pauseStartMs     = millis();
    enteredNum       = "";
    currentState     = PAUSED;
    unlockBox();
    Serial.println("Remote: break started " + String(minutes) + " min");
    server.send(200, "application/json", "{\"ok\":true}");

  } else if (action == "stop") {
    if (currentState == SETUP) {
      server.send(200, "application/json", "{\"ok\":false,\"reason\":\"no session running\"}");
      return;
    }
    addSession(studyDurationMs / 60000, rfidUsesTotal, false);
    currentState = SETUP;
    unlockBox();
    failBeep();
    enteredNum = "";
    showSetup("");
    Serial.println("Remote: session stopped");
    server.send(200, "application/json", "{\"ok\":true}");

  } else {
    server.send(200, "application/json", "{\"ok\":false,\"reason\":\"unknown action\"}");
  }
}

// initWeb — connect to WiFi, set up server routes, and start server
void initWeb() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS failed!");
    while (true);
  }
  Serial.println("LittleFS ready");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  configTime(gmtOffset, dstOffset, ntpServer);
  Serial.println("Syncing time via NTP...");
  delay(2000);

  server.on("/",          handleRoot);
  server.on("/style.css", handleCSS);
  server.on("/app.js",    handleJS);
  server.on("/data",      handleData);
  server.on("/cmd",       handleCmd);
  server.begin();
  Serial.println("Dashboard: http://" + WiFi.localIP().toString());
}

void printIP() {
  Serial.println("IP: " + WiFi.localIP().toString());
}