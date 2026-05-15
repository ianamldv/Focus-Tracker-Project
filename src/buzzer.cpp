#include "buzzer.h"
#include "config.h"
#include <Arduino.h>

void playTone(int freq, int dur) {
  int period = 1000000 / freq;
  int cycles = (dur * 1000) / period;
  for (int i = 0; i < cycles; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(period / 2);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(period / 2);
  }
}

void successBeep() {
  playTone(1000, 100); delay(50);
  playTone(1500, 100); delay(50);
  playTone(2000, 200);
}

void failBeep() {
  playTone(300, 500);
}

void keyBeep() {
  playTone(1200, 50);
}

void disabledBeep() {
  for (int i = 0; i < 3; i++) {
    playTone(400, 100);
    delay(100);
  }
}

void celebrationBeep() {
  playTone(1000, 100); delay(50);
  playTone(1200, 100); delay(50);
  playTone(1500, 100); delay(50);
  playTone(2000, 300);
}