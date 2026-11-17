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

### Earlier DM-reader snapshot (superseded)

- Firmware source commit: `1872efb` (documentation/export commits afterward do not change the installed firmware).
- Application image SHA-256: `edfed7e93f99a50a055d1e58217151e7e2fb66134d4f039cfb406ee2ce8b3a8b`.
- Target build: 97,336 bytes static RAM and 974,785 bytes application flash; the 64,800-byte display canvas is allocated dynamically in PSRAM.
- Final observed runtime samples: one subscribed link, one authenticated peer, active RTC, zero rejected packets in the captured interval and about 145,000 bytes free internal heap. These are short observations, not endurance results.
- Fourteen native suites and the independent CryptoKit signature check passed after private-text/receipt integration. JPEG previews were regenerated from the real UI with simulated data and visually inspected.
- The owner's initial missing message was identified as an Android DM sent before private-text display support. The DM reader/receipt update was installed; the owner subsequently confirmed Android delivery status and reading the DM on the Stick. Remote read status was not yet implemented at that checkpoint.

### Current installed Inbox / Mesh / read-receipt revision

- Firmware source commit: `ca9f68b`. Later documentation/export changes do not alter the installed firmware.
- Application image SHA-256: `61b521444822e3ee3838038729f30d1cfb8eb689718ea95c96bf3d7887663ea5`.
- Build: 98,296 bytes static RAM; 977,057 bytes application flash. Upload completed with hash verification.
- Final short USB observation: `links=2 peers=2 auth=2 accepted=13 rejected=0 queued=16 relay=7 drops=3 heap=144040 clock=RTC`. This confirms simultaneous subscribed/authenticated peers and source-level forwarding activity, not a controlled indirect-delivery test. Drops include attempts without a usable destination; they are not equivalent to protocol rejections.
- Native suites passed after inbox filtering, unread tracking, stable reader focus and encrypted read receipts. Tests decrypt the exact read-receipt payload, verify no read receipt before opening, and retry a failed enqueue.
- Owner confirmed Android delivery and DM reading before this revision. The new Android/iOS delivered-to-read UI transition remains a manual acceptance check.

## Mini JoyC and feedback update — 2026-11-11

The owner reported that attaching the HAT froze navigation and removing it immediately restored the menu. The original driver incorrectly requested two bytes from register 0x20. The manufacturer's STM32 source supplies exactly one byte for 0x20 (X), and one separate byte for 0x21 (Y). The initial recovery revision performed separate queries through a software I2C port with bounded clock-stretch handling and error recovery. The later reference integration below replaces that port with M5Unified Ex_I2C. The owner confirmed the menu remained responsive after the initial recovery revision. HAT movement, sound and LED behavior still require the physical checks above.

Host coverage includes the one-byte register contract, disabled click reads, active-low button decoding, release-to-arm/debounce, square checkbox navigation and persistence, distinct incoming-message counters, double confirmation tones and LED pulse completion. A stuck press while enabling Click produces no action until a stable release and new press.

