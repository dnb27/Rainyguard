#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin-Definitionen ---
#define PIN_RAIN_SENSOR   34  // Regensensor Dach (Analog)
#define PIN_DHT           17  // DHT11/22 Data
#define PIN_DHT_TYPE      DHT11

#define PIN_FAN_PWM       18  // Luefter Speed
#define PIN_FAN_DIR       19  // Luefter Direction/Enable
#define PIN_LED_STATUS    12  // Status LED
#define PIN_BUZZER        25  // Buzzer
#define PIN_BTN_RESET     16  // Reset Taster

// --- Hardware-Objekte ---
DHT dht(PIN_DHT, PIN_DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Bei Bedarf auf 0x3F aendern

// --- FSM Phasen ---
enum SystemPhase {
  PHASE_0_NORMAL,
  PHASE_1_VENTILATION,
  PHASE_2_CRITICAL,
  PHASE_3_EMERGENCY_RAIN
};

SystemPhase currentPhase = PHASE_0_NORMAL;

float temperature = 0.0;
float humidity = 0.0;
int rainRaw = 4095;
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RAIN_SENSOR, INPUT);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

  pinMode(PIN_FAN_PWM, OUTPUT);
  pinMode(PIN_FAN_DIR, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  dht.begin();
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
}

void setFan(int speed) {
  // speed: 0 (Aus) bis 255 (Vollgas)
  digitalWrite(PIN_FAN_DIR, speed > 0 ? HIGH : LOW);
  analogWrite(PIN_FAN_PWM, speed);
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate >= 1000) {
    lastUpdate = now;

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    rainRaw = analogRead(PIN_RAIN_SENSOR);

    if (isnan(humidity) || isnan(temperature)) return;

    // --- Phasen-Eskalation ---
    // Regensensor liefert bei Nässe typischerweise niedrige ADC-Werte (< 2500)
    if (rainRaw < 2500 || humidity > 85.0) {
      currentPhase = PHASE_3_EMERGENCY_RAIN;
    } else if (humidity >= 75.0) {
      currentPhase = PHASE_2_CRITICAL;
    } else if (humidity >= 60.0) {
      currentPhase = PHASE_1_VENTILATION;
    } else {
      currentPhase = PHASE_0_NORMAL;
    }

    // --- Aktor- & Display-Steuerung nach Phase ---
    lcd.clear();
    switch (currentPhase) {
      case PHASE_0_NORMAL:
        setFan(0);
        digitalWrite(PIN_LED_STATUS, LOW);
        digitalWrite(PIN_BUZZER, LOW);
        lcd.setCursor(0, 0);
        lcd.printf("T:%.1fC H:%.0f%%", temperature, humidity);
        lcd.setCursor(0, 1);
        lcd.print("Status: Normal");
        break;

      case PHASE_1_VENTILATION:
        setFan(130); // ~50% PWM
        digitalWrite(PIN_LED_STATUS, (now / 500) % 2); // Langsames Blinken
        digitalWrite(PIN_BUZZER, LOW);
        lcd.setCursor(0, 0);
        lcd.printf("T:%.1fC H:%.0f%%", temperature, humidity);
        lcd.setCursor(0, 1);
        lcd.print("P1: Lueftung 50%");
        break;

      case PHASE_2_CRITICAL:
        setFan(255); // 100% PWM
        digitalWrite(PIN_LED_STATUS, (now / 200) % 2); // Schnelles Blinken
        // Kurzer Beep jede Sekunde
        digitalWrite(PIN_BUZZER, (now % 1000 < 100) ? HIGH : LOW);
        lcd.setCursor(0, 0);
        lcd.printf("WARN H:%.0f%% !", humidity);
        lcd.setCursor(0, 1);
        lcd.print("P2: Max Lueftung");
        break;

      case PHASE_3_EMERGENCY_RAIN:
        setFan(0); // SOFORT STOPPEN
        digitalWrite(PIN_LED_STATUS, HIGH);
        digitalWrite(PIN_BUZZER, (now / 150) % 2); // Schneller Alarmton
        lcd.setCursor(0, 0);
        lcd.print("!! ALARM: REGEN !!");
        lcd.setCursor(0, 1);
        lcd.print("FENSTER ZU / NOT");
        break;
    }
  }
}