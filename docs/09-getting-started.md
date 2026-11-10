# Getting started — implementation status

Firmware foundation v0.1.0. English (en-US) is the default. The design documents describe a wider future product; this page distinguishes implemented code from verified hardware behavior.

## Implemented foundation

- C++/Arduino/PlatformIO target for the Plus2 ESP32-PICO-V3-02, 8 MB flash, PSRAM enabled.
- Early GPIO4 HOLD assertion, M5Unified initialization, portrait display, voltage and battery estimate.
- Centered splash name, logo placeholder, and consistent navigation instructions. A enters Home without a timed auto-dismiss or blocking scan delay.
- Home, Nearby, Channels, Messages, InfoBot, and Settings screens. Unimplemented messaging features show their actual unavailable state rather than sample chat.
- Passive BLE observation: no advertising, GATT links, or chat transmission. Five-second scan periods with approximately two seconds between them; 80 ms windows every 160 ms during a scan.
- Real advertisement fields when running on hardware: sanitized name, address/type, RSSI, age, and mainnet/testnet candidate classification.
- Bounded 32-event queue and 32-observation table; expire after 60 seconds and replace the oldest observation when full.
- Shared input handling, screen-off after 30 seconds, wake-only first press, 5/25/50/75/100% brightness presets (50% default), and RAM-only settings.
- Live header status square/battery icon; dedicated Battery, About, and Diagnostics views. About credits Wander Lima and github.com/wanderlima with a clearly labeled QR placeholder.
- Larger bold text, strong contrast, grouped panels, and a green Nearby card/left border. No regular-screen navigation footer.

Current screenshots are illustrative previews. This revision still needs physical UI, power, and scan validation after upload.

## Not implemented

Bitchat message GATT transport, packet codec, signed identity, verified peers, Noise participation, relay, chat reading, channel creation/joining, InfoBot, welcomes, Nostr, persistent settings/history, OTA, and Pocket Messenger. There is no hidden switch that enables them.

## Pinned dependencies

| Component | Version |
|---|---|
| PlatformIO Core used for validation | 6.1.19 |
| espressif32 platform | 6.9.0 |
| Arduino-ESP32 | 2.0.17, resolved package `3.20017.241212+sha.dcc1105b` |
| M5Unified | 0.3.0 |
| M5GFX | 0.3.0 |
| NimBLE-Arduino | 2.5.0 |
| Xtensa GCC | `8.4.0+2021r2-patch5` |

The PlatformIO board base is `m5stick-c`, explicitly overridden for 8 MB flash, PSRAM, and the local partition file. Runtime initialization checks for Plus2 identification. A 6 MiB factory application partition is used; the remainder is reserved, not conversation storage. Do not flash C3/C6/S3 binaries or select a different board casually.

