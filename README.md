# Bitchat Stick

Firmware **v1.0.00** · Default language: **English (en-US)**.

Modular BLE Bitchat firmware for ESP32. The initial supported profile is the M5StickC Plus2, with the Mini JoyC HAT as an optional peripheral of that profile. The firmware implements signed discovery and public chat, automatic identity-bound Noise sessions, bounded public-packet relay, and signed-public/private InfoBot commands. Other Bitchat users see the device as **Bitchat Stick** in the local `#bluetooth` context. Other ESP32 boards are not supported yet, and the Plus2 binary is not a universal image.

Android 2.2.0 has verified the Stick's announcement and completed a real Noise handshake. The owner also confirmed network visibility, corrected JoyC navigation and successful use of the installed message-sending flow on 2026-11-11, and later confirmed that Bitchat Stick works with the iPhone app. On 2026-11-17 the owner confirmed InfoBot replies and correct delivered/read checks on Android and iPhone. Indirect two-phone relay, background behavior and battery endurance still require the [device acceptance checklist](docs/13-device-checklist.md). Compilation and native tests alone do not establish interoperability.

## Build and upload

From the project root, with PlatformIO Core on your PATH:

```sh
pio run -e m5stickc-plus2
pio device list
```

Select the actual Plus2 serial port and replace the example below. **Uploading replaces the firmware currently on the device.**

```sh
STICK_PORT='/dev/cu.usbserial-REPLACE_ME'
pio run -e m5stickc-plus2 -t upload --upload-port "$STICK_PORT"
pio device monitor -p "$STICK_PORT" -b 115200
```

Exit the monitor with `Ctrl+C` before uploading again. Deployment currently means USB flashing; there is no OTA update path. See the [complete build and deployment guide](docs/10-build-deploy.md) for setup, tests, artifacts, troubleshooting, and validation.

## Controls and interface

- **A:** confirm/open the selected item.
- **B:** move up.
- **C:** move down. On the Plus2 profile, C is the power button.
- **Hold B for 700 ms:** go back; from Home, return to Start.
- **Hold C for 6 seconds:** retain the power-off behavior. A long C press never becomes a down action on release.

