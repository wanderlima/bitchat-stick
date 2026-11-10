# Protocol compatibility investigation

Research snapshot: November 9, 2026. This was a documentary investigation of primary sources, not a radio capture, firmware audit, or Plus2 interoperability test. Mutable branch links do not pin a release. Third-party descriptions demonstrate existence and claimed scope, not independent certification.

## Conclusion

A BLE Bitchat node on the Plus2 is plausible, but **messaging compatibility for this project remains unproven**. Advertisement discovery is not relay validation. Public ESP32 implementations exist; chip, stack, and protocol versions matter. Recognizing a geohash or displaying a channel name does not implement Nostr.

## Primary sources

| Source | Observed evidence | Implication |
|---|---|---|
| [Official Bitchat README](https://github.com/permissionlesstech/bitchat#channel-types) | Separates local `#bluetooth` from geographic Nostr channels | Do not promise offline geographic channels in a BLE MVP |
| [Official whitepaper](https://github.com/permissionlesstech/bitchat/blob/main/WHITEPAPER.md) | Identifies itself as v2.0, July 6, 2026; describes framing, presence, forwarding, sessions | Compare with actual serializers in selected commits |
| [Official Android implementation](https://github.com/permissionlesstech/bitchat-android) | Public mobile reference | Validate independently from Swift; cross-build equivalence is untested here |
| [fvolcic/bitchat-relay](https://github.com/fvolcic/bitchat-relay) | Declares a BLE MVP, ANNOUNCE/KEY_EXCHANGE handling, and partial deduplication | Study candidate, not a proven complete current implementation |
| [hackerhouse-opensource/bitchat-esp32](https://github.com/hackerhouse-opensource/bitchat-esp32) | Describes minimal C6/Zephyr firmware, UART interface, handshakes; cites XIAO ESP32C6 tests | Classic-ESP32 port and Plus2 UI are additional work |
| [bitleproject/bitle](https://github.com/bitleproject/bitle) | Claims relay/Noise on C3/S3 using ESP-IDF/NimBLE | Broader reference; no Plus2 support demonstrated by the consulted source |
| [M5Stack Plus2 hardware](https://docs.m5stack.com/en/core/StickC-Plus2) | Defines the actual target and controls | It is not a C3/C6/S3; binaries are not interchangeable |

No complete security/license audit of these implementations was performed. Handshake logs from another board do not demonstrate A→Stick→B delivery with the owner's phone builds.

## Scanner UUID evidence

The current scanner uses [BLEService.swift at commit 04995f68e470a18d1fc99f86fa326c2a87bab4f2](https://github.com/permissionlesstech/bitchat/blob/04995f68e470a18d1fc99f86fa326c2a87bab4f2/bitchat/Services/BLE/BLEService.swift):

- Mainnet: `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C`.
- Testnet: `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5A`.

This pins the origin of an advertisement filter only. It is not an approved mobile release or packet contract. A spoofed advertisement can match either UUID. Passive scanning may miss fields advertised only in scan responses.

## Contract to verify before messaging

- GATT service/characteristic UUIDs, properties, write/notify directions, CCCD, MTU negotiation, reconnect behavior, and roles.
- Serializer/deserializer versions, byte order, flags, lengths, types, recipient, signatures, padding, and fragments.
- Signed-announcement validation, key/peer-ID binding, identity lifetime, expiry, and replay.
- TTL, deduplication/destination rules, nonforwardable types, and history-sync behavior.
- Transit ciphertext versus terminating a Noise session. Verify the actual pattern, algorithms, binding, and state machine in pinned builds.
- Where BLE-local, groups, and Nostr contexts are identified and which keys are required.
- Explicit association events versus reachability; test replay, reconnection, and context identity.

The whitepaper's peer-presence announcements do not establish authoritative channel membership. This investigation did not establish a reliable welcome-bot trigger. That is an evidence gap, not a universal proof that no future implementation could offer one.

## Confidence matrix

| Capability | External evidence | Plus2 project status |
|---|---|---|
| Local BLE chat/relay in apps | Documented officially | Messaging/relay unimplemented |
| Geographic Nostr channels | Documented officially | Outside MVP |
| Passive BLE scanner | Implemented foundation | Compiled; actual observation coverage needs device testing |
| ESP32 relay | Third-party claims | Portability/interoperability unverified |
| Public-message reading | Depends on format/build | Unimplemented |
| `!info`, `!rules`, `!help` | Proposed application interface | Unimplemented |
| Arbitrary/password channels | Historical suggestions are insufficient | Do not promise |
| Reliable channel join | Insufficient evidence | Welcome blocked |
| Pocket Messenger | Product proposal | Future |

## Required version record

Before messaging work, record app A/build/OS/SHA, app B/build/OS/SHA, external code SHAs/licenses, SDK/stack versions, board manifest, supported packet types, and passing tests. Only the UUID-source snapshot and foundation dependencies are currently pinned. If documentation and code diverge, record and test what the selected clients actually emit.
