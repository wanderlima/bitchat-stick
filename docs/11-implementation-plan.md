# Implementation plan by screen and complexity

Updated: **2026-11-10**. English (en-US) firmware v0.2.0. Source implementation is complete for the current eleven-screen public-BLE scope. Physical acceptance is intentionally separate: use the [device checklist](13-device-checklist.md). Future product ideas are not represented as implemented.

## Screen order — lowest to highest complexity

| Order | Screen | Implemented behavior | Remaining physical acceptance |
|---|---|---|---|
| 1 | Start | Shared controls, project name, A opens Home | Battery boot, readability, wake suppression |
| 2 | About | Wander Lima, open-source credit, repository QR | Camera scan at practical brightness |
| 3 | Settings | Four destinations and shared navigation | Return/focus behavior on the device |
| 4 | Brightness | 5/25/50/75/100%, explicit apply, NVS persistence/error handling | All levels on USB/battery; full power cycle |
| 5 | Battery | Voltage, approximate percentage, unknown/low states | Compare USB/battery; measure actual endurance |
| 6 | Diagnostics | BLE reports/drops, heap, uptime, mesh accept/reject and clock | Long-run counters and memory trend |
| 7 | Home | Candidate/peer totals, relay state, live header | State transitions, no erase/redraw flicker |
| 8 | Nearby | Filtered candidates, signed peers, Noise authentication, stable candidate selection, IDs/details | Android/iOS foreground/background/reconnect behavior |
| 9 | Messages | Signed public and authenticated private input, bounded history, paging and text scrolling | Real text, Unicode substitution, truncation, reconnect |
| 10 | Channels | Public BLE #bluetooth and actual transport availability | Offline context and controlled indirect forwarding |
| 11 | InfoBot | Authenticated fresh commands, reply limits, ON/OFF control, welcomes OFF | Cross-platform replies, cooldown, no reply loops |

## Completed engineering groups

1. Persist and validate brightness settings; preserve Nearby candidate selection.
2. Exercise actual scanner callbacks and bounded filtering/recovery.
3. Implement bounded v1/v2 framing and fragment reassembly using official serializer fixtures.
4. Cross-check crypto primitives and Noise XX against independent vectors.
5. Match signed/compressed packet canonicalization and verify local signatures independently with CryptoKit.
6. Persist a stable local identity and fail closed on corrupt/unwritable storage.
7. Implement signed peer discovery, public messages, deduplication, TTL relay, Noise peer-state binding and rate-limited InfoBot.
8. Add two-link peripheral GATT with queued callbacks, paced MTU fragmentation and RTC time.
9. Fix the hardware-reproduced startup assertion; add idle-GAP, overflow and fragment transport regressions.
10. Automatically initiate Noise after discovery; verify Android announcement and handshake on hardware.
11. Connect all current screens to real state, add the bot switch, buffer display updates, and document acceptance limits.

Each implementation group is committed separately; no push is part of this work. Native success and upload verification do not turn the remaining physical checks into passes.

## Future scope — deliberately excluded from this version

- **M:** final logo asset; persistent bot preference; explicit local identity reset; build-information expansion.
- **M–L:** charging/source indication where supported, battery calibration/hysteresis and measured autonomy.
- **L:** Pocket Messenger composition and confirmed canned replies; persistent history only after an explicit retention decision.
- **L–XL:** private-message sending, media/groups, protected-channel participation, gossip synchronization, general encrypted routing and central-role connection scheduling.
- **XL:** Nostr/internet contexts, OTA, persistent/out-of-band peer trust and secure-storage provisioning.
- **Conditional:** welcomes only after an authoritative, tested entry signal exists. Reappearance, a BLE advert or a reconnect is insufficient.
