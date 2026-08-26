#include <Wire.h>

void setup() {
  Serial.begin(115200);
  
  // Standard-Pins für Keyestudio ESP32 Shield mit aktiviertem Pull-Up
  pinMode(21, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);
  
  Wire.begin(4,5);
  Wire.setClock(10000); // Takt drastisch auf 10 kHz senken für maximale Stabilität
  
  Serial.println("\nI2C Scanner gestartet...");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.printf("Stabil gefunden: 0x%02X\n", address);
      nDevices++;
    }
  }

  if (nDevices == 0) {
    Serial.println("Keine stabilen I2C-Geraete.");
  }
  delay(1500);
}