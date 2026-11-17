# Adding a board

Default language: **en-US**. Read this before writing code. Bitchat Stick is one shared ESP32 firmware core plus a board profile chosen at build time. It is not one binary that detects every ESP32, and it is not a second product name.

The only supported profile today is `m5stickc-plus2`. Another board stays unsupported until its own build and physical acceptance exist. Missing a display does not turn the node into a relay, does not disable Noise, and does not change forwarding.

## Stop if a fact is missing

Do not copy the Plus2 flash layout, PSRAM flags, GPIO4 hold, or JoyC pins onto another module. Fill this sheet first.

| Fact | Plus2 value |
| --- | --- |
| Profile id | `m5stickc-plus2` |
| PlatformIO environment | `m5stickc-plus2` |
| Compile flag | `-DSTICK_BOARD_M5STICKC_PLUS2` |
| PlatformIO board | `m5stick-c` |
| Platform | `espressif32@6.9.0`, Arduino framework |
| Flash | 8 MB, `partitions.csv`, app limit 6291456 |
| PSRAM | `-DBOARD_HAS_PSRAM` and `-mfix-esp32-psram-cache-issue` |
| Early power | GPIO4 output HIGH before `M5.begin` |
| Buttons | A, B, C as `M5.BtnPWR` |
| External I2C | JoyC on GPIO 0 and GPIO 26, port 0, 100 kHz |
| RTC | Internal M5 RTC |
| Display | `135x240-portrait` via `src/boards/m5stickc-plus2/Display.cpp` |
| Product name | `Bitchat Stick` in the core. Not a profile field |
| `!info` hardware word | `ESP32`, fixed in `src/InfoBot.h` |

Empty template:

| Fact | Value |
| --- | --- |
| Profile id | |
| Module and flash size | |
| USB/serial upload path | |
| Partition table, checked against that flash | |
| PSRAM or none | |
| Early power pins | |
| Buttons | |
| Buses and pins actually wired | |
| Display profile, or none | |
| Speaker, battery, RTC, HAT | |

## Source sets

The Plus2 environment compiles `src/boards/m5stickc-plus2/` and excludes every other directory under `src/boards/`. `include/Board.h` fails the build if `STICK_BOARD_M5STICKC_PLUS2` is missing, so there is no implicit generic board.

| Set | Files | Rule |
| --- | --- | --- |
| Core | `src/main.cpp`, `src/Clock.cpp`, scanner, transport, mesh, codec, Noise, identity, settings | No `M5Unified.h` and no `M5StickC` text |
| Board | `src/boards/m5stickc-plus2/Profile.cpp`, `Board.cpp`, `Display.cpp` | Power, RTC, buttons, battery, panel |
| Presentation | `src/Ui.cpp`, `src/assets/`, `src/Feedback.cpp`, `src/Joyc.cpp` | Required by this profile because it has a display, speaker, and optional HAT |

`src/Feedback.cpp` is the speaker adapter. `src/Joyc.cpp` is the optional Mini JoyC HAT adapter. Neither is a separate firmware target. A future environment without `speaker` or `hat` must not compile them. `src/hardware/SettingsSelfTest.cpp` also drives GPIO4 and stays exclusive to the `settings-selftest` environment.

Do not rename the NVS namespace `bitchat-stick`. Do not upgrade `espressif32@6.9.0`, NimBLE-Arduino 2.5.0, M5Unified, M5GFX, or the pinned Monocypher commit as part of adding a board.

## Proof

`sh tools/test-native.sh` and `pio run -e <that-environment>`. Confirm the map does not pull Plus2 GPIO4 or M5 init into a different profile. Hardware support still requires a device record. A successful compile is not enough.

See [adding a display](adding-a-display.md) before attaching a panel, and [the multi-board plan](multi-board-plan.md) for what remains proposed.
