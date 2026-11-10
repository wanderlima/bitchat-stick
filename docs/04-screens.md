# Screens, layout, and controls

Target: **M5StickC Plus2, 135 × 240 pixels, portrait, en-US**. The editable [Figma file](https://www.figma.com/design/z2zlfw1uBejLJ5fFfRGkeZ/Bitchat-Stick) defines the visual direction. All eleven screens are present in the firmware. Live device information is never replaced with sample peers or chat.

## Shared visual system

- Black background; warm cream `#EBDCBA` (`0xEEF7` in RGB565); light-gray secondary text; green accents; yellow warnings.
- Native FreeMono bitmaps: bold 16px body, regular 14px compact text, existing `FreeMonoBold9pt7b` for 18px titles, and `FreeMonoBold12pt7b` for the 24px brand/battery number. Body/compact glyphs are generated from the pinned M5GFX 0.3.0 fonts with area sampling, preserving thin strokes instead of fractional runtime scaling. See `tools/generate-ui-fonts.py`.
- Figma uses Courier Prime because FreeMono is unavailable in its font service. Glyphs are consequently not identical. Integer device advances are 10px body and 9px compact; a few insets compensate for that difference.
- Header: 20px high, subtle outline, live status square, scanner status, measured battery estimate and fill icon. Green means scanning, yellow scheduled idle, red scanner failure. Unknown battery is `--%`; below 20% the battery indication becomes yellow.
- Title region: y=20–50. Four-pixel outer panel margins; content starts at y=54. No regular-screen navigation footer.
- Home, Settings and Brightness use cream selection and 1px separators between rows. Battery, Messages, Diagnostics and Nearby separate distinct groups with an internal divider.
- Nearby retains its green fill and bright left border; this is visual grouping, not authenticated peer status. Channels uses a neutral availability panel.
- External names are measured and clipped with an ellipsis. Non-ASCII radio-name bytes are sanitized to `?`. Full Unicode remains future work.

## Consistent physical controls

| Control | Behavior |
|---|---|
| A short | Confirm/open, toggle local details, apply selected brightness |
| B short | Up / previous observation |
| C short (`M5.BtnPWR`) | Down / next observation |
| B held ≥700ms | Back once; details → parent, Settings child → Settings, other pages → Home, Home → Settings |
| A held ≥700ms | Reserved; no confirmation on release |
| C held ≥700ms | No down action on release; six-second hold retains power off |

Messages returns to Inbox on hold-B. Splash only leaves on short-A. Short actions occur on release, with no auto-repeat. The first press after the 30-second screen timeout only wakes the display; navigation stays suppressed until all buttons are released. Scanning continues on its normal schedule. Brightness is restored from a versioned NVS record on boot; invalid/missing records use 50%. Only explicit confirmations write flash; known unchanged saved values are skipped, while invalid/unknown records are repaired. Failed saves show `Not saved / RAM only` in Settings.

## Screens and data sources

| Screen | Live behavior | Limits / future scope |
|---|---|---|
| Start | Name, shared controls, A enters Home | Final logo remains a design asset task |
| Home | Advertisement candidates, signed-peer/unread counts, envelope, real relay/transport state | Counts do not prove channel membership |
| Nearby | Signed peers take priority when available; otherwise filtered candidates; A opens ID/address details | Signed-only and Noise-authenticated statuses are distinct; RSSI belongs only to observations |
| Inbox | Separate Mesh (`#bluetooth`) and Direct DMs destinations with unread counts; A opens the selected history | No private groups or Nostr context |
| Messages | Twelve-entry public/DM live history with BLE/DM labels; B/C cycle messages; A opens text-scroll mode; B/C scroll; hold-B leaves detail, then returns to Inbox | RAM-only, 192 displayed bytes, truncation indicated; no composer |
| InfoBot | Command reference, authentication readiness, immediate Bot ON/OFF switch | Toggle is per boot; Welcome OFF |
| Settings | Four working destinations | Additional configuration is future scope |
| Brightness | Five presets, explicit apply, persistent value, visible save failure | Physical 5% readability remains to be checked |
| Battery | Real voltage and approximate percentage | No calibrated runtime or unsupported charging claim |
| About | Wander Lima credits and real repository QR | Camera reading still requires physical acceptance |
| Diagnostics | Advertisement/drop counts, heap, uptime, accepted/rejected packets, RTC/build clock | Queued messages are not delivery acknowledgements |

Screens are composed in a 16-bit 135 × 240 PSRAM-backed canvas and pushed after the full frame is ready. The physical display is not cleared between normal frames. If buffer allocation fails, rendering falls back to direct drawing; that degraded path may flicker.

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

The **95 × 95px** QR points to **https://github.com/wanderlima/bitchat-stick**. It uses cream/black and the actual SVG exported from Figma, with a generated local bitmap. The rendered code has been decoded successfully in software; scanning the physical Plus2 display still needs validation. No remote image is fetched at runtime.

`src/assets/` contains the QR and 10 × 10px radio SVGs, `UiBitmaps.h`, and native `UiFonts.h`. `tools/generate-ui-bitmaps.py` reproduces the exact mask geometry. The checked-in SVG root dimensions define their display slots.

## Verification and previews

The [eleven JPEG previews](screens/README.md) are generated from current drawing calls and pinned font bitmaps, using simulated measurements. They are not hardware screenshots or protocol evidence. The host UI regression checks controls, applied-versus-focused brightness, all screens, live status, ordinary/candidate/empty peers, large counters and text/rectangle bounds. Physical readability, battery behavior, radio scanning, and QR camera reading need a device pass after upload.

## Inbox and read state

**Mesh** is a label for the apps' existing public local mesh (`#bluetooth`), not a new room created by the Stick. Multiple nearby participants share it. **Direct DMs** filters private messages addressed to the Stick. The shared header, including Start, shows an envelope whenever a retained incoming message is unread; Home and each inbox show counts. Viewing one message marks only that entry read. Incoming messages do not replace the message currently being read.

For DMs, a local read schedules the official encrypted Noise read receipt (`0x02` plus the original message ID). Unopened DMs produce only delivered receipts (`0x03`). Pending read receipts retry with bounded work when a suitable authenticated session is available. A queued receipt is not guaranteed radio delivery, and read state is not persisted across reboot.
