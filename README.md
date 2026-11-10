# Bitchat Stick — M5StickC Plus2

Initial firmware v0.1.0 · Default language: **English (en-US)**.

A pocket-sized BLE observer, with Bitchat relay and InfoBot participation planned after interoperability is demonstrated. The initial firmware implements passive scanning, a bounded nearby-device list, physical-button navigation, brightness control, and a battery estimate. **It does not receive or send Bitchat messages.** GATT messaging, authenticated identities, Noise sessions, relay, and InfoBot are not implemented.

The owner has reported using the initial firmware on the device. The latest navigation, splash screen, typography, grouping, and language changes still require a new upload and physical verification. Software compilation and native tests do not prove radio interoperability.

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
- **C:** move down. C is the Plus2 power button, exposed as `M5.BtnPWR`.
- **Hold B for 700 ms:** go back; from Home, open Settings.
- **Hold C for 6 seconds:** retain the power-off behavior. A long C press never becomes a down action on release.

The splash screen shows the project name and controls, aligned to the current Figma design. Press A to enter Home. Navigation instructions are shown only on the splash screen, freeing space on the other screens. The first press after the screen turns off only wakes it.

Settings offers 5%, 25%, 50%, 75%, and 100% brightness, with 50% as the default. About credits **Wander Lima** and includes a real QR code for [the project repository](https://github.com/wanderlima/bitchat-stick). A radio mark shows the applied brightness; the cream highlight shows the focused option.

The interface uses FreeMono Regular/Bold, warm cream `#EBDCBA` instead of pure white, an outlined BLE header, high-contrast selection, grouped panels, and dividers between menu items and information groups. Nearby retains a dark-green peer card and a bright-green left border. The [screen previews](docs/screens/README.md) illustrate the current direction; they are software renders of all eleven screens, not photographs or hardware captures.

## Live features and previews

Home, Nearby, the BLE header, battery, diagnostics, navigation and brightness use real device data/controls. Nearby separates observations from the unverified Bitchat candidate status. Channels and Messages show unavailable states. InfoBot commands open local examples labeled **Preview only**; nothing is transmitted. Relay and welcome bot remain OFF. See the [screen behavior matrix](docs/04-screens.md#screens-and-data-sources).

A bounded binary framing codec is also available for protocol development and host tests. It does not yet connect GATT, authenticate signatures or enable messaging. See the [pinned wire contract](docs/12-wire-contract.md).

## Planned progression

1. Observe BLE advertisements and RSSI; distinguish candidates from verified identities.
2. Validate GATT transport and packet formats against pinned mobile builds.
3. Demonstrate phone A → Stick → phone B forwarding without a direct link or internet.
4. Enable message reading and InfoBot only in contexts the Stick can legitimately participate in.
5. Explore Pocket Messenger with paged reading and confirmed canned replies.

The official project distinguishes the offline BLE `#bluetooth` context from geographic channels carried over Nostr and the internet. Do not assume arbitrary channels, passwords, or historical `/j` and `/pass` commands work in installed versions. See the [official channel description](https://github.com/permissionlesstech/bitchat#channel-types).

## Documentation

- [Requirements and scope](docs/01-requirements.md)
- [Proposed architecture](docs/02-architecture.md)
- [Features and bot behavior](docs/03-features.md)
- [Screens, wireframes, typography, and controls](docs/04-screens.md)
- [Evidence-driven roadmap](docs/05-roadmap.md)
- [Two-phone test plan](docs/06-testing.md)
- [Protocol compatibility investigation](docs/07-compatibility.md)
- [Limitations and risks](docs/08-limitations-risks.md)
- [Implementation status and getting started](docs/09-getting-started.md)
- [Build and USB deployment](docs/10-build-deploy.md)
- [Implementation plan by screen and complexity](docs/11-implementation-plan.md)

## Hardware and evidence

Target: M5StickC Plus2 with ESP32-PICO-V3-02, 135 × 240 display, and nominal 200 mAh battery. Refer to the [M5Stack hardware documentation](https://docs.m5stack.com/en/core/StickC-Plus2). Binaries built for ESP32-C3/C6/S3 are not interchangeable with this target.

“Documented upstream” means described by the cited source. “Third-party claim” is not an independent test. “Proposed” means future work. “Unverified” requires an experiment. No Plus2 Bitchat messaging compatibility, range, or battery runtime has been established by this project.
