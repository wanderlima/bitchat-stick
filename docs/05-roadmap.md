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

The scanner/UI foundation exists and has compiled; the owner has used an earlier build on the Plus2. Native model and navigation tests pass. Updated A/B/C controls, splash screen, larger fonts, restored panels, footer removal, and en-US defaults require a new hardware check. BLE message reception/transmission and relay remain unimplemented.

The official service UUID snapshot is recorded, but this is not completion of the full protocol gate. No mobile-build compatibility matrix has passed.

## Pending stack decision for messaging

Compare a minimal implementation, adaptation of an existing relay, and selected components from C3/C6/S3 projects. Evaluate classic-ESP32 support, licensing, protocol freshness, independent iOS/Android tests, memory cost, parser security, and UI integration. Do not choose by advertised feature count. Record the decision and SHAs before importing code.

Additional channels, protected groups, and Nostr are separate decisions. If the selected apps expose only the local BLE context, deliver that context honestly rather than inventing channels through local filtering.

## Future definition of done

Record builds and compatibility results, run the test matrix, measure runtime/heap/queues, review licenses, document limitations, and make the bot locally disableable. Two nearby phones receiving the same text is not proof that the Stick relayed it.
