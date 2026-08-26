#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin-Definitionen ---
#define PIN_RAIN_SENSOR   34  // Regensensor Dach (Analog)
#define PIN_DHT           17  // DHT11/22 Data Pin
#define PIN_DHT_TYPE      DHT11

#define PIN_FAN_PWM       18  // Luefter Speed (PWM)
#define PIN_FAN_DIR       19  // Luefter Direction/Enable
#define PIN_LED_STATUS    12  // Status LED
#define PIN_BUZZER        25  // Buzzer
#define PIN_BTN_RESET     16  // Reset Taster

// I2C Pins
#define I2C_SDA           4
#define I2C_SCL           5

// --- Hardware-Objekte ---
DHT dht(PIN_DHT, PIN_DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Falls Display dunkel bleibt: 0x3F probieren

// --- FSM Phasen ---
enum SystemPhase {
  PHASE_0_NORMAL = 0,
  PHASE_1_VENTILATION = 1,
  PHASE_2_CRITICAL = 2,
  PHASE_3_EMERGENCY_RAIN = 3
};

SystemPhase currentPhase = PHASE_0_NORMAL;

float temperature = 0.0;
float humidity = 0.0;
int rainRaw = 4095;
unsigned long lastUpdate = 0;

void setFan(int speed) {
  // speed: 0 (Aus) bis 255 (Vollgas)
  digitalWrite(PIN_FAN_DIR, speed > 0 ? HIGH : LOW);
  analogWrite(PIN_FAN_PWM, speed);
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Dem ESP32 & USB-UART Zeit zum Sync geben

  Serial.println("\n=================================");
  Serial.println("[RainyGuard] Booting ESP32 Node...");
  Serial.println("=================================");

  pinMode(PIN_RAIN_SENSOR, INPUT);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

  pinMode(PIN_FAN_PWM, OUTPUT);
  pinMode(PIN_FAN_DIR, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Aktoren initial zuruecksetzen
  setFan(0);
  digitalWrite(PIN_LED_STATUS, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Sensoren & I2C Bus initialisieren
  dht.begin();
  
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(50000); // 50kHz fuer hohe Signalstabilitaet

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RainyGuard ESP32");
  lcd.setCursor(0, 1);
  lcd.print("System Ready...");

  Serial.println("[RainyGuard] Initialisierung abgeschlossen. Starte FSM Loop.\n");
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate >= 1000) {
    lastUpdate = now;

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    rainRaw = analogRead(PIN_RAIN_SENSOR);

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("[WARN] DHT-Sensor liefert fehlerhafte Werte (NaN)!");
      return;
    }

    // --- Phasen-Eskalation ---
    // Regensensor liefert bei Naesse niedrige ADC-Werte (< 2500)
    if (rainRaw < 2500 || humidity > 85.0) {
      currentPhase = PHASE_3_EMERGENCY_RAIN;
    } else if (humidity >= 75.0) {
      currentPhase = PHASE_2_CRITICAL;
    } else if (humidity >= 60.0) {
      currentPhase = PHASE_1_VENTILATION;
    } else {
      currentPhase = PHASE_0_NORMAL;
    }

    // --- Serial Monitor Log ---
    Serial.printf("[FSM] Phase: %d | Temp: %.1f C | Humidity: %.1f %% | Rain ADC: %d\n", 
                  currentPhase, temperature, humidity, rainRaw);

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
        digitalWrite(PIN_BUZZER, (now % 1000 < 100) ? HIGH : LOW); // Kurzer Beep
        lcd.setCursor(0, 0);
        lcd.printf("WARN H:%.0f%% !", humidity);
        lcd.setCursor(0, 1);
        lcd.print("P2: Max Lueftung");
        break;

      case PHASE_3_EMERGENCY_RAIN:
        setFan(0); // Sofort stoppen
        digitalWrite(PIN_LED_STATUS, HIGH);
        digitalWrite(PIN_BUZZER, (now / 150) % 2); // Alarmton
        lcd.setCursor(0, 0);
        lcd.print("!! ALARM: REGEN !!");
        lcd.setCursor(0, 1);
        lcd.print("FENSTER ZU / NOT");
        break;
    }
  }
}