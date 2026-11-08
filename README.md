<div align="center">

# SMART BOOM

### ESP32 Remote Cracker Ignition System · Diwali Edition

Light a firecracker from a safe distance using nothing but your phone.
An offline Wi-Fi hotspot, a cinematic web dashboard, an animated OLED, and a multi-step safety interlock, all on a single ESP32.

![Platform](https://img.shields.io/badge/Platform-ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![Mode](https://img.shields.io/badge/Mode-Offline%20Hotspot-2dff8f?style=for-the-badge)
![Display](https://img.shields.io/badge/OLED-SSD1306-blue?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

[YouTube](https://www.youtube.com/@techtadka360official) · [Instagram](https://www.instagram.com/techtadka360official) · [Facebook](https://www.facebook.com/share/1EkKAJNLdB/) · [GitHub](https://github.com/Mr-soumik)

</div>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [How It Works](#how-it-works)
- [Safety System](#safety-system)
- [Components Required](#components-required)
- [Pin Connections](#pin-connections)
- [Power Architecture](#power-architecture)
- [Wiring Diagram](#wiring-diagram)
- [Software Setup](#software-setup)
- [Using the Dashboard](#using-the-dashboard)
- [OLED Screens](#oled-screens)
- [HTTP API](#http-api)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [Roadmap](#roadmap)
- [Safety Disclaimer](#safety-disclaimer)
- [Connect With Me](#connect-with-me)
- [License](#license)

---

## Overview

**Smart Boom** is a DIY maker project that turns an ESP32 into a remote igniter for consumer firecrackers. The ESP32 creates its own Wi-Fi network, so it needs no router and no internet. Anyone who joins is redirected straight to a custom dashboard where the cracker can be armed, counted down, and fired.

The firecracker is connected to the ESP32 through a long two-core wire. When you fire, a relay briefly passes current from a separate battery pack through a short piece of nichrome wire. The wire heats up and lights the fuse, while you stand far away.

Built by **TechTadka360**.

---

## Features

| Feature | Description |
|---|---|
| Offline hotspot | ESP32 runs as its own Wi-Fi access point, no router or internet needed |
| Captive portal | The dashboard opens automatically after connecting |
| Pro web dashboard | Dark neon UI with live fireworks, progress ring and state-based colours |
| Slide to arm | Prevents accidental activation |
| Hold to fire | The fire button must be held for 1.2 seconds |
| Selectable countdown | 5 s, 10 s or 15 s |
| Instant abort | Cuts the firing relay immediately at any stage |
| Animated OLED | Boot intro, standby, armed, countdown ring, explosion and finish screens |
| LED and buzzer feedback | Green LED patterns and countdown beeps that get faster near zero |
| Auto cut-off | The relay closes for only 3 seconds and then opens by itself |
| Blast counter | Counts blasts on both the dashboard and the OLED |
| Single-file firmware | One `.ino` file, no extra web files or SPIFFS upload |

---

## How It Works

```
 Phone / Laptop
      │  Wi-Fi (SmartBoom_TT360)
      ▼
 ┌──────────────────────────────┐
 │            ESP32             │
 │  Async web server + DNS      │
 │  State machine               │
 │  OLED animation engine       │
 └──────┬──────────┬────────────┘
        │          │
        ▼          ▼
     Relay    LED / Buzzer / OLED
        │
        ▼
 Battery ─ Switch ─ Relay ─ Long wire ─ Nichrome ─ Cracker fuse
```

**State machine**

```
SAFE ──(slide to arm)──► ARMED ──(hold fire)──► COUNTDOWN ──► FIRING (3 s) ──► DONE ──► SAFE
  ▲                        │                       │              │
  └──── DISARM ────────────┘                       └── ABORT ─────┘
```

Only the `FIRING` state can close the relay. Every other state keeps it open.

---

## Safety System

Safety is the most important part of this project. The firmware includes several layers:

1. **Safe on boot.** The relay pin is set to the OFF level before anything else runs.
2. **Two-step arming.** The system has to be armed first, and the fire command is only accepted while armed.
3. **Hold-to-fire.** The dashboard needs a deliberate 1.2 second hold.
4. **Countdown with warnings.** Beeps every second, faster in the last 3 seconds, plus the LED and OLED countdown.
5. **Abort at any time.** Abort works during the countdown and even during ignition.
6. **Time-limited ignition.** The relay stays closed for 3 seconds only.
7. **Isolated firing circuit.** The firing battery is completely separate from the ESP32 power.
8. **Physical master switch.** A slide switch in the firing battery line keeps the circuit dead until you flip it.

---

## Components Required

| # | Component | Qty | Notes |
|---|---|---|---|
| 1 | ESP32 Dev Module (30 or 38 pin) | 1 | Main controller |
| 2 | Single-channel 5V relay module | 1 | Switches the firing current |
| 3 | 18650 Li-ion cells | 2 | Wired in series (7.4V) for firing; use a holder with protection |
| 4 | Nichrome wire | ~5 cm | Total resistance about 1.5 to 3 ohm |
| 5 | 0.96" OLED display (SSD1306, I2C) | 1 | Address 0x3C |
| 6 | Green LED | 1 | Status indicator |
| 7 | 220 ohm resistor | 1 | For the LED |
| 8 | Active buzzer (5V) | 1 | Active type, not passive |
| 9 | Buck converter (LM2596 or MP1584) | 1 | Set to exactly 5.0V for the logic supply |
| 10 | Battery or power bank for the ESP32 | 1 | Logic supply, separate from the firing battery |
| 11 | Slide switches | 2 | One for logic power, one master switch for firing |
| 12 | Thick 2-core copper wire | 5 to 10 m | 1 sq mm or more, for the long firing line |
| 13 | Jumper wires and breadboard or perfboard | - | Assembly |

---

## Pin Connections

All signal pins are on the **same side** of the ESP32 board.

| ESP32 Pin | Connected To | Purpose |
|---|---|---|
| GPIO18 | Relay IN | Ignition relay control |
| GPIO19 | Green LED (+) via 220 ohm | Status LED |
| GPIO23 | Buzzer (+) | Countdown and alert sound |
| GPIO21 | OLED SDA | I2C data |
| GPIO22 | OLED SCL | I2C clock |
| 3V3 | OLED VCC | Display power |
| VIN (5V) | Relay VCC | Relay module power |
| GND | OLED GND, Relay GND, LED (-), Buzzer (-) | Common ground |

GPIO 2, 4, 5 and 15 are deliberately unused. GPIO 2, 5 and 15 are boot-strapping pins that can briefly glitch at power-up, which is unsafe for a relay.

---

## Power Architecture

The project uses **two separate power systems**.

**1. Logic supply (ESP32, relay coil, OLED, LED, buzzer)**

```
Battery + ─► Switch ─► Buck IN+          Buck OUT+ ─► ESP32 VIN
Battery - ───────────► Buck IN-          Buck OUT+ ─► Relay VCC
                                         Buck OUT- ─► ESP32 GND and Relay GND
```

Set the buck converter to exactly **5.0V** with a multimeter before connecting the ESP32 and relay. Do not feed 7V directly into VIN or the relay VCC (see [Troubleshooting](#troubleshooting)).

**2. Firing supply (high current)**

```
2S Li-ion (+) ─► Master switch ─► Relay COM
Relay NO ─► Long 2-core wire ─► Nichrome wire ─► Long wire return ─► 2S Li-ion (-)
```

**Do not connect the firing battery negative to the ESP32 GND.** Keeping the high-current loop isolated protects the ESP32 from current spikes and resets.

---

## Wiring Diagram

```
                           ┌───────────────┐
        Buck 5V OUT+ ─────►│ VIN       3V3 │──► OLED VCC
        Buck 5V OUT- ─────►│ GND       GND │──► OLED GND, LED(-), Buzzer(-)
                           │               │
        Relay IN ◄─────────│ GPIO18        │
        LED (+) via 220R ◄─│ GPIO19        │
        Buzzer (+) ◄───────│ GPIO23        │
        OLED SDA ◄─────────│ GPIO21        │
        OLED SCL ◄─────────│ GPIO22        │
                           └───────────────┘

   Firing loop (isolated from ESP32):

   [2S Li-ion +] ──► [Master Switch] ──► [Relay COM]
                                          [Relay NO] ──► Long wire ──► [Nichrome] ──► Long wire ──► [2S Li-ion -]
```

Add your photographed or drawn circuit diagram here:

```
docs/circuit_diagram.jpg
```

---

## Software Setup

### 1. Install Arduino IDE and the ESP32 board package

Add this to **Preferences → Additional Boards Manager URLs**:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Then install **esp32 by Espressif Systems** from the Boards Manager. Select **ESP32 Dev Module**.

### 2. Install libraries

| Library | Source |
|---|---|
| Adafruit SSD1306 | Library Manager |
| Adafruit GFX Library | Library Manager |
| ESPAsyncWebServer | GitHub (ESP32Async) |
| AsyncTCP | GitHub (ESP32Async) |

### 3. Upload

1. Put `Smart_Boom.ino` inside a folder named `Smart_Boom`.
2. Open it in the Arduino IDE.
3. Select the correct COM port and click **Upload**.
4. The OLED shows the boot animation, then the SAFE screen.

---

## Using the Dashboard

1. Power up the ESP32 and connect your phone to the Wi-Fi:

   | | |
   |---|---|
   | **SSID** | `SmartBoom_TT360` |
   | **Password** | `128128128` |
   | **Address** | `192.168.4.1` |

2. The dashboard opens automatically. If it doesn't, open `192.168.4.1` in your browser and accept the "no internet" prompt.
3. Choose a countdown of **5s, 10s or 15s**.
4. **Slide to arm.** The ring turns amber and the OLED shows the armed screen.
5. **Hold the fire button** for about 1.2 seconds. The countdown starts.
6. At zero the relay closes for 3 seconds and the cracker is lit.
7. Press **ABORT** at any time to stop and return to SAFE.

| State | Ring colour | Meaning |
|---|---|---|
| SAFE | Green | Standby, relay open |
| ARMED | Amber | Waiting for the fire command |
| COUNTDOWN | Red | Counting down, abort is available |
| FIRING | Orange | Relay closed, ignition live |
| DONE | Gold | Finished, returns to SAFE automatically |

---

## OLED Screens

| Screen | Animation |
|---|---|
| Boot | A rocket rises, bursts into particles, then "SMART BOOM" and the TechTadka360 logo are revealed |
| Safe | Scrolling starfield, a bomb with a sparking fuse, and rotating info (SSID, IP, password, users, blasts) |
| Armed | Pulsing double border, flashing warning triangle and a scanning bar |
| Countdown | A circular tick-ring that drains, large digits, and "STAND BACK!" blinking in the last 3 seconds |
| Firing | A white flash, double particle explosion and a heating progress bar |
| Done | A shaking "BOOM!", twinkling sparkles and "HAPPY DIWALI" |

---

## HTTP API

All endpoints use `GET` and return the current status as JSON.

| Endpoint | Description |
|---|---|
| `/` | Dashboard page |
| `/status` | Returns state, remaining time and blast count |
| `/arm` | SAFE → ARMED |
| `/disarm` | ARMED → SAFE |
| `/fire?t=10` | ARMED → COUNTDOWN (`t` is 5 to 30 seconds) |
| `/abort` | Immediately opens the relay and returns to SAFE |

Example response:

```json
{"s":"COUNTDOWN","ms":6400,"total":10,"fired":2}
```

---

## Configuration

Edit these lines at the top of `Smart_Boom.ino`:

```cpp
#define RELAY_PIN 18
#define LED_PIN 19
#define BUZZER_PIN 23
#define RELAY_ON LOW
#define RELAY_OFF HIGH

#define FIRE_MS 3000
#define DONE_MS 3500

const char* AP_SSID = "SmartBoom_TT360";
const char* AP_PASS = "128128128";
```

| Setting | Change it to |
|---|---|
| `RELAY_ON` / `RELAY_OFF` | Swap the values if your relay module is active-HIGH |
| `FIRE_MS` | Ignition time in milliseconds |
| `AP_SSID` / `AP_PASS` | Your own hotspot name and password (password must be at least 8 characters) |

---

## Troubleshooting

| Problem | Solution |
|---|---|
| **Relay is always ON** | You are feeding about 7V into VIN or the relay VCC. Use a buck converter set to exactly 5.0V for both. |
| Relay still stays ON with a clean 5V | Some modules don't turn off from a 3.3V HIGH. In the code use `pinMode(RELAY_PIN, INPUT)` for OFF and `pinMode(RELAY_PIN, OUTPUT); digitalWrite(RELAY_PIN, LOW)` for ON. |
| Dashboard doesn't open | Open `192.168.4.1` manually. Turn off mobile data so the phone doesn't pick another network. |
| Can't connect to Wi-Fi | The password must be at least 8 characters. Forget the network and reconnect. |
| OLED is blank | Check SDA on GPIO21, SCL on GPIO22, address `0x3C`. SH1106 displays need an SH1106 driver. |
| ESP32 resets when firing | The firing battery is sharing ground with the ESP32, or the logic supply is weak. Keep the two circuits isolated. |
| Nichrome doesn't glow | Wire is too long (resistance too high) or the battery is low. Aim for 1.5 to 3 ohm total. |
| No sound from the buzzer | Use an **active** buzzer. Passive ones need tone signals. |
| Upload fails | Hold the BOOT button while uploading, and check the COM port. |

---

## Roadmap

- [ ] Multi-channel relay for sequenced multi-cracker firing
- [ ] Live battery voltage display on the OLED and the dashboard
- [ ] Wire continuity check before arming
- [ ] Password-protected dashboard login
- [ ] Custom countdown time slider
- [ ] Sound and light show mode for Diwali

---

## Safety Disclaimer

This project is for **educational and entertainment purposes**, intended for adults and for legally sold consumer firecrackers only.

- Stay at least **10 metres** away from the cracker and keep children and pets far back.
- Never touch the nichrome wire or the firing line once the system is armed.
- Always keep the master switch in the firing line **OFF** until you are at a safe distance.
- Test the whole system with **no firecracker attached**.
- Follow your local laws and regulations on firecrackers, including permitted times and places.
- Do not use this project for anything that harms people, animals or property.

The author takes no responsibility for any injury, damage or misuse. Build and use it at your own risk.

---

## Connect With Me

<div align="center">

**TechTadka360** · Electronics and DIY maker projects

[![YouTube](https://img.shields.io/badge/YouTube-@techtadka360official-FF0000?style=for-the-badge&logo=youtube&logoColor=white)](https://www.youtube.com/@techtadka360official)
[![Instagram](https://img.shields.io/badge/Instagram-@techtadka360official-E4405F?style=for-the-badge&logo=instagram&logoColor=white)](https://www.instagram.com/techtadka360official)
[![Facebook](https://img.shields.io/badge/Facebook-TechTadka360-1877F2?style=for-the-badge&logo=facebook&logoColor=white)](https://www.facebook.com/share/1EkKAJNLdB/)
[![GitHub](https://img.shields.io/badge/GitHub-Mr--soumik-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/Mr-soumik)

If this project helped you, please **star the repo**, **subscribe** on YouTube and **share it** with a fellow maker.

Happy Diwali!

</div>

---

## License

Released under the **MIT License**. You are free to use, modify and share this project, with credit to **TechTadka360**.

<div align="center">

Made with love by **Soumik · TechTadka360**

</div>
