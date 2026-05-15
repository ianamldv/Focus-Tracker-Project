#include "servo_lock.h"
#include "config.h"
#include <Arduino.h>

Servo lockServo;

void initServo() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);
}

void lockBox() {
  lockServo.write(LOCKED_POS);
  Serial.println("Servo: LOCKED (0 degrees)");
}

void unlockBox() {
  lockServo.write(UNLOCKED_POS);
  Serial.println("Servo: UNLOCKED (180 degrees)");
}