The optional **M5Stack Mini JoyC HAT (U156)** also navigates the menus: left goes back, right confirms, and up/down move and repeat every 1.5 seconds when held past the activation threshold. Left/right require recentering before another action. The physical buttons remain available. Settings → JoyC HAT adds optional Click confirmation and LED pulse, both disabled by default. See the [JoyC controls](docs/04-screens.md#optional-mini-joyc-hat) and [device checks](docs/13-device-checklist.md#1a-mini-joyc-hat).

The splash screen shows the project name and controls, aligned to the current Figma design. Press A to enter Home. Navigation instructions are shown only on the splash screen, freeing space on the other screens. The first press after the screen turns off only wakes it.

Settings → Sound offers independent checkboxes for interaction beeps, received DMs and #bluetooth messages. Choices persist across restart.

Settings offers 5%, 25%, 50%, 75%, and 100% brightness, with 50% as the default. About credits **Wander Lima** and includes a real QR code for [the project repository](https://github.com/wanderlima/bitchat-stick). A radio mark shows the applied brightness; the cream highlight shows the focused option.

The interface uses FreeMono Regular/Bold, warm cream `#EBDCBA` instead of pure white, an outlined BLE header, high-contrast selection, grouped panels, and dividers between menu items and information groups. Nearby retains a dark-green peer card and a bright-green left border. The [screen previews](docs/screens/README.md) illustrate the current direction; they are software renders of all seventeen screens, not photographs or hardware captures.

## Live features and remaining scope

- Home, Nearby, Channels, Messages, InfoBot, Settings, Brightness, Battery, About, and Diagnostics use real firmware state.
- Nearby distinguishes service-advertisement candidates, signed peers, and Noise-authenticated peers. A signed name is not an out-of-band verified human identity.
- Channels separates **#bluetooth** (the shared public BLE `#bluetooth` context) and **Direct DMs** (private messages addressed to this Stick). It does not create a new channel. Messages reads signed public text and authenticated DMs, with explicit BLE/DM labels. It keeps twelve RAM-only entries, with message paging and text scrolling; at most 192 bytes are rendered per entry, with an explicit truncation mark.
- Signed public announcements and text are verified, deduplicated, and eligible for TTL-limited forwarding without echoing the ingress link. Queue acceptance is not a delivery acknowledgement.
- InfoBot supports `!info`, `!rules`, `!help`, and `!about`, plus `!regras`/`!ajuda` aliases. `!info` answers `InfoBot: Bitchat Stick | ESP32 | #bluetooth local relay and private text DMs.` Signed public commands receive public replies; authenticated DMs receive private replies. A three-second peer limit and one-second global limit prevent reply floods. The InfoBot screen shows its latest reply/block status. See [usage and troubleshooting](docs/15-infobot.md). The on-device `Bot: ON/OFF` switch applies immediately for the current boot.
- Public channel-join greetings remain OFF because discovery/reconnection does not reliably prove channel entry. With Bot ON, the first fresh authenticated DM from each person receives a private introduction explaining !help/!about, once per boot for up to 32 identities.
- An envelope in the header indicates unread incoming messages. Home and each inbox show unread counts; opening a DM sends an encrypted read receipt. New arrivals preserve the current reading position. Unread state is RAM-only.
- Channels → #bluetooth offers Read chat and Write msg; public drafts show the channel explicitly and use signed public sending. Replying to a user remains private.
- Nearby users and Channels messages open a private composer: 32 scrolling Short msg presets, a QWERTY keyboard, or Welcome msg using the shared introduction, then Send/Cancel. Drafts are bounded to 192 ASCII characters. Sending requires an authenticated live session; Queued, Delivered and Read are distinct states. See the [composer and device checklist](docs/14-direct-message-composer.md).
- Drawing uses a 135 × 240 offscreen framebuffer before updating the display, avoiding the visible erase/redraw cycle.

Private media, private groups, Nostr, gossip/history synchronization, arbitrary routed Noise forwarding, OTA, persistent chat history remain future scope. See the [wire contract and security limits](docs/12-wire-contract.md) and [current implementation plan](docs/11-implementation-plan.md).

The official project distinguishes the offline BLE `#bluetooth` context from geographic channels carried over Nostr and the internet. Do not assume arbitrary channels, passwords, or historical `/j` and `/pass` commands work in installed versions. See the [official channel description](https://github.com/permissionlesstech/bitchat#channel-types).

## Project website

The one-page website lives in [`website/`](website/README.md), alongside the firmware. It uses Astro, Tailwind CSS and GSAP, with all application source in `website/src/`. See the [website guide](website/README.md) for Node requirements, local preview, browser checks and GitHub Pages setup. Website commands do not flash the device.

## Documentation

- [Changelog](CHANGELOG.md)
- [Requirements and scope](docs/01-requirements.md)
- [Architecture](docs/02-architecture.md)
- [Features and bot behavior](docs/03-features.md)
- [Screens, wireframes, typography, and controls](docs/04-screens.md)
- [Evidence-driven roadmap](docs/05-roadmap.md)
- [Two-phone test plan](docs/06-testing.md)
- [Protocol compatibility investigation](docs/07-compatibility.md)
- [Limitations and risks](docs/08-limitations-risks.md)
- [Implementation status and getting started](docs/09-getting-started.md)
- [Build and USB deployment](docs/10-build-deploy.md)
- [Implementation plan by screen and complexity](docs/11-implementation-plan.md)
- [Wire contract and interoperability evidence](docs/12-wire-contract.md)
- [Device acceptance checklist](docs/13-device-checklist.md)
- [Private composer and validation checklist](docs/14-direct-message-composer.md)
- [InfoBot commands and troubleshooting](docs/15-infobot.md)
- [Multi-board and headless-relay implementation plan](docs/16-multi-board-plan.md)
- [Adding a display](docs/17-adding-a-display.md)
- [Adding a board](docs/18-adding-a-board.md)
- [ADR-001: shared core, board profiles and optional UI](docs/decisions/001-multi-board-core-and-optional-ui.md)

## Hardware and evidence

Initial hardware: M5StickC Plus2 with ESP32-PICO-V3-02, 135 × 240 display, and nominal 200 mAh battery. The Mini JoyC HAT is optional on that profile. Refer to the [M5Stack hardware documentation](https://docs.m5stack.com/en/core/StickC-Plus2). Binaries built for ESP32-C3/C6/S3 are not interchangeable with this target.

“Documented upstream” means described by the cited source. “Third-party claim” is not an independent test. “Proposed” means future work. “Unverified” requires an experiment. The Android discovery/signature/Noise path has hardware evidence. Full messaging compatibility, controlled indirect relay, range, and battery runtime have not yet been established.