The combined feedback revision passed seventeen native suites plus the independent CryptoKit check and the Plus2 build/upload. A short post-upload sample retained an active RTC and stable heap. UART reported `joyc=absent` during that sample, so physical HAT navigation, actual beep audibility and LED operation remain acceptance checks. The corrected register contract is pinned to the vendor source at [main.c](https://github.com/m5stack/M5Unit-MiniJoyC-Internal-FW/blob/main/HAT_MiniJoyC/Core/Src/main.c), which supplies one byte per normalized-axis query and an active-low button.

## HAT reference comparison — 2026-11-11

Settings now places About last. The supplied local `m5stick-nemo-wl` reference uses M5Unified Ex_I2C and STOP-separated register reads, with 0x54/0x38 discovery. The Bitchat driver now follows that interface; its direction mapping was subsequently corrected using owner feedback below. The runtime libraries were pinned to the reference's M5GFX 0.2.36 and M5Unified 0.2.8. RTC setup verifies writes through readback for the older API.

A temporary address/pin diagnostic found no acknowledgement and observed the GPIO26 line low. The probe was removed and normal firmware restored. Seventeen native suites passed, and the target built/uploaded successfully, with RTC and the application loop alive. The captured runtime still showed `joyc=absent`. A full power cycle/remount was requested to check for a stalled accessory. At that diagnostic stage, successful physical HAT navigation was not yet confirmed.

The owner subsequently confirmed Hat ready and working navigation after powering down and remounting the HAT. Both axes were reported reversed. The driver now uses raw X and inverted raw Y, with host assertions covering both signs. The owner subsequently confirmed the corrected directions. Click, LED and sound have not been independently confirmed.

## Private composer — 2026-11-11

The owner confirmed the corrected JoyC directions. Eighteen native suites and independent CryptoKit signature verification passed after the private composer update. Tests decrypt an outgoing maximum-length private TLV, verify recipient addressing, require an authenticated session, preserve history on failed enqueue, correlate delivery/read receipts and prevent read-state downgrade. UI tests cover Nearby and Inbox entry, stable recipient capture, cancel/back, empty Done, every keyboard mode/key, long drafts and all confirmation pages within 135×240.

The target build passed. Software JPEG previews include all seventeen screens. Actual outbound Android/iOS reception and delivery/read presentation remain manual acceptance checks; build/upload and host decryption do not establish those results. See [the composer checklist](14-direct-message-composer.md).

The updated firmware upload completed with flash hash verification (981,952 bytes). A short USB read produced no telemetry; no post-upload session/delivery result is inferred from that read.

## Owner acceptance update — 2026-11-11

On 2026-11-11, the owner reported that the installed message-sending flow worked well on the device. This confirms the tested flow at owner-observed level; the report did not identify which phone/OS, entry path, input method or receipt state was exercised. Platform-specific and edge-case checks below remain open until individually recorded.

The owner also explicitly confirmed successful JoyC navigation after both axis directions were corrected. Existing eighteen-suite/CryptoKit results and the verified upload remain the latest software checks; this documentation update does not represent another test or flash run.

## InfoBot and navigation correction — 2026-11-11

A new native regression reproduced the original signed-public/no-private-session command being ignored. Public commands now rely on the existing signature verification instead of an additional private-auth gate. Private commands receive only encrypted private replies after the delivery acknowledgement; replaying the same message ID never emits another bot reply. Tests cover !about/project link, aliases, case/whitespace, cooldown, disabled state, stale clocks, invalid signatures and enqueue failure.

UI tests cover Home Back to Start, #bluetooth naming, all four command details plus the bot switch, absence of the former welcome footer, and every one of the 32 presets without text clipping. The selected row remains visible through a sliding five-item viewport and wraps at both ends. Physical acceptance of these new bot paths remains pending; earlier owner confirmation of ordinary outgoing DMs does not establish it.

This revision passed nineteen native suites plus independent CryptoKit verification. The Plus2 build/upload passed with flash hash verification (983,056 bytes). A post-upload USB sample reported `links=2 peers=2 auth=2 bot=0 botState=Waiting rejected=1 drops=7 heap=142188 clock=RTC joyc=present`. It confirms active transport/authentication/HAT detection, not an on-phone bot response; no new command had been observed by the bot in that sample.

## Private introduction — 2026-11-11

Nineteen native suites plus independent CryptoKit verification passed. The private-message test decrypts the first introduction and checks the exact English hint, addressed private frame and delivery-receipt order. It also covers Bot OFF, failed enqueue followed by a retry on a new DM, and suppression for duplicate/subsequent DMs. Public-only bot tests assert zero private introductions. The 32-entry identity memory has no eviction and is independent of peer/session expiry; it resets at reboot. Actual phone display of this new introduction remains an owner acceptance check in [the InfoBot guide](15-infobot.md#first-dm-introduction).

## DM session recovery and public composer — 2026-11-11

The owner reported DM sending/receiving stopped while public chat continued. A yellow Not sent status retained Send/Cancel. Initial telemetry showed three discovered peer identities but only two authenticated peers, matching the old two-session capacity. This is a plausible failure path, not proof that it was the only cause of the owner's exchange. A focused host build using the old two-session header fails the new third-identity handshake assertion; the twelve-session build passes.

The session pool is now bounded to twelve, orphaned sessions are released and explicit DM sends initiate missing authentication while retaining the draft. Detailed send status distinguishes Connecting, expired peers and link/queue failures. Counter-only USB diagnostics report received writes/public/DM totals, transmit queue/notifies, clock and selected rejection reasons. No message bodies or keys are logged.

Nineteen native suites and independent CryptoKit verification passed; the target built and uploaded with hash verification (990,336 bytes). UI tests exercise the new public Read chat/Write msg path, explicit public confirmation, actual public dispatch and cancellation without converting user replies to public traffic. Three additional software-rendered states were inspected. A post-upload observation found the loop/HAT active and heap stable at 137,088 bytes, but no phone was connected during that observation. Actual DM recovery and public reception await the owner's retry.

## Owner confirmation and Channels label — 2026-11-11

In response to the requested DM and #bluetooth Write msg checks, the owner confirmed that both worked correctly. This closes the general recovery/public-writing acceptance for the tested setup; no separate Android/iOS, endurance or all-peer receipt result is inferred. The navigation label was subsequently renamed from Inbox to Channels; public/private destinations and controls are unchanged.

## Manual Welcome msg — 2026-11-11

Nineteen native suites and independent CryptoKit verification passed. UI coverage confirms that selecting Welcome msg copies the exact shared introduction without sending, Back restores the third method selection, Cancel transmits nothing, and an explicitly confirmed public welcome stays public. The Plus2 target compiled successfully; private/public method pickers and welcome confirmation states were also updated and visually reviewed in Figma. Actual phone reception of a manually selected welcome remains a device acceptance check.

## Modular Plus2 profile — 2026-11-17

The native suite passed, including the Plus2 profile contract, clock policy, UI bounds and the ESP32 `!info` sentence. `pio run -e m5stickc-plus2` passed without treating that image as a universal ESP32 binary. The owner then confirmed, on Android and iPhone, that InfoBot replies and that the delivered/read checks are correct. Controlled relay, reconnect, range and battery checks remain open.

## Release 1.0.00 — 2026-11-11

The first annotated release tag is `v1.0.00`. The version label is defined in `include/Version.h` and reused in the boot banner and Diagnostics. Nineteen native suites, independent CryptoKit verification, the Plus2 firmware build and software preview bounds checks passed for this source snapshot. Tagging did not flash another image or close outstanding per-platform, manual Welcome msg, range/relay or endurance checks. Earlier owner-confirmed DM recovery/public sending evidence remains recorded above.
