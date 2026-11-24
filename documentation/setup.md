# Setup

Two independent setups exist: the Plus2 firmware at the repository root, and the static website in `website/`. Installing website dependencies does not flash a device. Flashing the Stick does not build the website.

## Firmware

### M5Burner

The current Plus2 firmware is ready to burn through M5Burner. Connect an M5StickC Plus2 with a USB data cable and open:

<https://burner.m5stack.com/Z64CT2>

Burning replaces the firmware currently on the device. The sections below are for building that profile from source.

Requirements:

- An M5StickC Plus2 and a USB-C data cable.
- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) on `PATH`. The validated Core in this repository's build guide is 6.1.19.
- Internet access the first time PlatformIO downloads the pinned platform and libraries.
- A C++ compiler, and Swift, only when running host tests.

From the repository root:

```sh
export PATH="$HOME/.platformio/penv/bin:$PATH"
pio --version
pio run -e m5stickc-plus2
```

`pio run` with no environment builds every environment in `platformio.ini`. Pass `-e m5stickc-plus2` so the temporary settings self-test image is not built by accident. The first successful build writes `.pio/build/m5stickc-plus2/firmware.bin`. That file is an application image. The PlatformIO upload recipe also writes the bootloader and partition table. Do not flash `firmware.bin` at address zero by hand.

Generated `.pio/`, `*.elf`, and `*.bin` files are gitignored.

### USB upload

Uploading replaces the firmware on the connected device and writes this project's partition table. Keep power connected during flashing.

```sh
pio device list
STICK_PORT='/dev/cu.usbserial-REPLACE_ME'
pio run -e m5stickc-plus2 -t upload --upload-port "$STICK_PORT"
pio device monitor -p "$STICK_PORT" -b 115200
```

Replace `STICK_PORT` with the port that appears when the Plus2 is connected. macOS often uses `/dev/cu.usbserial-...`, Linux `/dev/ttyUSB0` or `/dev/ttyACM0`, and Windows `COM3`. Those names are examples. Exit the monitor with `Ctrl+C` before uploading again. Upload baud is 460800. Monitor baud is 115200.

The boot line is:

```text
Bitchat Stick 1.1.00 | signed BLE mesh | authenticated InfoBot
```

The banner prints `STICK_VERSION` from `include/Version.h`. `1.1.00` is the current source label. The annotated tag remains `v1.0.00`.

There is no OTA path. After the temporary `settings-selftest` environment, flash `m5stickc-plus2` again before normal use.

Host tests, after dependencies are installed:

```sh
pio pkg install -e m5stickc-plus2
sh tools/test-native.sh
```

`pio test` is not configured and does not replace that script.

### Artifacts

After a successful Plus2 build, `.pio/build/m5stickc-plus2/` contains:

| File | Purpose |
| --- | --- |
| `firmware.bin` | Application image |
| `firmware.elf` | Symbols for debugging |
| `bootloader.bin` | Boot component written by the upload recipe |
| `partitions.bin` | Compiled partition table |

Record the commit, local edits, Core version, `pio pkg list -e m5stickc-plus2`, the board, and the upload result. The annotated tag `v1.0.00` marks that source snapshot. Rebuilding it creates a new clock floor, so the binary is not byte-identical to an earlier upload. There is no OTA rollback.

```sh
git show v1.0.00 --no-patch
```

### Troubleshooting

| Symptom | What to check |
| --- | --- |
| `pio: command not found` | `PATH` includes the PlatformIO Core `penv/bin` directory |
| Download or DNS failure | Network and registries. Do not change pinned versions to get past a download error |
| No serial device | Data cable, USB port, and the vendor CH9102 driver when the Plus2 is not enumerated |
| Port busy | Exit the monitor and any other program holding the port |
| Linux permission denied | [PlatformIO udev rules](https://docs.platformio.org/en/latest/core/installation/udev-rules.html) |
| Upload timeout | Cable and power. A temporary `upload_speed = 115200` in `platformio.ini` is a diagnostic, not the normal setting |
| Garbled serial output | Monitor baud 115200. Upload baud is separate |
| Wrong-board guard | Confirm the hardware is a Plus2. Do not remove the guard to force another model |
| Blank screen after inactivity | Press and release A, B, or C. The 30-second display timeout is expected |
| No Bitchat candidate | The phone app is advertising. Passive scanning can miss scan responses |

The [Plus2 vendor guide](https://docs.m5stack.com/en/core/StickC-Plus2) describes a USB-C recovery: disconnect, hold the power button until the green indicator is off, then reconnect. Button A or B is not that board's BOOT sequence.

Passing a build does not prove display, power, BLE, or messaging. Continue with [the device checklist](device-checklist.md).

## Website

Astro's current minimum Node.js is an even-numbered release at or above **22.23.4**. This repository records Node **24** in `website/.node-version`, and GitHub Actions installs Node 24. Node 22.22.3 is older than the Astro minimum used here.

```sh
node -v
cd website
npm ci
npm run dev
```

Open `http://127.0.0.1:4321/bitchat-stick/`. The base path matches GitHub Pages so broken asset links show up locally.

```sh
npm run check
npm run build
npm run preview
```

Static output is `website/dist/`. It has no application server and no flashing endpoint.

Browser tests need a Chromium install and a production build. Playwright serves the preview on port 4322:

```sh
npx playwright install chromium
npm test
```

## GitHub Pages

The configured public URL is `https://wanderlima.github.io/bitchat-stick/`. That URL is the deployment target. A local branch is not evidence that Pages is already publishing it.

1. In the GitHub repository, set **Settings → Pages → Build and deployment → Source** to **GitHub Actions**.
2. Push `website/` and `.github/workflows/website.yml` to `main`.
3. The `Website` workflow installs from the lockfile, audits dependencies, type-checks, lints, builds, and runs Playwright. A successful non-pull-request run on `main` deploys `website/dist/`.

For a custom domain or repository rename, change `site` and `base` in `website/astro.config.mjs`.

## Related Documentation

- [Commands](commands.md)
- [Environments](environments.md)
- [Tech stack](tech-stack.md)
- [Device checklist](device-checklist.md)
- [Website guide](../website/README.md)
