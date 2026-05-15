#pragma once

#include <MFRC522.h>

extern MFRC522 rfid;

void initRFID();
String readUID();
bool isAuthorizedUID(String uid);