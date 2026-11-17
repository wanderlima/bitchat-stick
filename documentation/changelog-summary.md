# Changelog summary

The full record is [CHANGELOG.md](../CHANGELOG.md). Dates there are the date of the recorded change or report. This summary does not add releases that the changelog does not contain.

`include/Version.h` still defines `STICK_VERSION` as `1.0.00`. Work listed under Unreleased is in the tree and is not a new tagged firmware release.

## Unreleased

### Modular Plus2 profile

The firmware is split into a shared ESP32 core and a compile-time M5StickC Plus2 profile. Power, RTC, buttons, the optional JoyC HAT, and the 135 × 240 panel stay in that profile. Layout draws through a display surface. The core does not name M5StickC or include M5Unified. `!info` identifies the device as Bitchat Stick on ESP32, with local `#bluetooth` relay and private text DMs.

On 2026-11-17, Wander Lima confirmed InfoBot replies and correct delivered/read checks on Android and iPhone after this profile was flashed. Controlled relay, reconnect, range, and battery checks stayed open.

### Website

`website/` is an Astro, Tailwind CSS, and GSAP one-page site with local fonts, firmware UI renders, a GitHub Pages workflow, and Playwright coverage. Later website passes added the hero entrance, the reduced-motion control, section motion, the hardware walkthrough, structured data, and production-only analytics. Those passes do not flash the Stick and do not by themselves publish GitHub Pages.

## 1.0.00 — 2026-11-11

The tagged release added the private and public composer, 32 short messages, the QWERTY draft editor, Welcome msg, authenticated DM send with Queued / Delivered / Read, JoyC navigation, persistent sound and JoyC checkboxes, and the screen-capture set. Channels replaced the Inbox name. The private-session pool grew from two to twelve. InfoBot gained public replies without a Noise session, private replies on authenticated DMs, `!about`, and the rate limits. M5GFX 0.2.36 and M5Unified 0.2.8 were pinned.

The Git tag is `v1.0.00`. Rebuilding that tag produces a new build-time clock floor, so the binary is not byte-identical to an earlier upload.

## Related Documentation

- [Overview](overview.md)
- [Known issues](known-issues.md)
- [Changelog](../CHANGELOG.md)
- [Requirements](requirements.md)
