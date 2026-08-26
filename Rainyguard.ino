#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin-Definitionen ---
#define PIN_RAIN_SENSOR   34  // Analog/Digital Input Regensensor
#define PIN_DHT           17  // DHT Data Pin
#define PIN_DHT_TYPE      DHT11 // Bei Bedarf auf DHT22 anpassen

#define PIN_GAS_MQ        23  // MQ Gassensor
#define PIN_PIR           14  // PIR Bewegungsmelder
#define PIN_BTN_RESET     16  // Manueller Taster / Quittierung

#define PIN_FAN_PWM       18  // Luefter Motor
#define PIN_FAN_DIR       19  // Luefter Enable/Richtung
#define PIN_LED_STATUS    12  // Status LED
#define PIN_BUZZER        25  // Buzzer

// --- Hardware-Objekte ---
DHT dht(PIN_DHT, PIN_DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C-Adresse 0x27 oder 0x3F

// --- Messwerte & Timing ---
float temperature = 0.0;
float humidity = 0.0;
int rainValue = 0;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL = 2000; // Alle 2 Sek. abfragen

void setup() {
  Serial.begin(115200);
  delay(500);

  // Pin-Modi konfigurieren
  pinMode(PIN_RAIN_SENSOR, INPUT);
  pinMode(PIN_GAS_MQ, INPUT);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

  pinMode(PIN_FAN_PWM, OUTPUT);
  pinMode(PIN_FAN_DIR, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Aktoren initial aus
  digitalWrite(PIN_FAN_PWM, LOW);
  digitalWrite(PIN_FAN_DIR, LOW);
  digitalWrite(PIN_LED_STATUS, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Sensoren & Display starten
  dht.begin();
  Wire.begin(21, 22); // Standard SDA=21, SCL=22 beim ESP32
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("RainyGuard ESP32");
  lcd.setCursor(0, 1);
  lcd.print("Init Sensors...");

  Serial.println("[RainyGuard] Sensor-Treiber initialisiert.");
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking Sensor-Abfrage
  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentMillis;

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    rainValue = analogRead(PIN_RAIN_SENSOR);

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("[WARN] Fehler beim Lesen des DHT-Sensors!");
      return;
    }

    // Debug-Ausgabe auf der seriellen Konsole
    Serial.printf("[Sensoren] Temp: %.1f °C | Feuchte: %.1f %% | Regen (Raw ADC): %d\n", 
                  temperature, humidity, rainValue);

    // Live-Werte auf LCD schreiben
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("T:%.1fC H:%.1f%%", temperature, humidity);
    lcd.setCursor(0, 1);
    lcd.printf("Rain ADC: %d", rainValue);
  }
}