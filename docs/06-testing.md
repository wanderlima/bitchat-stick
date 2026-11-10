# Test plan — two phones and one Plus2

**Full two-phone acceptance matrix: pending. Android discovery, signature verification and Noise establishment have hardware evidence.** Native software tests and successful compilation are separate evidence. The owner has reported using the initial device UI; this does not establish indirect forwarding or messaging compatibility.

## Preparation

Use phones A and B with Bitchat installed; prefer one Android and one iOS when available. If both use one platform, record that coverage gap and repeat cross-platform later. Record model, OS, app version/build, source, corresponding SHA if known, BLE permissions, battery optimization, and foreground/background state. For the Stick, record hardware revision, firmware SHA/snapshot, SDK/libraries, BLE settings, and power source.

Turn off Wi-Fi and cellular data on both phones, keep Bluetooth enabled, verify there is no Nostr path or other relay, and use unique per-trial message nonces. Airplane mode may require re-enabling BLE. Keep identities stable during a trial. Do not erase unrelated user data. Logs should contain monotonic time, direction, and permitted packet identifiers/digests, not private keys or private message content.

## Test matrix

| ID | Procedure | Proposed evidence/acceptance |
|---|---|---|
| T01 | Scan with apps foreground/background/closed and permissions denied | Honest observation states, no complete-presence claim |
| T02 | Connect A and B individually to the Stick | Service, MTU, write/notify, and state evidence |
| T03 | Messages A↔Stick and B↔Stick | 20 per direction without corruption; version/type recorded |
| T04 | A→Stick→B and reverse | Controlled indirect test below |
| T05 | Duplicates, replay, terminal TTL | No loops; counted and explained drops |
| T06 | Lengths around MTU, accents, emoji | Supported reassembly or safe rejection; valid text rendering |
| T07 | Disconnect/reconnect and peer restart | Bounded retries without reply floods |
| T08 | `!info`, `!rules`, `!help`, ordinary text, command arguments | One correct authorized reply; other input ignored |
| T09 | Repeat commands 20 times and replay history | Rate limits; no history-triggered replies |
| T10 | Unknown radios, duplicate nicknames, changing MAC | No false association or welcome events |
| T11 | Geographic channel without internet | Do not credit a Nostr delivery to BLE |
| T12 | Display off, USB power, low battery | Profile-consistent radio and honest power status |
| T13 | 60 minutes of controlled traffic and queue saturation | No crash; report minimum heap, drops, latency |
| T14 | Malformed payloads, bad signatures, incomplete fragments | Safe rejection and memory recovery after timeout |
| T15 | Private A↔B traffic through a relay | No plaintext exposure to the bot; only claim supported formats |

Tests for unimplemented features remain pending. Adversarial input belongs in an owned test environment with synthetic data.

## Prove the indirect path with two phones

1. With the Stick off, separate A and B until direct BLE delivery fails, while each can reach the planned intermediary location. Check that no internet or other relays bridge the gap.
2. Send 20 new nonces each way; wait a fixed 60-second window per batch. If delivery occurs, the negative control failed and the trial is inconclusive.
3. Enable the Stick relay at the midpoint. Confirm both links and log matching RX/TX packet identity/digests and TTL in each direction. Do not rely only on phone screenshots.
4. Send 20 new nonces each way. Initial project target: at least 19/20 within 30 seconds, no visible duplicates; report p50/p95 latency and losses. These are test targets, not protocol guarantees.
5. Disable only the Stick relay and repeat the negative control using new nonces. Separate delayed courier/history synchronization from live delivery.
6. Repeat three times. If the direct phone link cannot be isolated, report “Reception with Stick present; path unproven.”

Two phones test one intermediary, not a large/dense mesh. More hops require additional nodes.

## Welcome messages and power

Only test welcomes if protocol investigation finds a reliable association event. Include exit/rejoin, reconnect, reboot, duplicates, replay, and nearby nonmembers. No false-positive welcome is acceptable. Without such an event, report unsupported/unverified and retain command-driven information as the fallback.

For battery life, measure three cycles per profile: scanner, future two-link relay, and future participant/bot. Record brightness, scan interval, traffic, temperature, starting/ending voltage, and shutdown time. A USB meter characterizes external supply, not internal-cell discharge by itself. Do not extrapolate hours solely from a percentage estimate. Set runtime targets after measurement.

