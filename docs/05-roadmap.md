# Evidence-driven roadmap

Do not estimate later phases before transport feasibility is established. Each gate depends on the preceding evidence; a visually complete UI does not promote an unverified protocol feature.

| Phase | Deliverable | Exit gate |
|---|---|---|
| 0 — design | Requirements, architecture, screens, investigation, tests | Scope and evidence gaps recorded |
| 1 — pinned protocol | Mobile versions/SHAs, GATT/packet tables, licenses | Both platform implementations are traceable |
| 2 — Plus2 foundation | Build, display, buttons, power, scanner | Correct hardware behavior, readable UI, responsive scan |
| 3 — interoperability | GATT links and messages with phones A/B | Both directions, Unicode, MTU, reconnect tests |
| 4 — relay | TTL, deduplication, bounded queues, metrics | Indirect test with negative controls and path evidence |
| 5 — monitor/InfoBot | Approved BLE context and three commands | No replies to duplicate/history/self-generated traffic |
| 6 — battery | Profiles, screen-off, measured current/runtime | Scenario report, bounded memory, no unexplained resets |
| 7 — Pocket Messenger | Reading and canned replies | Explicit destination confirmation and safe cancellation |
| Conditional — welcome | Reliable association event | Do not implement without a demonstrated event |

## Current progress

The eleven-screen firmware, persistent identity/brightness, signed public mesh, automatic Noise, bounded GATT, public message reader, and authenticated InfoBot are implemented. Native tests include official codec/signature fixtures, independent Android Noise vectors, CryptoKit verification, hostile input, mesh processing, UI bounds/navigation and transport startup/fragmentation. Android hardware has verified discovery/signatures/Noise; the owner confirmed visibility.

The active gate is physical acceptance: iOS message exchange, controlled two-phone relay, bot behavior on the actual apps, screen readability/flicker, reconnect/background behavior, and endurance. See the [checklist](13-device-checklist.md). Passing software tests is not a substitute.

## Recorded stack decision

Use the existing Plus2 Arduino/NimBLE stack with a bounded local protocol model. Cryptography uses pinned Monocypher 4.0.2 and ESP32 mbedTLS SHA-256; host tests cross-check official Swift/Android behavior. The exact SHAs and limitations are recorded in the [wire contract](12-wire-contract.md). No C3/C6/S3 firmware port is claimed.

Private groups/media, Nostr, general encrypted routing, synchronization and Pocket Messenger composition remain separate future milestones. Welcome automation remains blocked by the lack of a reliable entry event.

## Future definition of done

Record builds and compatibility results, run the test matrix, measure runtime/heap/queues, review licenses, document limitations, and make the bot locally disableable. Two nearby phones receiving the same text is not proof that the Stick relayed it.
