# Test plan — two phones and one Plus2

**Protocol/radio matrix: not executed by this project.** Native software tests and successful compilation are separate evidence. The owner has reported using the initial device UI; this does not establish indirect forwarding or messaging compatibility.

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
