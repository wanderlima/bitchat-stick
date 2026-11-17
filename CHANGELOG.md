# Changelog

User-visible changes and validation evidence are recorded here in English (en-US). Entries use the date of the recorded change or report. Unreleased changes do not imply a tagged firmware release.

## Unreleased

### Modular Plus2 profile

- Split the firmware into a shared ESP32 core and a compile-time M5StickC Plus2 profile. Power, RTC, buttons, the optional JoyC HAT and the 135×240 panel stay in that profile. Layout code draws through a small display surface. The core does not name M5StickC or include M5Unified. The Plus2 image is not a universal ESP32 binary.
- `!info` now answers `InfoBot: Bitchat Stick | ESP32 | #bluetooth local relay and private text DMs.` The announced product name remains Bitchat Stick.
- The website, README and project docs describe that modular boundary. Guides for adding a board or a display are in `docs/17-adding-a-display.md` and `docs/18-adding-a-board.md`.
- On 2026-11-17 the owner confirmed InfoBot replies and correct delivered/read checks on Android and iPhone after this profile was flashed. Controlled relay, reconnect, range and battery checks remain open.

### Website refinement

- Added a once-per-load stepped hero entrance (520ms), with the network loop starting afterward, static reduced-motion behavior and immediate visibility when a hero link receives keyboard focus.

- Recorded the owner’s iPhone operation confirmation in website copy, structured facts and linked compatibility/evidence notes. Receipt checks were still open in that website pass; the 2026-11-17 device report above later confirmed InfoBot replies and the delivered/read checks. Relay, reconnect, range and battery checks stayed open.
- Refined the infinite hero loop to highlight a continuous active route and both participating devices. Removed hero play/pause; added a site-wide reduced-motion preference and keyboard shortcut.
- Applied stepped title reveals to all section headings, batched content entrances and desktop GSAP smooth scrolling with accessible anchor focus and native touch/reduced-motion fallbacks.
- Reused the hero device shell for the side-by-side physical-controls preview, with real firmware UI renders for menu selections, opened pages and Start.
- Added safe new-tab external links, linked SEO entities, a social preview, machine-readable project facts, and GA4 G-VHFJLDXMVC plus the supplied GTM container. Local previews and browser tests do not send analytics.
- Validation: static build and type checks passed; the refinement passed sixteen Chromium tests in CI mode; the subsequent hero entrance revision passed all seventeen tests, including once-only entrance, desktop/mobile route geometry, repeat, reduced motion, keyboard focus and mocked analytics.

### Added

- Independent Astro/Tailwind/GSAP one-page website in `website/`, implemented from the refined Figma design with local pixel/mono fonts, original vector icons and real firmware UI software renders.
- Connected conceptual hero with send/receive states, pause/resume, offscreen/tab pause and a static reduced-motion alternative. Native FAQ, interactive A/B/C control demonstration, project limitations and documentation links are included.
- Static SEO metadata, sitemap, robots, structured data, production browser acceptance tests and a GitHub Pages workflow. Deployment requires GitHub Pages to use GitHub Actions; this change does not publish a release or alter firmware behavior.

## 1.0.00 — 2026-11-11

### Added

- Welcome msg in the composer method picker, using the shared introduction text and explicit confirmation for private or public destinations.
- Read chat/Write msg actions in #bluetooth, using the shared composer with explicit public destination confirmation.
- First-DM private introduction explaining !help/!about, controlled by Bot ON/OFF and remembered by authenticated peer ID for the current boot (32 identities, no eviction). Public traffic/reconnections never trigger it; queue failures can retry on the next new DM.
- Private message composition from identified Nearby users and Channels messages, using a shared four-screen flow: Reply, Short msgs, Keyboard and Confirm.
- Thirty-two scrolling ready-made English replies and a compact QWERTY editor with lower/uppercase letters, numbers, punctuation, space and delete. Drafts are limited to 192 ASCII characters and kept in RAM.
- Authenticated Noise DM sending with random message IDs, explicit recipient confirmation, Send/Cancel, draft retention after send failure and protection against repeat submission after queue acceptance.
- Distinct Queued, Delivered and Read states, with incoming receipts matched to both message ID and peer ID.
- Mini JoyC HAT navigation: left returns, right confirms, up/down navigate beyond ±80; horizontal gestures require recentering to ±10 and vertical actions repeat every 1.5 seconds.
- Independent persistent checkboxes for interaction beeps, DM alerts, #bluetooth alerts, optional joystick click and LED pulse.
- Twenty software screen/state JPEGs, a seventeen-screen overview and composer/device checklists. Figma includes separate public/private sending and receipt/recovery state artboards.

### Changed

- Renamed the navigation page from Inbox to Channels; #bluetooth and Direct DMs retain their existing behavior.
- Public inbox is named #bluetooth again and distinguishes public chat from Direct DMs, with unread counts and a global header envelope.
- Settings places About last and retains five brightness presets: 5%, 25%, 50%, 75% and 100%.
- M5GFX and M5Unified are pinned to 0.2.36 and 0.2.8 respectively, aligned with the supplied working HAT reference. RTC writes are verified through readback for that API.

### Fixed

- Expanded the private-session pool from two to twelve so a third discovered identity is not excluded. Explicit DM sends initiate missing authentication and retain the draft; orphaned sessions are released.
- DM send failures show a concrete reason instead of an ambiguous yellow status. Added counter-only BLE/session/clock diagnostics.
- InfoBot now answers signed public commands without requiring a private Noise session and answers authenticated DM commands privately. Added !about with the project link, case/whitespace handling, three-second peer cooldown and visible reply/block status.
- Removed the misleading noninteractive Welcome: OFF footer; automatic public channel-join greetings remain disabled.
- Back from Home returns to Start. Added movement/direction/help/SOS presets with a sliding list and scrollbar.
- HAT register reads request one byte per axis and use STOP-separated M5Unified external-I2C transactions, with Mini/classic discovery and recovery. Optional click reads stay disabled unless explicitly enabled.
- Corrected both joystick directions for the physical Plus2 orientation. The owner confirmed HAT readiness after a full power cycle/remount and then confirmed correct navigation.
- Outgoing DMs never generate local read receipts; a late delivered receipt cannot downgrade an already read message.

### Validation — 2026-11-11

- The composer revision passed eighteen native suites; the subsequent InfoBot/navigation revision passed nineteen suites, including the reproduced public-command regression. Independent CryptoKit signature verification passed. The Plus2 build and USB upload completed with flash hash verification.
- The owner confirmed DM recovery and new public #bluetooth writing after the session-pool correction. Manual Welcome msg subsequently passed nineteen native suites, the Plus2 build/upload and Figma visual checks; its actual phone reception is still unchecked.
- The owner reported that the installed message-sending flow worked well. The report did not identify a phone platform, entry path, input method or delivered/read state; these are not inferred as separate passes.
- Earlier owner reports confirmed Android discovery, incoming DM display and HAT navigation. See [recorded tests](docs/06-testing.md) and the [device checklist](docs/13-device-checklist.md).

### Remaining scope

- Separate Android/iOS acceptance, delivered/read presentation, disconnect/retry cases, all keyboard keys/limits, click/LED/sound behavior, controlled indirect relay and endurance still need individually recorded device checks.
- Private media/groups, Nostr, history synchronization, arbitrary routed Noise forwarding, OTA and persistent chat history are not implemented. Public channel-join greetings remain disabled without reliable entry detection; private first-contact introductions and manually confirmed Welcome msg are implemented.
