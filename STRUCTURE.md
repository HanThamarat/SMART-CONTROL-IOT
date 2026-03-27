# Project Structure 🏗️

This repository follows the default PlatformIO layout, with the application firmware living in `src/` and expansion points for shared headers, reusable libraries, and tests.

## Tree 🌳

```text
SMART-CONTROL-IOT/
├── include/
│   └── README
├── lib/
│   └── README
├── src/
│   └── main.cpp
├── test/
│   └── README
├── platformio.ini
├── README.md
└── STRUCTURE.md
```

## Directory Guide 🗂️

| Path | Purpose |
|---|---|
| `src/` | Main firmware source code |
| `include/` | Shared header files |
| `lib/` | Private project libraries |
| `test/` | PlatformIO test files |

## File Highlights ✨

### [src/main.cpp](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/src/main.cpp)

Contains the active firmware logic:

- Wi-Fi station connection
- MQTT setup and callback handling
- reconnect logic
- serial logging
- heartbeat publishing

### [platformio.ini](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/platformio.ini)

Defines how the project is built:

- `espressif8266` platform
- `huzzah` board target
- Arduino framework
- PubSubClient dependency

### [README.md](/Users/hanthamarat/Documents/PlatformIO/Projects/SMART-CONTROL-IOT/README.md)

Main project documentation for:

- setup
- build and upload
- MQTT topics
- current project status

## Expansion Notes 🔌

### `include/`

Use this folder for shared `.h` files when the project grows beyond a single source file.

### `lib/`

Use this folder for reusable modules such as:

- Wi-Fi manager
- MQTT client wrapper
- relay or sensor drivers

### `test/`

Use this folder for PlatformIO tests once firmware behavior is split into testable units.
