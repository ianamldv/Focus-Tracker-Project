#include "rfid_reader.h"
#include "config.h"
#include <SPI.h>
#include <Arduino.h>

MFRC522 rfid(SS_PIN, RST_PIN);

void initRFID() {
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID ready");
}

String readUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uid += ":";
  }
  uid.toUpperCase();
  return uid;
}

bool isAuthorizedUID(String uid) {
  for (int i = 0; i < numCards; i++) {
    if (uid == authorizedUIDs[i]) return true;
  }
  return false;
}