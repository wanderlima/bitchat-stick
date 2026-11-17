# Requirements and scope

## Context

Bitchat Stick is modular BLE chat firmware for ESP32. The initial supported profile is the M5StickC Plus2, with the Mini JoyC HAT as an optional peripheral of that profile. One build produces one board image; the Plus2 binary is not a universal ESP32 firmware. The implemented participant is described in [getting started](09-getting-started.md). Earlier conversation suggestions are product context, not protocol evidence. English (en-US) is the default for firmware and documentation.

## Proposed functional requirements

| ID | Requirement | Enablement criterion |
|---|---|---|
| RF01 | BLE scanner with RSSI and last-seen age | Observe real advertisements; a matching UUID is not an identity |
| RF02 | Device/peer states | Distinguish observed, candidate, connected, verified, and expired states as supported |
| RF03 | BLE mesh relay | Demonstrate transport, codec, TTL, deduplication, and indirect delivery |
| RF04 | Channel/context monitor | Identify the actual context in the selected mobile protocol/build |
| RF05 | Authorized message reading | Decode the payload while respecting cryptographic boundaries |
| RF06 | InfoBot: `!info`, `!rules`, `!help` | Compatible reception/transmission plus spam controls |
| RF07 | Join-triggered welcome message | Reliable, authenticated context-and-identity association event |
| RF08 | Power, errors, and statistics | Use available measurements or show “Unavailable” |
| RF09 | Initial Pocket Messenger composer | Read messages, choose a reply, and confirm its destination before sending |

RF03–RF07 remain gated by protocol investigation. Discovery, announcements, reconnections, a new nickname, or a first message must not be substituted for a channel join.

## Nonfunctional requirements

- Physical-button operation: A confirms, B moves up, C moves down, and holding B returns. Holding C preserves power control.
- Consistent screen behavior, readable bold text, high contrast, and information grouping. Keep navigation instructions on the splash screen rather than every footer.
- Explicit memory limits for tables, queues, fragments, and history. Count drops; never grow allocations without bounds based on radio input.
- Validate packet sizes, versions, flags, and limits before allocation or copying.
- A relay may carry private ciphertext without reading it. The bot must not read encrypted traffic without legitimate participation and keys.
- Keep Wi-Fi outside the BLE MVP. Persist configuration/identity only when needed; do not persist conversations by default.
- Retry connections with backoff when that feature exists. Keep UI responsive during scanning and advertisement floods.
- Distinguish queued, transmitted, and acknowledged states; transmission alone is not delivery.
- Measure power with the display on/off and relay active. Do not promise distance or battery life.

## Hardware and tool evaluation

The [official Plus2 specifications](https://docs.m5stack.com/en/core/StickC-Plus2) establish the target and its physical controls. Do not confuse the Plus2 power circuit with the Plus/AXP192 model. Verify the board revision and library behavior before using pins or power APIs.

The firmware uses C++, Arduino and PlatformIO, with versions pinned in `platformio.ini`. The shared core does not include M5Unified. The Plus2 profile uses M5Unified 0.2.8 and M5GFX 0.2.36 for power, display, RTC, speaker and the optional JoyC HAT. NimBLE-Arduino 2.5.0 carries BLE. M5Unified is a board adapter, not Bitchat compatibility. The [battery documentation](https://docs.m5stack.com/en/arduino/m5stickc_plus2/battery) links to its power layer. ESP-IDF remains an alternative if later protocol work requires it. Do not mix Arduino/IDF/NimBLE versions without build and runtime evidence.

## Outside the BLE MVP

Nostr/Wi-Fi, LoRa, audio, images, OTA, comfortable free-text composition, universal interoperability, remote moderation, and guaranteed delivery. Private channels/groups require separate validation. This is not an established critical-communications device.
