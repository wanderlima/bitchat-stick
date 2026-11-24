# Adding a display

Default language: **en-US**. Read this before writing code. The project name stays **Bitchat Stick**. A new panel does not get a new product name, and a failed display init must not become a headless relay.

Compilation is build-only evidence. Do not mark a panel supported until the physical checks at the end pass.

## Stop if a fact is missing

Do not invent pins, probe unknown GPIOs, or copy the Plus2 GPIO4 hold pin. Fill this sheet first. If any required row is unknown, stop.

| Fact | Required | Plus2 value |
| --- | --- | --- |
| Board profile id | Yes | `m5stickc-plus2` |
| Controller | Yes | Panel brought up by `M5.begin` |
| Bus | Yes | Owned by M5Unified; do not restate raw SPI pins for this profile |
| Width × height | Yes | 240 × 135 |
| Rotation | Yes | 3 (−90°) |
| Color depth | Yes | 16 |
| Brightness control | Yes | `M5.Display.setBrightness` |
| Layout id | Yes | `240x135-landscape` |
| Usable input | Yes for a composer | Buttons A, B, and C (`M5.BtnPWR`) |
| Optional HAT | No | Mini JoyC is a Plus2 peripheral, not a separate firmware |

Empty template:

| Fact | Value |
| --- | --- |
| Board profile id | |
| Controller | |
| Bus and pins | |
| Width × height | |
| Rotation | |
| Color depth | |
| Brightness control | |
| Layout id | |
| Buttons or other input | |
| Optional peripherals | |

## What the current layout draws through

`src/Ui.cpp` owns the 240×135 landscape screens. It does not include `M5Unified.h` or LGFX. It calls the surface in `include/DrawSurface.h`:

- `surfaceBegin`
- `frameStart`, `frameFill`, `frameFillRect`, `frameDrawRect`, `frameHLine`
- `frameSetFont`, `frameTextWidth`, `frameSetColor`, `frameDrawText`
- `frameFinish`
- `displaySetBrightness`

Fonts are the VT323 sizes `UiFont::Vt16` through `Vt23` and `Vt31`. One-bit icons and the QR are expanded by the layout into horizontal filled rectangles. There is no widget toolkit.

`include/DisplayProfile.h` fixes the only layout this file may draw: `240x135-landscape`, 240×135, rotation 1. `src/Ui.cpp` `static_assert`s that profile. The Plus2 implementation is `src/boards/m5stickc-plus2/Display.cpp`, and it is the only firmware file that talks to the panel with LGFX.

Buttons, battery percentage, and free heap come from `include/Board.h`, not from the surface. JoyC stays in `src/Joyc.cpp`.

## Path A — same 240×135 landscape, different panel

1. Add a new board profile and a new `Display.cpp` that implements `include/DrawSurface.h`.
2. Keep `src/Ui.cpp` and its coordinates unchanged.
3. Point that profile's display profile at `240x135-landscape`, 240×135, rotation 1.
4. Run `sh tools/test-native.sh`. The existing pixel assertions in `test/ui_test.cpp` stay the oracle. Do not edit them to fit a new renderer.
5. `test/ui_preview.cpp` must keep emitting `F` rectangle lines and `T` text lines. `tools/render-previews.py` already reads that format. Do not regenerate the website PNGs unless an operation trace changed and the surface was then corrected back.

The new panel does not have to be LGFX. It has to implement the same surface calls.

## Path B — any other resolution or orientation

1. Do not scale `240x135-landscape` and do not edit its coordinates in place.
2. Add a new layout id, a new drawing file, and new pixel tests.
3. A display without usable buttons does not get the composer. The JoyC HAT does not replace a description of the board's own buttons.

## Files this work must not change

`src/Mesh.cpp`, `src/Transport.cpp`, the codec, identity, Noise, `partitions.csv`, and the NVS namespace `bitchat-stick`. The announced name remains `Bitchat Stick`. `!info` says `ESP32`, not the panel model.

## Proof

`sh tools/test-native.sh` and `pio run` for that board's environment. Then, on the physical panel: readable text, navigation, destination confirmation, no flicker, and brightness only where it was measured. A successful compile is not hardware support.
