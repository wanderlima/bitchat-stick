# Tech stack

Versions below are the ones declared or recorded in this repository. Host-tool versions can differ on another machine; the firmware guide records PlatformIO Core **6.1.19** as the validated Core.

## Firmware

| Component | Declared or recorded version | Role |
| --- | --- | --- |
| PlatformIO environment | `m5stickc-plus2` | Default build and USB upload target |
| Platform | `espressif32@6.9.0` | ESP32 toolchain and Arduino framework package |
| Framework | Arduino | `src/main.cpp` uses `setup` / `loop` |
| Arduino-ESP32 | 2.0.17, package `3.20017.241212+sha.dcc1105b` | Recorded resolved package in the getting-started guide |
| Board base | `m5stick-c` | Overridden for 8 MB flash, PSRAM, and `partitions.csv` |
| Xtensa GCC | `8.4.0+2021r2-patch5` | Recorded compiler for the validated firmware build |
| M5Unified | 0.2.8 | Plus2 power, buttons, RTC, speaker, and external I2C |
| M5GFX | 0.2.36 | 240 × 135 landscape UI on the Plus2 panel |
| NimBLE-Arduino | 2.5.0 | BLE scanning, advertising, and GATT |
| Monocypher | commit `0d85f98c9d9b0227e42cf795cb527dff372b40a4` | Ed25519, Noise primitives, and hashing |

The application partition is a 6 MiB factory image (`0x600000` in `partitions.csv`). A reserved SPIFFS region is unused; conversations are not stored there. PSRAM is enabled with `-DBOARD_HAS_PSRAM` and `-mfix-esp32-psram-cache-issue`.

Crypto and protocol code in the shared core does not include `M5Unified.h` and does not name M5StickC. `tools/test-native.sh` fails if `src/main.cpp`, mesh, transport, scanner, identity, settings, clock, `src/InfoBot.h`, or the public headers do.

## Website

| Component | Declared version | Role |
| --- | --- | --- |
| Node.js | `>=22.23.4`; `.node-version` is `24` | Astro supports even-numbered Node releases. The current documented minimum is 22.23.4. CI uses Node 24 |
| npm | lockfile in `website/package-lock.json` | `npm ci` is the install used by CI |
| Astro | `~7.4.11` | Static site. `output: 'static'` prerenders pages |
| Tailwind CSS | `^4.3.5` | Utility CSS through the Vite plugin |
| `@tailwindcss/vite` | `^4.3.5` | Registered in `website/astro.config.mjs` |
| GSAP | `^3.15.1` | Hero, section, and scroll motion |
| Three.js | `^0.180.0` | Optional wireframe hardware walkthrough |
| Playwright | `^1.56.0` | Chromium acceptance tests |
| TypeScript | `^5.9.3` | `astro check` and script types |
| ESLint / Prettier | see `website/package.json` | Lint and format |

Astro `site` is `https://wanderlima.github.io` and `base` is `/bitchat-stick`, with `trailingSlash: 'always'`. Canonical URLs, the sitemap, and robots use that configuration. Local preview uses the same base path: `http://127.0.0.1:4321/bitchat-stick/`.

Fonts are self-hosted with Fontsource: Pixelify Sans for short headings and Courier Prime for body copy. Firmware UI renders on the site are software images, not photographs.

## Host tests

Native firmware tests compile with the system `c++` (override with `CXX`) at C++17, `-Wall -Wextra -Werror`, and AddressSanitizer plus UndefinedBehaviorSanitizer. Packet signature checks also run `tools/VerifyPacketSignatures.swift` with Swift CryptoKit. Those tools are required only when running `tools/test-native.sh` or regenerating fixtures.

## Related Documentation

- [Setup](setup.md)
- [Commands](commands.md)
- [Environments](environments.md)
- [Architecture](architecture.md)
- [Build and USB deployment](setup.md)
- [Website guide](../website/README.md)