## UI regression checklist

- Splash: Figma-aligned name and controls and English controls; A enters Home.
- Every screen: A confirms, B moves up, C moves down, holding B returns consistently.
- B-long never also moves up; C-long never moves down after release.
- Wake with A/B/C consumes that interaction without hidden navigation.
- No navigation footers on regular screens; grouped cards use the freed area.
- Nearby retains its green background/left border; selection stays high contrast.
- Check physical font size, readability, clipping, and battery-only boot.

## Native tests

The foundation has host tests for observation-table bounds/expiry and shared input semantics, including clock wrap and wake suppression. See [test commands](09-getting-started.md#native-tests). They do not replace radio or physical UI tests.

## Report template

Date; operator; builds/SHAs; hardware; topology; radios; steps; expected/observed; counts/latency; logs; passed/failed/inconclusive/not executed; limitations; next action. Avoid storing real people's identifiers unless necessary.

## Executed device checks — 2026-11-10

- Plus2, USB-powered: the dedicated `settings-selftest` firmware used the production Settings adapter, wrote brightness index 4 (100%) from original index 2 (50%), restarted the ESP32, read back index 4, then restored and read back index 2. UART repeatedly reported `SETTINGS_SELFTEST PASS original=2 persisted=4 restored=2`.
- The normal `m5stickc-plus2` firmware was reinstalled immediately afterward; esptool verified the written flash hash. At that earlier checkpoint this included persistent brightness and stable Nearby selection; protocol/GATT integration was added later in the same session.
- This verifies NVS across a software/device restart, not a battery-only power-cycle or calibrated brightness/voltage/autonomy. Physical readability and camera QR reading remain manual checks.
- Available app versions were queried: Android 2.2.0/build 42, Android 16 on SM-A366E; iOS app 1.8.0/build 1 on iPhone 15 Pro. No authenticated phone↔Stick message exchange has passed yet. See the [wire contract](12-wire-contract.md).

To repeat the controlled NVS check, use the PlatformIO Python environment (pyserial required), an explicit Plus2 port and the PlatformIO executable:

```sh
python tools/test-settings-device.py --port /dev/cu.usbserial-REPLACE_ME --pio /path/to/pio
```

This temporarily installs a dedicated test firmware, preserves/restores the original brightness, and always attempts to reinstall production firmware in a `finally` block. Do not disconnect the device during the operation; if interrupted forcibly, reinstall `m5stickc-plus2` manually. The test only clears its own `stick-test-v1` namespace.

## 2026-11-10 integrated firmware observation

The first integrated upload reproduced a NimBLE GATT-registration assertion while scanning, causing repeated reboots. The corrected startup pauses discovery before registering services; the scanner resumes on its normal loop schedule. Native transport tests now assert this ordering. A subsequent USB observation showed stable heap around 145 KB before framebuffer changes, one subscribed Android link, a signed peer, then a Noise-authenticated peer with zero protocol rejections. Android 2.2.0 independently logged successful signature verification and responder handshake establishment. The owner reported seeing Bitchat Stick in the network. These observations do not establish indirect relay or message delivery.

The framebuffer revision was built/uploaded after the owner reported lighter redraw flicker; physical visual acceptance remains with the owner. Use the [device checklist](13-device-checklist.md) to record the final revision, both phone versions and each actual result.

### Final installed source snapshot

- Firmware source commit: `1872efb` (documentation/export commits afterward do not change the installed firmware).
- Application image SHA-256: `edfed7e93f99a50a055d1e58217151e7e2fb66134d4f039cfb406ee2ce8b3a8b`.
- Target build: 97,336 bytes static RAM and 974,785 bytes application flash; the 64,800-byte display canvas is allocated dynamically in PSRAM.
- Final observed runtime samples: one subscribed link, one authenticated peer, active RTC, zero rejected packets in the captured interval and about 145,000 bytes free internal heap. These are short observations, not endurance results.
- Fourteen native suites and the independent CryptoKit signature check passed after private-text/receipt integration. JPEG previews were regenerated from the real UI with simulated data and visually inspected.
- The owner's initial missing message was identified as an Android DM sent before private-text display support. The DM reader/receipt update was installed and a new trial requested; receipt/display of that retry has not yet been confirmed.
