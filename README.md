# SMART-CONTROL-IOT ⚡

An ESP8266 + PlatformIO firmware project for connecting a HUZZAH board to Wi-Fi and controlling devices over MQTT.

```text
ESP8266 -> Wi-Fi -> MQTT Broker -> Smart Control Messages
```

## Overview 🚀

This project currently acts as a small MQTT-connected firmware prototype. It brings the board online, connects to a broker, listens for commands, and publishes periodic messages for monitoring or control flows.

The firmware in [src/main.cpp](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/src/main.cpp) currently:

- connects to a Wi-Fi network in station mode
- authenticates with an MQTT broker
- subscribes to `TEST/COMMAND`
- publishes `ESP8266 Online` to `TEST/STATUS`
- publishes a heartbeat value to `light/control` every 5 seconds
- prints received MQTT messages to the serial monitor

## Snapshot 📌

| Item | Value |
|---|---|
| Board | Adafruit HUZZAH ESP8266 |
| Platform | `espressif8266` |
| Framework | Arduino |
| Build Tool | PlatformIO |
| MQTT Library | `knolleary/PubSubClient@^2.8` |
| Serial Baud | `9600` |

## Quick Start 🛠️

### 1. Install prerequisites 📦

- [PlatformIO](https://platformio.org/)
- ESP8266-compatible board
- USB cable / serial connection
- Wi-Fi access
- MQTT broker reachable from the board

### 2. Update device settings 🔧

Open [src/main.cpp](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/src/main.cpp) and change:

```cpp
#define WIFI_STA_NAME "your-wifi-name"
#define WIFI_STA_PASS "your-wifi-password"

#define MQTT_SERVER   "192.168.x.x"
#define MQTT_PORT     1883
#define MQTT_USERNAME "your-user"
#define MQTT_PASSWORD "your-password"
#define MQTT_NAME     "smart-control-esp8266"
```

### 3. Build the firmware 🏗️

```bash
pio run
```

### 4. Upload to the board 📤

```bash
pio run --target upload
```

### 5. Open serial monitor 🖥️

```bash
pio device monitor
```

## MQTT Behavior 📡

| Action | Topic | Payload |
|---|---|---|
| Publish on connect | `TEST/STATUS` | `ESP8266 Online` |
| Subscribe on connect | `TEST/COMMAND` | incoming command messages |
| Periodic publish | `light/control` | `30` |
| Extra subscribe call in loop | `TEST/MQTT` | broker-side messages |

## PlatformIO Configuration ⚙️

The project defines one environment in [platformio.ini](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/platformio.ini):

```ini
[env:huzzah]
platform = espressif8266
board = huzzah
framework = arduino
lib_deps = knolleary/PubSubClient@^2.8
```

## Firmware Flow 🔄

```text
Boot
  -> Start Serial
  -> Connect Wi-Fi
  -> Set MQTT server + callback
  -> Retry MQTT connection every 5 seconds
  -> Process messages
  -> Publish heartbeat
```

## Project Layout 🗂️

See [STRUCTURE.md](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/STRUCTURE.md) for the full folder map.

Important files:

- [src/main.cpp](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/src/main.cpp): main firmware logic
- [platformio.ini](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/platformio.ini): board and dependency configuration
- [STRUCTURE.md](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/STRUCTURE.md): folder reference

## Current Limitations ⚠️

- Wi-Fi and MQTT credentials are hardcoded in source.
- MQTT topics are still prototype-level and not yet normalized.
- The loop contains a repeated subscribe call that should likely be cleaned up later.
- No project-specific tests have been added yet.

## Next Improvements 💡

- move secrets into a separate config header or environment-based setup
- define stable MQTT topic naming
- split network and MQTT logic into reusable modules
- add tests under [test/README](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/test/README)
