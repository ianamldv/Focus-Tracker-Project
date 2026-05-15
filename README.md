# FOCUS TRACKER📵

An ESP32-based phone lock box that enforces focused study sessions. Lock your phone inside, set a timer, and the servo-driven latch won't release until your session is complete. Use an RFID card to unlock short breaks — you only get 3.

A live web dashboard accessible over Wi-Fi lets you monitor the session, start breaks, or stop the timer remotely.

---

## How it works

1. Power on the device. The box starts unlocked — place your phone inside.
2. Type your desired study duration in minutes on the keypad, then press `#` to start.
3. The servo locks the lid. A countdown appears on the OLED display.
4. To take a break, tap an authorized RFID card. You have **3 breaks per session**.
5. Type the break duration (max 30 minutes) and press `#`. The lid unlocks.
6. When the break ends, the lid locks again automatically and the study timer resumes.
7. When the session countdown reaches zero, the lid unlocks and a summary is shown.
8. Press `*` on the summary screen to start a new session.

To cancel a session mid-way, hold `*` on the keypad for 3 seconds.

---

## Hardware

| Component | Part |
|---|---|
| Microcontroller | ESP32 DevKitV1 |
| RFID reader | RC522 |
| Display | 0.96" OLED (SSD1306, I2C, 128x64) |
| Keypad | 4x4 matrix keypad |
| Servo | SG90 |
| Buzzer | Passive buzzer |

---

## Wiring

### RC522 RFID Reader (SPI)

| RC522 Pin | ESP32 Pin |
|---|---|
| SDA (SS) | D5 (GPIO 5) |
| SCK | D18 (GPIO 18) |
| MOSI | D23 (GPIO 23) |
| MISO | D19 (GPIO 19) |
| RST | D4 (GPIO 4) |
| 3.3V | 3.3V |
| GND | GND |

### OLED Display (I2C)

| OLED Pin | ESP32 Pin |
|---|---|
| SDA | D21 (GPIO 21) |
| SCL | D22 (GPIO 22) |
| VCC | 3.3V |
| GND | GND |

### 4x4 Matrix Keypad

| Keypad Pin | ESP32 Pin |
|---|---|
| Row 1 | D13 (GPIO 13) |
| Row 2 | D12 (GPIO 12) |
| Row 3 | D14 (GPIO 14) |
| Row 4 | D27 (GPIO 27) |
| Col 1 | D26 (GPIO 26) |
| Col 2 | D25 (GPIO 25) |
| Col 3 | D33 (GPIO 33) |
| Col 4 | D32 (GPIO 32) |

### SG90 Servo

| Servo Wire | ESP32 Pin |
|---|---|
| Signal (orange) | D15 (GPIO 15) |
| VCC (red) | 5V (VIN) |
| GND (brown) | GND |

> **Note:** Power the servo from the 5V/VIN pin, not 3.3V. 

### Passive Buzzer

| Buzzer Pin | ESP32 Pin |
|---|---|
| Signal | D2 (GPIO 2) |
| GND | GND |

---

## Keypad reference

| Key | Function |
|---|---|
| `0`–`9` | Enter digits |
| `#` | Confirm input |
| `*` | Clear input / hold 3s to cancel session |
| `A` `B` `C` `D` | Unused |

---

## Web dashboard

Once connected to Wi-Fi, the ESP32 hosts a dashboard at its local IP address (shown on the OLED at startup). Open it in any browser on the same network.

The dashboard shows the current session state, remaining time, break usage, and session history. You can also start a session, trigger a break, or stop the timer remotely from the dashboard.

The dashboard files (`index.html`, `style.css`, `app.js`) are stored on the ESP32's flash filesystem using LittleFS and must be uploaded separately before first use (see Setup below).

---

## Setup

### 1. Clone the repo

```bash
git clone https://github.com/yourname/rfid-study-timer.git
cd rfid-study-timer
```

### 2. Add your Wi-Fi credentials

Copy the example secrets file and fill it in:

```bash
cp include/secrets_example.h include/secrets.h
```

Open `include/secrets.h` and replace the placeholders:

```cpp
#define WIFI_SSID     "your_network_name"
#define WIFI_PASSWORD "your_password"
```

### 3. Add your RFID card UIDs

Scan your cards using a UID reader sketch, then update `include/config.h`:

```cpp
const String authorizedUIDs[] = {
  "XX:XX:XX:XX",  // your card
  "XX:XX:XX:XX"   // your fob
};
```

### 4. Set your timezone

In `include/config.h`, update the GMT offset in seconds:

```cpp
const long gmtOffset = 7200; // GMT+2 (Moldova) — change to match your timezone
```

### 5. Upload the filesystem

In VS Code with PlatformIO, open the PlatformIO sidebar and run:
**"Upload Filesystem Image"** (under your environment's Platform section)

This uploads the `data/` folder (HTML/CSS/JS dashboard) to the ESP32's flash.

### 6. Build and upload

```bash
pio run --target upload
```

Or use the Upload button in VS Code.

---

## Dependencies

All libraries are installed automatically by PlatformIO from `platformio.ini`.

| Library | Purpose |
|---|---|
| miguelbalboa/MFRC522 | RC522 RFID reader |
| adafruit/Adafruit SSD1306 | OLED display driver |
| adafruit/Adafruit GFX Library | Graphics primitives |
| Chris--A/Keypad | Matrix keypad scanning |
| madhephaestus/ESP32Servo | Servo PWM on ESP32 |
| bblanchon/ArduinoJson | JSON for web API |
| lorol/LittleFS_esp32 | Filesystem for dashboard files |

---

## Project structure

```
rfid-study-timer/
├── data/                  # Web dashboard files (uploaded to LittleFS)
│   ├── index.html
│   ├── style.css
│   └── app.js
├── include/
│   ├── config.h           # Pin definitions, constants, state enum
│   ├── secrets.h          # Wi-Fi credentials (not committed)
│   └── secrets_example.h  # Template for secrets.h
├── src/
│   ├── main.cpp           # setup() and loop()
│   ├── session.cpp/.h     # Session state and history
│   ├── display_oled.cpp/.h # OLED screen functions
│   ├── buzzer.cpp/.h      # Tone and beep functions
│   ├── servo_lock.cpp/.h  # Servo lock/unlock
│   ├── rfid_reader.cpp/.h # RFID scanning and auth
│   └── web.cpp/.h         # Wi-Fi, web server, JSON API
└── platformio.ini
```

---

## License

MIT