# Getting started — implementation status

Firmware **v0.2.0**, English (en-US). Source implementation and physical acceptance are tracked separately in the [device checklist](13-device-checklist.md).

## Implemented

- PlatformIO/Arduino firmware for Plus2, early power HOLD, physical navigation, real voltage/estimated battery, screen timeout and wake suppression.
- Eleven working screens with FreeMono, warm cream, grouped panels, dividers, green Nearby styling, and buffered rendering to remove erase/redraw flicker.
- Persistent brightness: 5/25/50/75/100%, default 50%; explicit apply writes NVS and storage failures are visible.
- Passive UUID-filtered candidate scanning alongside mainnet GATT advertising. At most two subscribed peripheral links; mobiles initiate the BLE connection.
- Persistent Noise/signing identity, bounded v1/v2 codec, signed packets, raw-DEFLATE expansion, fragment reassembly, automatic Noise XX and authenticated peer-state binding.
- Twelve signed peer records; candidate, signed-only, and Noise-authenticated statuses are distinct.
- Twelve public/private received messages in RAM, 192 displayed bytes each, paging and text scrolling. Unsupported display glyphs are substituted; malformed UTF-8 is rejected.
- Signature-checked public relay with TTL reduction, deduplication and ingress exclusion; paced MTU-aware notifications with bounded queues.
- Authenticated, recent public InfoBot commands and rate limits; immediate on-device Bot ON/OFF switch. This switch resets to ON at reboot; brightness and local identity persist.
- Real repository QR, Wander Lima credits, BLE/heap/uptime counters and UTC RTC status. Serial telemetry contains counters, not message bodies or private keys.

## Confirmed hardware evidence

The earlier brightness self-test saved/read back a changed value across ESP32 restart and restored the original setting. The integrated firmware exposed an initialization assertion when GATT was registered during scanning; pausing scanning before registration fixed the reproduced reboot loop. After correction the Stick stayed running with an active RTC, a subscribed Android link, a verified signed announcement and an authenticated Noise peer. Android Bitchat 2.2.0 independently logged successful signature verification and Noise establishment. The owner confirmed network visibility.

Full two-phone messaging/relay, iOS payload exchange, display flicker/readability, battery-only operation, and endurance remain physical acceptance items, not claimed passes. Use the [checklist](13-device-checklist.md).

## Future scope

Private-message sending, media/groups, arbitrary encrypted channels, Nostr, gossip synchronization, general routed Noise forwarding, OTA, persistent chat history and Pocket Messenger sending are not implemented. Welcomes remain disabled until reliable entry detection exists. Keys are stored in unencrypted NVS; peer key pins/history are RAM-only.

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

The full cryptographic suite uses macOS system SHA-256, Swift/CryptoKit and installed pinned PlatformIO dependencies. Run all eleven host suites, including the actual scanner callback with NimBLE/queue fakes:

```sh
sh tools/test-native.sh
```

This covers filter decisions, sanitization, bounded queue/drop counters and retry behavior; it does not emulate radio discovery or prove mobile interoperability.

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
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -I test/fakes -I include test/ui_test.cpp src/PeerTable.cpp src/PeerSelection.cpp src/Settings.cpp -o /tmp/bitchat-ui-test
/tmp/bitchat-ui-test
```

Brightness persistence regressions use the real storage adapter with an NVS fake:

```sh
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -I test/fakes -I include test/settings_test.cpp src/Settings.cpp -o /tmp/bitchat-settings-test
/tmp/bitchat-settings-test
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

On the splash screen, only A enters Home; B/C and long-B do not change pages. Short actions occur on release. After screen-off, the first A/B/C press only wakes it; suppress navigation until all buttons are released. Brightness persists in NVS; transient screen/navigation state remains volatile. Real USB/battery power-off behavior must be checked on the Plus2.

The UI uses native FreeMono body/compact bitmaps at 16/14px, generated from pinned M5GFX glyphs to preserve strokes without fractional scaling. Titles use FreeMonoBold9pt7b and the brand uses FreeMonoBold12pt7b. Primary cream is #EBDCBA, quantized to RGB565 0xEEF7. The 20px BLE header has a subtle outline. Text is English; non-ASCII radio-name bytes still render as `?`. Current JPEG previews are not pixel-identical hardware screenshots. InfoBot command pages explain public replies; the ON/OFF row controls real behavior for the current boot.

## Source structure

- `src/main.cpp`: initialization and application loop.
- `src/Scanner.cpp`: NimBLE and bounded observation queue.
- `src/Transport.cpp`, `src/Clock.cpp`: peripheral GATT queues/fragmentation and UTC RTC setup.
- `src/Mesh.cpp`: signed peers, public/DM history, Noise authentication, relay and InfoBot.
- `src/Identity.cpp`, `src/CryptoPrimitives.cpp`, `src/NoiseSession.cpp`, `src/PacketSecurity.cpp`: persistence and protocol security.
- `src/PeerTable.cpp`, `include/PeerTable.h`: hardware-independent observation model.
- `src/Ui.cpp`: display, common screen layout, navigation, power-display settings.
- `src/NavigationInput.h`: shared input semantics.
- `src/assets/`: local Figma SVGs and generated QR/radio/font bitmaps.
- `tools/`: deterministic asset/font generation and host preview rendering.
- `test/peer_table_test.cpp`, `test/navigation_input_test.cpp`, `test/ui_test.cpp`, `test/fakes/M5Unified.h`: host regressions and hardware fake.
- `platformio.ini`, `partitions.csv`: build target and flash layout.

Only the loop mutates PeerTable. BLE callbacks enqueue fixed-size observations without blocking and never draw. Drain at most 32 events per iteration; count queue drops. Observations are RAM-only. Under load, relevant observations may be replaced; this is not a complete network inventory.

## Evidence and hardware checklist

The v0.2 target built and uploaded successfully. Native tests cover fourteen suites plus an independent CryptoKit signature check. The hardware-reproduced GATT startup assertion was corrected, Android verified the announcement and Noise session, and the owner confirmed network visibility. Buffered drawing replaces normal direct erase/redraw; visual acceptance remains a device check.

The owner initially tested a DM while the reader supported only public BLE text. Authenticated private-text decoding, encrypted delivery receipts and DM labels were subsequently added. DMs remain separate from public InfoBot processing; DM commands never generate public replies. Direct-message sending/composition is still future scope.

The earlier brightness self-test verified NVS readback across ESP32 restart and restored the original setting. Use the [complete acceptance checklist](13-device-checklist.md) for final display, Android/iOS message reception, controlled relay, reconnect and battery tests. Upload verification does not mark those tests as passed.
