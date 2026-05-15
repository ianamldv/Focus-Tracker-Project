#pragma once

#include <ESP32Servo.h>

// The servo object is defined here so web.cpp and main.cpp
// can both call lockBox / unlockBox without re-declaring it.
extern Servo lockServo;

void initServo();
void lockBox();
void unlockBox();