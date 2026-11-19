# Known issues

Items here are open limits or recorded risks. They are not a backlog of promised fixes. Device procedures live in [the acceptance checklist](device-checklist.md).

| Risk | Effect | What has to be true before treating it as solved |
| --- | --- | --- |
| Mobile protocol changes | Discovery works while messaging fails | Pinned codecs and a tested phone matrix |
| Another ESP32 family binary on this Plus2 | Build, BLE, or power failure | A separate profile with its own hardware pass |
| Two phones still linked to each other | A false relay demonstration | Negative controls and a path that excludes the direct link |
| Nostr treated as the mesh | An offline-channel claim the firmware does not implement | The UI shows the BLE transport |
| Presence treated as joining a channel | Repeated welcomes | A reliable join event, which this firmware does not have |
| Private ciphertext | InfoBot cannot read it | Public relay stays opaque; private replies need an authenticated session |
| BLE load | Loss, latency, dropped links | Bounded queues and a counted drop |
| Fragment or memory pressure | Crash or denial of service | Fixed pools, expiry, and a heap counter |
| Replay, spoofing, or duplicate names | Wrong replies and misleading counts | Signatures, identity binding, and per-identity limits |
| Logs and history | Privacy exposure | Counters only; RAM history; no key logging |
| Display and radio load | Short battery life | A measured screen-on, screen-off, and relay profile |
| Plus2 power handling | Unexpected reset | The Plus2 HOLD and button C behavior, not another Stick model's PMIC |
| Dependency licenses | Incorrect redistribution | Review at the imported commit |
| Long or non-ASCII text | Clipped or substituted glyphs | Bounded rendering and a physical readability check |

## Not yet established on hardware

Wander Lima's 2026-11-17 report covers InfoBot replies and delivered/read checks on Android and iPhone after the Plus2 profile was flashed. These checks are still open:

- Controlled indirect relay between two phones that are not directly connected to each other, including negative controls.
- Background behavior of the phone apps.
- Radio range.
- Battery-only runtime and endurance.
- Individual disconnect, retry, and reconnect cases beyond the reported delivered/read checks.
- Click, LED, and sound behavior as a recorded device pass.
- Every keyboard key and the draft length limit on the physical panel.
- Landscape rotation against physical buttons A and B.
- JoyC axis directions after that rotation. Navigation confirmed before the landscape UI does not close this check.
- A camera scan of the About QR, which encodes `https://wanderlima.github.io/bitchat-stick/`.
- Saving Date & time to the RTC on the device. Invalid dates are rejected in host tests. A successful panel save is not recorded.

Upload success, a green host-test run, and a website build do not close those items.

## Protocol and security limits

- Public `#bluetooth` text is visible to nearby participants. Noise protects private DMs, not that public context.
- Identity seeds are in unencrypted NVS. This project does not provision secure boot or flash encryption.
- Peer pins are RAM-only. A signed name is not proof of a human identity.
- Two GATT links, twelve peer records, twelve history entries, and bounded queues drop or overwrite under load.
- The relay path forwards verified public announcements and text. It does not relay private media, private groups, or arbitrary Noise payloads.
- BLE does not guarantee delivery, a fixed range, anonymity, or availability. The device is not invisible or untraceable.
- The RTC must be in UTC. The build inserts a time floor when the clock is unset. A stale binary with a dead RTC can fail freshness checks.
- Battery percentage is an estimate. Charging state and low-voltage policy are not calibrated.

## Product scope that is absent

Private media, private groups, Nostr, geographic channels, history sync, OTA, and persistent chat are not implemented. Official Bitchat channel types that depend on the internet are outside this offline BLE context. Historical `/j` and `/pass` commands are not a feature of this firmware.

## Firmware portability

Only the Plus2 profile has source and a hardware path. The board id in PlatformIO is `m5stick-c` because that is the closest board definition; runtime still checks for Plus2. Do not treat a successful compile, or a C3/C6/S3 binary, as support for another board. Headless relay is an accepted direction in [ADR-0001](adr/adr-0001-shared-core-and-board-profiles.md) and is not a second image in the tree.

[The wire contract](wire-contract.md) is dated 2026-11-10 and still lists release gates that later firmware and the 2026-11-17 device report closed in part. Prefer this page, `CHANGELOG.md`, and [the device checklist](device-checklist.md) when those sections disagree. The framing rules and fixture SHAs in that wire document remain the codec reference.

## Website

- `website/src/tools/audit.mjs` allowlists `GHSA-fc5q-39xc-vjpf` (picomatch, every published release through 4.0.7). The audit script's comment says `npm audit fix --force` would downgrade Astro and still depend on picomatch. The allowlist entry must be removed when npm stops reporting that advisory.
- `robots.txt` is served under `/bitchat-stick/` on a GitHub project site. Crawlers usually read the domain-root robots file, so this file does not replace the account site's root policy.
- Hero routes and the hardware walkthrough are illustrations. They are not relay evidence.
- The configured Pages URL is a target. Publishing still depends on the GitHub Actions source setting and a successful `main` deploy.

## Related Documentation

- [Features](features.md)
- [Environments](environments.md)
- [Device checklist](device-checklist.md)
- [Changelog summary](changelog-summary.md)
