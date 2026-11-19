# Overview

**Bitchat Stick** is open-source ESP32 firmware, by **Wander Lima**, that participates in the local Bitchat BLE mesh. Other Bitchat users see the device as **Bitchat Stick** in the shared `#bluetooth` context. The source label in `include/Version.h` is **1.1.00**. That label is unreleased work after the `v1.0.00` tag and does not create a new firmware release by itself.

The supported device is the **M5StickC Plus2** (ESP32-PICO-V3-02, 240 × 135 landscape UI, nominal 200 mAh battery). The **M5Stack Mini JoyC HAT (U156)** is an optional peripheral of that profile. The Plus2 binary is not a universal ESP32 image. ESP32-C3, C6, and S3 binaries are not interchangeable with this target, and the original StickC or StickC Plus are not supported.

A one-page project website lives in `website/`. Website commands do not build or flash the Stick.

## What the firmware does

The Plus2 profile implements:

- Signed discovery and public chat on the offline BLE `#bluetooth` context.
- Automatic identity-bound Noise sessions and private text direct messages.
- Bounded forwarding of verified public packets, with TTL reduction and no echo on the ingress link.
- Signed-public and authenticated-private InfoBot commands.
- On-device menus for nearby peers, channels, messages, InfoBot, settings, and a private/public composer.
- A UTC civil clock in the status bar. Date & time stores a valid time on the Plus2 RTC.

Queue acceptance is not a delivery acknowledgement. A signed peer name is not an out-of-band verified human identity. Public `#bluetooth` text is readable to nearby participants.

## Evidence boundary

Android Bitchat 2.2.0 has verified the Stick announcement and completed a real Noise handshake. Wander Lima confirmed network visibility, corrected JoyC navigation, and use of the installed sending flow on 2026-11-11, later confirmed that Bitchat Stick works with the iPhone app, and on 2026-11-17 confirmed InfoBot replies and correct delivered/read checks on Android and iPhone.

Indirect two-phone relay, background behavior, range, and battery endurance still require the device acceptance checklist. Compilation and host tests do not establish interoperability.

## Repository layout

This is a two-part repository, not an npm or pnpm workspace.

| Part | Path | Role |
| --- | --- | --- |
| Firmware | repository root (`platformio.ini`, `src/`, `include/`, `test/`) | PlatformIO/Arduino image for the Plus2 |
| Website | `website/` | Static Astro site deployed by GitHub Actions |
| Documentation | `documentation/` | Handbook, specs, checklists, screen notes, and ADRs |

Every project document lives in `documentation/`. Each topic has one page: setup, architecture, features, the wire contract, screens, tests, and the device checklist are not split across a second folder.

## Related Documentation

- [Tech stack](tech-stack.md)
- [Setup](setup.md)
- [Commands](commands.md)
- [Architecture](architecture.md)
- [Features](features.md)
- [Environments](environments.md)
- [BLE and command interface](api.md)
- [Known issues](known-issues.md)
- [Changelog summary](changelog-summary.md)
- [Documentation standards](documentation-standards.md)
- [Requirements](requirements.md)
- [Screens and controls](screens.md)
- [Roadmap](roadmap.md)
- [Testing](testing.md)
- [Compatibility](compatibility.md)
- [Wire contract](wire-contract.md)
- [Device checklist](device-checklist.md)
- [Message composer](direct-message-composer.md)
- [InfoBot](infobot.md)
- [Implementation plan](implementation-plan.md)
- [Multi-board plan](multi-board-plan.md)
- [Adding a display](adding-a-display.md)
- [Adding a board](adding-a-board.md)
- [ADR-0001](adr/adr-0001-shared-core-and-board-profiles.md)
- [Root README](../README.md)