The scanner UUID origin is pinned in the [compatibility investigation](07-compatibility.md#scanner-uuid-evidence). This is advertisement filtering, not messaging homologation. Addresses may rotate, names may collide, advertisements may be forged, and a passive observer may miss scan-response-only fields.

## Build

See the [complete build/upload guide](10-build-deploy.md). From the root:

```sh
pio run -e m5stickc-plus2
```

If `pio` is not on PATH on this machine:

```sh
/Users/wanderlima/.platformio/penv/bin/pio run -e m5stickc-plus2
```

The first build downloads dependencies. Generated `.pio/build/m5stickc-plus2/firmware.bin` is ignored by Git. It is an application image, not a complete standalone installer; the initial upload also needs boot/partition components.

## Native tests

From the root, using a C++ compiler:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -I include src/PeerTable.cpp test/peer_table_test.cpp -o /tmp/bitchat-peer-table-test
/tmp/bitchat-peer-table-test
```

Shared input regression tests, with supported sanitizers:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined test/navigation_input_test.cpp -o /tmp/bitchat-navigation-test
/tmp/bitchat-navigation-test
```

UI state/layout tests use a lightweight display fake and font advance metrics from pinned M5GFX; they execute the real `src/Ui.cpp`:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -I test/fakes -I include test/ui_test.cpp src/PeerTable.cpp -o /tmp/bitchat-ui-test
/tmp/bitchat-ui-test
```

They cover navigation through screens/sub-screens, brightness application/cancellation, full author attribution, live header/data, grouping, and text/rectangle bounds. Software rendering is not a physical display test.

The table test covers updates, address-type distinction, bounds/oldest replacement, termination, exact expiry, and clock wrap. The input test covers A/B/C, single long-B dispatch, no short action after long-C, wake suppression, and wrap. Both have passed natively with memory/undefined-behavior sanitizers. They do not validate the display, radio callbacks, power, or protocol.

## Controls

| Input | Action |
|---|---|
| A short | Confirm/open; show/close detail; change selected brightness |
| B short | Up / previous observation |
| C short (`M5.BtnPWR`) | Down / next observation |
| B held ≥700 ms | Back once; Messages→Channels; Settings sub-screens→Settings; details→list; other pages→Home; Home→Settings |
| A held ≥700 ms | Reserved; no confirmation on release |
| C held ≥700 ms | No down action; 6-second hold retains power off |

On the splash screen, only A enters Home; B/C and long-B do not change pages. Short actions occur on release. After screen-off, the first A/B/C press only wakes it; suppress navigation until all buttons are released. Settings are volatile. Real USB/battery power-off behavior must be checked on the Plus2.

The UI uses FreeSansBold9pt7b (22-pixel line advance), FreeSansBold12pt7b for the brand, and Font2 at 16 pixels for compact information. Text is English; non-ASCII radio-name bytes still render as `?`. Current JPEG previews are not pixel-identical hardware screenshots.

## Source structure

- `src/main.cpp`: initialization and application loop.
- `src/Scanner.cpp`: NimBLE and bounded observation queue.
- `src/PeerTable.cpp`, `include/PeerTable.h`: hardware-independent observation model.
- `src/Ui.cpp`: display, common screen layout, navigation, power-display settings.
- `src/NavigationInput.h`: shared input semantics.
- `test/peer_table_test.cpp`, `test/navigation_input_test.cpp`, `test/ui_test.cpp`, `test/fakes/M5Unified.h`: host regressions and hardware fake.
- `platformio.ini`, `partitions.csv`: build target and flash layout.

Only the loop mutates PeerTable. BLE callbacks enqueue fixed-size observations without blocking and never draw. Drain at most 32 events per iteration; count queue drops. Observations are RAM-only. Under load, relevant observations may be replaced; this is not a complete network inventory.

## Evidence and hardware checklist

The initial firmware compiled successfully. Initial static RAM: 42,480 bytes; initial application flash: 846,013 bytes. A later navigation/typography/splash build also compiled (42,496 bytes static RAM, 856,077 bytes application flash). These are build snapshots, not runtime heap or battery measurements. The final en-US/header/settings revision compiled successfully with 42,496 bytes of static RAM and 857,181 bytes of application flash. All three native suites passed with sanitizers. The owner reported using an earlier build on the Plus2. No new firmware was uploaded by the agent during these edits.

After uploading the latest revision:

1. Verify USB and battery-only boot, Plus2 identification, and HOLD behavior.
2. Check centered splash/placeholder, English controls, and A entering Home.
3. Exercise A/B/C and long presses on all screens; ensure no accidental action after wake/long press.
4. Check larger text, panel boundaries, high-contrast selection, green Nearby card, and lack of regular navigation footers.
5. Compare voltage/estimate on USB/battery; do not infer remaining hours.
6. Run Bitchat on phones A/B and compare actual advertised UUIDs, permissions, foreground/background behavior, RSSI, and expiry.
7. Observe heap/queue drops for 60 minutes in an owned test environment.
8. Confirm screen-off leaves scanning scheduled and holding C behaves correctly with/without USB.
9. Keep messages, relay, and bot disabled until the [protocol test gates](06-testing.md) pass.
