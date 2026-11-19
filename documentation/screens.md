# Screens, layout, and controls

Target: **M5StickC Plus2, 240 × 135 pixels, landscape, en-US**. Layout id `240x135-landscape`, rotation 1 (+90° from the panel's native portrait). See [adding a display](adding-a-display.md) before introducing another panel. The editable [Figma file](https://www.figma.com/design/z2zlfw1uBejLJ5fFfRGkeZ/Bitchat-Stick) is the pixel contract. The written guide is the behavior contract. Where they disagree, the rendered frame wins and the difference is listed below. Live device information is never replaced with sample peers or chat.

## Shared visual system

- Black background; warm cream `#EBDCBA` (`0xEEF7` in RGB565); light-gray secondary text `#DEDFDE`; green `#84FF84`; yellow warnings; red scanner failure.
- VT323 Regular, rasterized to 1-bit GFX bitmaps at 16, 17, 18, 19, 20, 21, 22, 23 and 31 px. Capitals are 2 px taller than the first landscape pass, because the font's em is taller than its ink. ASCII 0x20–0x7E only. See `tools/generate-ui-fonts.py` and the OFL notice in `src/assets/fonts/OFL.txt`. The Stick cannot draw the gray anti-aliasing Figma shows; each glyph is cream or black.
- Status bar: 20 px. A 4×4 BLE square, `BLE scan` / `BLE idle` / `BLE err`, centered `HH:MM`, and the battery at the right. A green envelope sits between the radio label and the clock while any incoming message is unread. Home and Channels rows still show `N new`.
- Title sits under the status line. Lists show three 216×22 rows. Focus is a cream row with about a 2 px corner, black text, and a 16×16 icon. A 4 px track sits at x=228 when the list is longer than three rows. The focused row is not the applied value.
- Footer at y=116: `B/C` on the left, hold-to-go-back in the center, and the A action on the right. Labels change with the screen.
- External names are measured and clipped with an ellipsis. Non-ASCII radio-name bytes are sanitized to `?`. Full Unicode remains future work.

Frame choices that override the written guide:

- Focused rows use a 2 px corner. The guide says radius 0.
- The Start subtitle is 21 px. The bolt is the Start-frame mask.
- Divider `#48423A`, scrollbar track `#767676`, and keyboard key `#101419` are used even though they are not in the guide's color table.
- Confirm shows at most two preview lines and an ellipsis. It does not page the draft. The full draft remains in the keyboard.
- Delivered and Read keep the title `Queued` and put the real state on the right.
- The status clock is the civil time stored on the device. There is no timezone and no NTP in this version.

## Consistent physical controls

| Control | Behavior |
|---|---|
| A short | Confirm/open, toggle local details, apply selected brightness |
| B short | Up / previous observation |
| C short (`M5.BtnPWR`) | Down / next observation |
| B held ≥700ms | Back once; details → parent, Settings child → Settings, other pages → Home, Home → Start |
| A held ≥700ms | Reserved; no confirmation on release |
| C held ≥700ms | No down action on release; six-second hold retains power off |

Messages returns to Channels on hold-B. Splash leaves on short-A or JoyC right. Short physical-button actions occur on release, with no auto-repeat. The first press after the 30-second screen timeout only wakes the display; navigation stays suppressed until all buttons are released. Scanning continues on its normal schedule. Brightness is restored from a versioned NVS record on boot; invalid/missing records use 50%. Only explicit confirmations write flash; known unchanged saved values are skipped, while invalid/unknown records are repaired. Failed saves show `Not saved / RAM only` in Settings.

## Optional Mini JoyC HAT

The firmware supports the [M5Stack Mini JoyC HAT (U156)](https://docs.m5stack.com/en/hat/Hat_Mini_JoyC). The HAT connection uses SDA G0/SCL G26 at 100 kHz through M5Unified Ex_I2C, separate from the internal RTC bus. It reads the signed 8-bit axes at `0x54`, registers `0x20`–`0x21`, following the [M5Stack protocol](https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/610/HAT_MiniJoyC.pdf). Each axis is read with its own one-byte register query. Settings → JoyC HAT offers independent Click and LED pulse checkboxes. Both start disabled. Calibration is read from the module.

| Gesture | Activation | Behavior |
|---|---|---|
| Left | X < −80 | Back once |
| Right | X > 80 | Confirm once |
| Up | Y < −80 | Move up; repeat every 0.5 s while held |
| Down | Y > 80 | Move down; repeat every 0.5 s while held |

Exactly −80 or 80 is inactive. To rearm left/right, return both axes to the inclusive −10…10 center range. The stronger axis wins on a diagonal; equal diagonals do nothing. A deflection wakes a dark screen without navigating; center before the next gesture. Physical buttons take priority if used together. After detection or reconnection, the HAT must report center before it can navigate. Missing/failed reads are ignored and retried without delaying startup.

Diagnostics shows `J:X,Y` when the HAT responds. It exposes the module's calibrated values so the physical direction and thresholds can be verified without changing calibration.

## Screens and data sources

| Screen | Live behavior | Limits / future scope |
|---|---|---|
| Start | Name, shared controls, A enters Home | Final logo remains a design asset task |
| Home | Nearby, Channels, InfoBot, Settings. Row values show the nearby count, unread public text, and Bot ON/OFF | Counts do not prove channel membership |
| Nearby | Signed peers take priority when available; otherwise filtered candidates; A opens ID/address details | Signed-only and Noise-authenticated statuses are distinct; RSSI belongs only to observations |
| Channels | Separate #bluetooth and Direct DMs destinations with unread counts; A opens the selected history | No private groups or Nostr context |
| Messages | Twelve-entry public/DM live history with BLE/DM labels; B/C cycle messages; A opens text-scroll mode; B/C scroll; A again replies; hold-B leaves detail, then returns to Channels | RAM-only, 192 displayed bytes, truncation indicated; replies use a live authenticated session |
| InfoBot | Command reference, authentication readiness, immediate Bot ON/OFF switch | Toggle is per boot; Welcome OFF |
| Settings | Brightness, Battery, Diagnostics, Sound, JoyC HAT, About, Date & time | Seven rows; three visible at a time |
| Sound | Independent Interact, DM alert and Channel square checkboxes | All default off |
| JoyC HAT | Click and LED pulse square checkboxes, connection/button state | Both default off |
| Brightness | Five presets, explicit apply, persistent value, visible save failure | Physical 5% readability remains to be checked |
| Battery | Real voltage and approximate percentage | No calibrated runtime or unsupported charging claim |
| About | Wander Lima credits and a black-on-white QR for the project website | Camera reading still requires physical acceptance |
| Date & time | Shows the applied civil date and 24-hour clock. Set date and Set time edit one field at a time | Invalid dates are rejected. A failed save shows Not saved and leaves the applied clock unchanged |
| Diagnostics | Advertisement/drop counts, heap, uptime, accepted/rejected packets, RTC/build clock | Queued messages are not delivery acknowledgements |

Screens are composed in a 16-bit 240 × 135 PSRAM-backed canvas and pushed after the full frame is ready. The physical display is not cleared between normal frames. If buffer allocation fails, rendering falls back to direct drawing; that degraded path may flicker. Rotation 1 is +90° from portrait. The JoyC axis map was left unchanged for the owner to check in this orientation.

The original wireframes below document the approved layout direction. Their older unavailable labels are illustrative; the table above and current firmware define behavior.

## Nearby hierarchy

```text
[status square] BLE       battery
Nearby
Device 1/3
┌─────────────────────────┐
│ Test phone              │
│ -64 dBm                 │
│ Seen 2s ago             │
│ ─────────────────────── │
│ Bitchat?                │
│ Not verified            │
└─────────────────────────┘
```

The first group contains observed radio data; the second explicitly separates identity uncertainty. Ordinary BLE advertisements are excluded from Nearby; only matching Bitchat service UUID advertisements are listed. Mainnet/testnet filtering remains in the observation model but is not presented as confirmed network membership. An empty list displays `No Bitchat` with a waiting or scan-error state. Diagnostics still counts all received BLE advertisement reports. Discovery is limited to received advertisements; do not infer distance, people, multi-hop neighbors, or reliable arrival from an RSSI/name.

A opens two readable MAC groups and the BLE address type. These are radio identifiers, not authenticated identities. Hold-B closes details before returning Home. Selection follows address/type when other observations expire or slots are reused; if the selected candidate disappears, details close and a valid fallback is selected.

## Channels hierarchy

```text
Channels
┌─────────────────────────┐
│ No channels             │
│ Chat is                 │
│ not ready               │
│ Scanner only            │
└─────────────────────────┘
┌─────────────────────────┐
│ BLE devices             │
│ See Nearby              │
└─────────────────────────┘
```

There is no fake `#bluetooth` entry, active channel styling, membership count, or channel creation. The lower panel is explanatory text; it is not a selectable Nearby shortcut. A retains access to the Messages status preview for this foundation build.

## About assets

The **84 × 84px** QR points to **https://wanderlima.github.io/bitchat-stick/**. It is black on white and generated by `tools/generate-ui-bitmaps.py`. Scanning the physical Plus2 display still needs validation. No remote image is fetched at runtime.

`src/assets/` contains the QR and 10 × 10px radio SVGs, `UiBitmaps.h`, and native `UiFonts.h`. `tools/generate-ui-bitmaps.py` reproduces the exact mask geometry. The checked-in SVG root dimensions define their display slots.

## Verification

The host UI regression checks controls, applied-versus-focused brightness, all screens, live status, ordinary/candidate/empty peers, large counters and text/rectangle bounds. Physical readability, battery behavior, radio scanning, and QR camera reading need a device pass after upload.

## Channels and read state

**#bluetooth** is the apps' existing public local mesh, not a new room created by the Stick. Multiple nearby participants share it. **Direct DMs** filters private messages addressed to the Stick. Home and each inbox show unread counts. The status bar draws a green envelope while any incoming message is unread. Viewing one message marks only that entry read. Incoming messages do not replace the message currently being read.

For DMs, a local read schedules the official encrypted Noise read receipt (`0x02` plus the original message ID). Unopened DMs produce only delivered receipts (`0x03`). Pending read receipts retry with bounded work when a suitable authenticated session is available. A queued receipt is not guaranteed radio delivery, and read state is not persisted across reboot.

## Sound and joystick preferences

Settings → Sound uses square checkboxes for independent selections. Interact plays a short 30 ms beep on an interaction and two beeps on confirmation. DM alert and Channel use different short tones for newly accepted incoming messages. Duplicate packets, announcements and outgoing replies do not trigger message alerts. Sound runs asynchronously while the UI and BLE loop continue.

Settings → JoyC HAT has Click and LED pulse. Click reads the active-low button only when enabled, debounces for 30 ms and requires a released button before the first confirmation. LED pulse produces a brief green fade after interaction. All five checkboxes default off and persist as one versioned NVS record, independently of brightness. Save failure is shown as Not saved.

### HAT reference integration — 2026-11-11

The driver follows the local working Nemo reference's M5Unified transaction sequence, including STOP between register selection and reading. It probes Mini JoyC at 0x54 and classic JoyC at 0x38, using their respective axis/button/LED registers. After a full power cycle and HAT remount, the owner confirmed Hat ready and working navigation. The portrait map uses raw X to the right and inverted raw Y. The panel is now rotation 1. The axis map was not edited again; left returns, right confirms, and up/down navigate until that position is checked on the device.

## Private composer — four additional screens

Nearby A selects the currently displayed identified user. Channels A opens the text reader; A again starts a private reply to that message's user. Reply offers Short msg/Keyboard/Welcome msg; Short msgs provides 32 ready-made English messages with three visible rows and a scrollbar; Keyboard puts the draft above a compact QWERTY layout and has no status bar; Confirm shows the recipient and at most two draft lines, then Send/Cancel. Cancel is initially selected.

B/C traverse the keyboard in reading order. JoyC moves across the key grid: left and right along the row, up and down between rows, including the ABC/123, SP, DEL and OK row. Holding any keyboard direction repeats every 0.5 seconds; other screens repeat only up and down at that same interval. A, and the hat click when it is enabled, selects the key. Hold B backs out one step. ABC/123/abc switches case/numbers, SP inserts a space, DEL removes the last character, and OK opens confirmation. The confirmation preview does not page. Hold B returns to the editor, which keeps the full 192-character draft.

The composer captures a peer ID, not a list position. Failed sends preserve the draft and show Not sent; successful enqueues show Queued and cannot be submitted twice. Delivered/Read require an authenticated receipt. See [behavior and acceptance checks](direct-message-composer.md).

Owner acceptance on 2026-11-11: the installed sending flow worked well. This general confirmation does not individually close the platform, receipt-state or edge-case items in the composer checklist.

## App navigation and InfoBot update — 2026-11-11

The public inbox is named #bluetooth again, with Public chat as its subtitle. Home Back returns to Start. Sound labels the public-message alert Channel. InfoBot offers !info, !rules, !help, !about and Bot: ON/OFF; the top status shows command results or reasons to wait. The noninteractive Welcome: OFF footer is removed. See [InfoBot behavior](infobot.md) for public/private reply conditions.

## Public writing and DM recovery

Channels → #bluetooth now has Read chat/Write msg actions. Write msg opens the shared method/preset/keyboard/confirmation flow with an explicit public target. Existing per-user Reply remains private. A failed private send shows Not sent as the heading and its reason below: Connecting, Peer gone, or Busy. A failed public send says No phone when nothing is subscribed, or Busy when a phone is connected but the outgoing queue did not accept the packet. Connecting retains the draft and starts negotiation; the user must retry Send after authentication.
