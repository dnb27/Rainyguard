#include <Wire.h>

void setup() {
  Wire.begin(21, 22); // SDA=21, SCL=22
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nI2C Scanner laeuft...");
}

void loop() {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanne...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.printf("I2C-Geraet gefunden an Adresse 0x%02X\n", address);
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println("Keine I2C-Geraete gefunden!\n");
  else Serial.println("Scan abgeschlossen.\n");
  delay(3000);
}