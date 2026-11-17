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

The seventeen-screen participant, persistent identity/brightness, signed public mesh, automatic Noise, bounded GATT, public and private messaging, and authenticated InfoBot are implemented. The firmware is a shared ESP32 core plus a compile-time Plus2 profile. Native tests include official codec/signature fixtures, independent Android Noise vectors, CryptoKit verification, hostile input, mesh processing, UI bounds/navigation, the profile contract and transport startup/fragmentation. Android hardware has verified discovery/signatures/Noise. On 2026-11-17 the owner confirmed InfoBot replies and correct delivered/read checks on Android and iPhone.

The active gate is physical acceptance of controlled two-phone relay, screen readability/flicker, reconnect/background behavior and endurance. See the [checklist](device-checklist.md). Passing software tests is not a substitute.

## Recorded stack decision

Use the existing Plus2 Arduino/NimBLE stack, behind a board profile, with a bounded local protocol model. Cryptography uses pinned Monocypher 4.0.2 and ESP32 mbedTLS SHA-256; host tests cross-check official Swift/Android behavior. The exact SHAs and limitations are recorded in the [wire contract](wire-contract.md). No C3/C6/S3 firmware port is claimed.

Private groups/media, Nostr, general encrypted routing, synchronization and persistent Pocket Messenger history remain separate future milestones. Welcome automation remains blocked by the lack of a reliable entry event.

## Future definition of done

Record builds and compatibility results, run the test matrix, measure runtime/heap/queues, review licenses, document limitations, and make the bot locally disableable. Two nearby phones receiving the same text is not proof that the Stick relayed it.

The initial Pocket Messenger composer is implemented: canned replies, QWERTY draft entry and explicit destination confirmation. Actual outbound Android/iOS acceptance remains on the [device checklist](direct-message-composer.md).
