<p align="center">
  <img src="assets/logo.png" width="450" alt="RainyGuard — Smart Home Defense">
</p>

<p align="center">
  <a href="https://github.com/Ri4ards2006/Rainyguard"><img src="https://img.shields.io/badge/Platform-ESP32-blue?style=flat-square&logo=espressif" alt="Platform"></a>
  <a href="https://github.com/Ri4ards2006/Rainyguard"><img src="https://img.shields.io/badge/Language-C++-00599C?style=flat-square&logo=c%2B%2B" alt="Language"></a>
  <a href="https://github.com/Ri4ards2006/Rainyguard/releases"><img src="https://img.shields.io/github/v/release/Ri4ards2006/Rainyguard?color=2ea043&style=flat-square" alt="Latest release"></a>
</p>

<p align="center">
  <a href="#see-it-running"><b>Demo</b></a> ·
  <a href="#the-architecture"><b>Architecture</b></a> ·
  <a href="#get-started"><b>Quick Start</b></a>
</p>

**Autonomous environmental defense, zero cloud dependencies.** RainyGuard is an automated smart home defense station designed for a miniature house. It continuously monitors the indoor climate and protects the structure from high humidity and sudden rain through a strict, multi-stage escalation process. 

> **RainyGuard is a resilient embedded system you can deploy today.** Its primary goal is to shift smart home logic from delayed, cloud-dependent APIs back to local, instant hardware execution. By utilizing a strict protocol, it ensures your environment is ventilated when needed and locked down during critical weather events.

RainyGuard deliberately avoids complex networking overhead. There is **no SLA on API responses, and a hard guarantee on local execution**: the moment the roof-mounted analog rain sensor detects a drop, or the DHT11 crosses a humidity threshold, the system instantly transitions, adjusting DC fans, servo-driven windows, and alarms in milliseconds. 

```bash
$ Serial Monitor (115200 baud)
  =================================
  [RainyGuard] Booting ESP32 Node...
  =================================
  ✓ I2C Bus active @ 50kHz
  ✓ Sensors initialized
  [FSM] Phase: 0 | Temp: 22.4 C | Humidity: 45.0 % | Rain ADC: 4095
  [FSM] Phase: 3 | Temp: 22.4 C | Humidity: 45.0 % | Rain ADC: 1200
  !! ALARM TRIGGERED: EMERGENCY RAIN LOCKDOWN !!
 ```
## See it running

<p align="center">
  <img src="assets/setup.png" width="900" alt="RainyGuard physical setup — sensors and display">
</p>
<p align="center"><em>The physical deployment: An ESP32 routing logic to a 16x2 I2C LCD, DHT11/22, and the roof-mounted analog rain sensor[cite: 1]. Fabricating custom mounts for the hardware array ensures the sensors remain properly positioned on the miniature frame. The UI provides instant <strong>Phase Status</strong> and environmental metrics[cite: 1].</em></p>

<p align="center">
  <img src="assets/actuators.png" width="900" alt="RainyGuard actuators in action">
</p>
<p align="center"><em>The <strong>Actuator Array</strong>: PWM-controlled DC fan for room ventilation[cite: 1] scaling with humidity, automated servo motor window control[cite: 1], and strict acoustic/visual alerts (LED & Buzzer)[cite: 1] tied to Phase 2 (Critical) and Phase 3 (Emergency).</em></p>

## The Core Architecture

With RainyGuard, hardware defense is not limited by Wi-Fi drops or server outages. It removes external dependencies by aggressively optimizing a functional, local state machine pipeline written in C++.

* **Deterministic FSM:** The system operates strictly within 4 defined escalation phases (Normal, Ventilation, Critical, Emergency)[cite: 1].
* **Non-blocking I/O:** All sensor polling and actuator updates use `millis()`-based delta timing, preventing `delay()` bottlenecks and ensuring the system remains responsive.
* **Signal Stability:** The I2C bus is deliberately downclocked (50kHz) and pulled up to ensure maximum LCD stability in environments with electrical noise.
* **Isolated Utility Tools:** System diagnostics (like I2C scanning) are separated into a dedicated `tools/` directory, keeping the primary `Rainyguard.ino` compilation clean and focused.

## The Escalation Protocol

RainyGuard evaluates environmental data across a strict hierarchy. Limited logic changes output, never the core semantics.

| Phase | Thresholds (Example) | DC Fan[cite: 1] | Alert Systems[cite: 1] | Servo Window[cite: 1] |
|---|---|---|---|---|
| **0: Normal** | Hum < 60% & Dry | OFF | Disabled | Open |
| **1: Ventilation** | Hum 60% - 74% | 50% PWM | Slow LED pulse | Open |
| **2: Critical** | Hum ≥ 75% | 100% PWM | Fast LED + Beep | Open |
| **3: Emergency** | **Rain active** | **HALT (0%)** | Continuous Alarm | **LOCKED (Closed)** |

*Note: Phase 3 overrides all other phases. If rain is detected, ventilation is immediately halted to prevent pulling moisture inside, and the servo automatically closes the window[cite: 1].*

## Get started

You need the microcontroller and the component array[cite: 1]. The engine is a single `.ino` file designed for the ESP32.

### 1. Hardware Requirements
* ESP32 Microcontroller[cite: 1]
* DHT11 or DHT22 Temperature & Humidity Sensor[cite: 1]
* Analog Rain Sensor (Roof mount)[cite: 1]
* Servo Motor (Automated Window/Flap)[cite: 1]
* DC Fan Motor (Room Ventilation)[cite: 1]
* 16x2 I2C LCD Display[cite: 1]
* Status LED & Active Buzzer[cite: 1]

### 2. Software Setup
1. Clone the repository:
   ```bash
   git clone [https://github.com/Ri4ards2006/Rainyguard.git](https://github.com/Ri4ards2006/Rainyguard.git)
   ```
