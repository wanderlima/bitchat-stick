# Screens, layout, and controls

Target display: **135 × 240 pixels, portrait**. Default language: **en-US**. Screens and previews are design/implementation references, not proof of Bitchat messaging compatibility.

## Shared visual system

- Black page background, white primary text, light-gray secondary text, mint/green accents, yellow availability warnings.
- Bold body/headings: `FreeSansBold9pt7b` with a 22-pixel line advance. Brand name: `FreeSansBold12pt7b`. Status and compact secondary information: `Font2`, 16 pixels. The original 8-pixel Font0 has been removed.
- High-contrast selection: a white row with black text. Do not rely on color alone; show state labels.
- Group related information inside outlined panels. Nearby and the BLE channel card use a dark-green fill and bright-green left border, retaining the original visual grouping.
- A live square precedes BLE status: green while scanning, yellow during the scheduled idle period, red on scanner failure. The header also shows the measured battery estimate and a fill-level icon, or `--%` when unavailable.
- Main screen status occupies roughly y=0–20; title y=26–47; content y=51–234. Four-pixel outer panel margins, with text inset to clear borders.
- **No navigation footer on regular screens.** Reuse its area for content. Show navigation instructions only on the splash screen.
- Measure text with the active font. External names may be clipped in the list; address details use two lines rather than a smaller font. Current radio-name rendering sanitizes non-ASCII bytes to `?`; full Unicode names remain future work.

## Consistent physical controls

| Control | Behavior |
|---|---|
| A short press | Confirm/open the selected item; show/close details; adjust selected brightness |
| B short press | Move up / previous observation |
| C short press | Move down / next observation |
| Hold B for 700 ms | Return; Messages → Channels, other screens → Home; Home → Settings |
| Hold A | Reserved; do not also confirm on release |
| Hold C | Reserved for power; never also move down on release |

C is the Plus2 power button, exposed as `M5.BtnPWR`, not a separate generic button object. The firmware retains the 6-second power-off call. Check USB/battery behavior on hardware. The [M5Stack hardware guide](https://docs.m5stack.com/en/core/StickC-Plus2) documents power and HOLD behavior.

Short actions occur on release before 700 ms. B-long fires once while held and does not produce an up action when released. No auto-repeat is implemented. In Home, B/C select a destination and A opens it. Screens with no list do not invent a B/C action. The first A/B/C press after screen-off only wakes it; suppress all navigation until every button is released.

## Splash screen

Centered logo placeholder above the centered BITCHAT / STICK name. Controls below, followed by a high-contrast start action. A enters Home; B/C do not select anything on this informational screen. Holding B does not leave the splash screen. No timed auto-dismiss and no blocking delay: scanning may continue while it is visible. Normal 30-second screen-off still applies.

```text
+-------------------+
|      [ LOGO ]     |
|                   |
|      BITCHAT      |
|       STICK       |
|                   |
|    A: confirm     |
|      B: up        |
|     C: down       |
|   Hold B: back    |
|                   |
|    [ A: START ]   |
+-------------------+
```

## 1. Home / Dashboard

A summary panel displays BLE observations, candidate count, and relay availability. A separate menu panel contains Nearby, Channels, InfoBot, and Settings. B/C navigate these four items; A opens one. Counts are observations rather than verified people, and candidates must not be added to the total as a separate population.

```text
+-------------------+
| BLE scan     ~72% |
| Home              |
|-------------------|
| +---------------+ |
| | BLE: 3        | |
| | Candidates 0  | |
| | Relay: OFF    | |
| +---------------+ |
| +---------------+ |
| | > Nearby      | |
| |   Channels    | |
| |   InfoBot     | |
| |   Settings    | |
| +---------------+ |
+-------------------+
```

## 2. Nearby Peers

One readable observation card at a time, with index/count above it. **Dark-green background, bright-green left border**, white name/RSSI, candidate classification, last-seen age, and an explicit unverified label. B/C select observations. A toggles address/type details. A complete MAC is split into two readable groups; it is not an authenticated identity.

```text
+-------------------+
| BLE scan     ~72% |
| Nearby            |
|-------------------|
| 1 / 3             |
| +---------------+ |
| || ana          | |
| || -64 dBm      | |
| || Candidate    | |
| || Mainnet?     | |
| || 2s ago       | |
| || Unverified   | |
| +---------------+ |
+-------------------+
```

Empty state: “No BLE seen / yet / Scanning...” Do not show fabricated peers. Discovery is limited to received advertisements. Do not convert RSSI to meters or imply that multi-hop peers are direct neighbors.

## 3. Channels

A green BLE-context card groups `#bluetooth`, “Local BLE,” and “No RX/TX.” A separate panel explains that Nostr is unsupported. A opens the message-status screen. This is a proposed context represented in the UI, not a joined channel. No arbitrary-channel creation or membership count is offered.

## 4. Messages

A grouped empty/unavailable state explains “No RX/TX / Messages not available yet / Scanner only.” No invented chat appears. Holding B returns to Channels. Future live messages should page at this font size, preserve reading position when new traffic arrives, and show encrypted/unavailable payloads honestly.

## 5. InfoBot

Three groups: unavailable RX/TX status, selectable `!info` / `!rules` / `!help` commands, and welcome-bot OFF status. A shows a local explanation; it does not send anything. Keep command examples separate from operational status. The future bot must remain gated by transport and context validation.

## 6. Settings

A grouped menu opens Brightness, Battery, About, and Diagnostics. B/C choose the item; A opens it; holding B returns to Home.

### Brightness

Five explicit choices: **5%, 25%, 50%, 75%, 100%**. B/C choose, A applies and returns to Settings, holding B cancels. An asterisk marks the applied preset. Default is 50%. Hardware PWM values are rounded to 13/64/128/191/255; labels retain the intended preset percentages.

### Battery

Shows actual voltage when available, an approximate percentage, and explicit estimate/runtime limitations. No invented charging state or remaining hours.

### About

Credits **Wander Lima**, identifies an open-source Bitchat-inspired companion, displays **github.com/wanderlima**, and reserves a box labeled **QR / SOON**. The box is a placeholder, not a scannable QR code. It links to the author's profile, not an unverified project repository or official Bitchat affiliation.

### Diagnostics

Live scanner-report count, queue drops, free heap, uptime, and firmware version/profile. “BLE” counts advertisement reports, not received chat messages. Holding B returns to Settings from each sub-screen.

## Battery and common states

Show an approximate percentage only when available; otherwise `--%`. Do not invent remaining hours or infer charging state without a supported measurement. The firmware defaults to 50% brightness, offers the five explicit presets above, and turns off the display after 30 seconds. The scanner remains scheduled while the display is off.

Low-battery policies originally proposed at 20%/10%, calibrated hysteresis, and richer charging UI are future work; do not describe them as implemented. Initial firmware only changes battery text color below 20%. Real autonomy has not been measured.

Test long names, accents/unsupported glyphs, duplicate names, 0/32 observations, unavailable voltage, large counters, and one-handed operation. The [JPEG previews](screens/README.md) are illustrative and use simulated data. Physical readability must be checked after uploading this revision.
