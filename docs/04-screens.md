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

Messages returns to Channels on hold-B. Splash only leaves on short-A. Short actions occur on release, with no auto-repeat. The first press after the 30-second screen timeout only wakes the display; navigation stays suppressed until all buttons are released. Scanning continues on its normal schedule. Brightness is restored from a versioned NVS record on boot; invalid/missing records use 50%. Only explicit changed choices write flash. Failed saves show `Not saved / RAM only` in Settings.

## Screens and data sources

| Screen | Live behavior | Placeholder / mock behavior |
|---|---|---|
| Start | A enters Home; scanner continues in the background | Static project name and controls; logo omitted to match the current Figma |
| Home | Bitchat candidate count, unverified status, scanner/battery, four working destinations | Relay remains OFF |
| Nearby | Name, RSSI, age, index/count; B/C cycle; A toggles address/type details | `Bitchat?` is only an advertised-service candidate; every device remains `Not verified` |
| Channels | Honest availability state and reference to Nearby | `No channels / Chat is not ready / Scanner only`; A opens the Messages preview, without joining anything |
| Messages | Hold-B returns to Channels | `No RX/TX / Messages not ready yet / Scanner`; no invented conversation |
| InfoBot | B/C select `!info`, `!rules`, `!help`; A opens/closes a local preview | `Preview only` command examples; no sending, automated reply, or welcome detection; welcome bot OFF |
| Settings | Brightness, Battery, About, Diagnostics destinations | None |
| Brightness | 5%, 25%, 50%, 75%, 100%; A applies and returns, hold-B cancels | None; radio mark is the applied value, cream highlight is the current focus |
| Battery | Actual voltage and estimated percentage, refreshed every five seconds | No remaining-runtime or charging claim |
| About | Wander Lima credits and a real repository QR | No logo or QR placeholder |
| Diagnostics | Advertisement-report count, queue drops, free heap, uptime, version | BLE counts reports, not chat messages |

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
