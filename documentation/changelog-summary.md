# Changelog summary

The full record is [CHANGELOG.md](../CHANGELOG.md). Dates there are the date of the recorded change or report. This summary does not add releases that the changelog does not contain.

`include/Version.h` defines `STICK_VERSION` as `1.1.00`. That is the current source label. This summary does not create a Git tag.

## 1.1.00 — 2026-11-19

The Pocket UI replaces the portrait layout with a 240 × 135 landscape panel, rotation 1, VT323, a civil status clock, and Date & time stored on the RTC. JoyC up and down repeat every 0.5 seconds. The firmware remains a shared ESP32 core plus a compile-time M5StickC Plus2 profile. `!info` identifies the device as Bitchat Stick on ESP32.

On 2026-11-17, Wander Lima confirmed InfoBot replies and correct delivered/read checks on Android and iPhone. Controlled relay, reconnect, range, and battery checks stayed open. Rotation against buttons A/B, JoyC axes after that rotation, the About QR, and an RTC save on the device are still open.

The website is an Astro, Tailwind CSS, and GSAP one-page site. The hero uses the landscape screens. Its loop is one send and one receive, about 11.4 seconds. Publishing the site still requires the GitHub Pages workflow on `main`.

## 1.0.00 — 2026-11-11

The tagged release added the private and public composer, 32 short messages, the QWERTY draft editor, Welcome msg, authenticated DM send with Queued / Delivered / Read, JoyC navigation, persistent sound and JoyC checkboxes, and the screen-capture set. Channels replaced the Inbox name. The private-session pool grew from two to twelve. InfoBot gained public replies without a Noise session, private replies on authenticated DMs, `!about`, and the rate limits. M5GFX 0.2.36 and M5Unified 0.2.8 were pinned.

The Git tag is `v1.0.00`. Rebuilding that tag produces a new build-time clock floor, so the binary is not byte-identical to an earlier upload.

## Related Documentation

- [Overview](overview.md)
- [Known issues](known-issues.md)
- [Changelog](../CHANGELOG.md)
- [Requirements](requirements.md)
