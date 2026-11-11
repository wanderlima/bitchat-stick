# Build and USB deployment

Firmware v1.0.00. **Deployment means flashing the Plus2 over USB.** There is no web deployment, OTA, or Bluetooth updater. Build/upload evidence is recorded separately in the device checklist.

## 1. Prepare the environment

Requirements: an M5StickC **Plus2**, USB-C data cable, available serial port, PlatformIO Core, and internet for the first dependency download. Native tests additionally require a C++ compiler. The validated Core version is 6.1.19.

An installation already exists on this Mac:

```sh
/Users/wanderlima/.platformio/penv/bin/pio --version
```

For the standard macOS/Linux installation, add its executable directory to this terminal's PATH:

```sh
export PATH="$HOME/.platformio/penv/bin:$PATH"
pio --version
```

On another computer, follow the [official Core installation guide](https://docs.platformio.org/en/latest/core/installation/index.html); PlatformIO IDE for VS Code also includes Core. Avoid mixing installations while diagnosing issues. Libraries are declared in the project, not installed manually in Arduino IDE.

Open the project root:

```sh
cd /Users/wanderlima/Code/wl/m5Stack/m5stick-bitchat-stick
```

Use the corresponding local path on another computer. Subsequent commands assume this root.

## 2. Build configuration

| Item | Value |
|---|---|
| Environment | `m5stickc-plus2` |
| Platform | `espressif32@6.9.0` |
| Framework | Arduino; validated with Arduino-ESP32 2.0.17 |
| Board base | `m5stick-c` with Plus2 overrides |
| Physical flash setting | 8 MB |
| Application partition | 6 MiB factory, from `partitions.csv` |
| PSRAM | Enabled through flags |
| Libraries | M5Unified 0.2.8, M5GFX 0.2.36, NimBLE-Arduino 2.5.0 |
| Upload speed | 460800 baud |
| Serial monitor speed | 115200 baud |

The runtime checks Plus2 identification. The board-base name does not make this firmware suitable for the original StickC or Plus. Do not substitute C3/C6/S3 targets. A “6 MB Flash” PlatformIO summary reflects the application limit, not the total physical flash capacity.

## 3. Compile without flashing

```sh
pio run -e m5stickc-plus2
```

Expect exit code zero and `[SUCCESS]`. A passing build does not prove display, power, BLE, or messaging behavior.

When a clean rebuild is warranted:

```sh
pio run -e m5stickc-plus2 -t clean
pio run -e m5stickc-plus2
```

First-time downloads may take several minutes. Core packages live in the PlatformIO Core directory; project dependencies/products live under `.pio/`. An earlier agent build used an isolated Core, so a normal installation may download its own packages.

Run the [native model/input tests](09-getting-started.md#native-tests) as documented. They require no device. No PlatformIO-native test environment has been configured, so `pio test` is not a substitute for those commands.

## 4. Identify the USB port

Connect the Plus2 with a data cable, then list serial devices:

```sh
pio device list
```

Compare before/after connection to identify the correct device. Example names: `/dev/cu.usbserial-...` on macOS, `/dev/ttyUSB0` or `/dev/ttyACM0` on Linux, and `COM3` on Windows. These are examples, not a detected device port.

Replace the value below with the actual port:

```sh
STICK_PORT='/dev/cu.usbserial-REPLACE_ME'
```

Use the same terminal for commands referencing that variable. Close other serial monitors before upload.

## 5. Flash over USB

**Uploading replaces the existing device firmware and writes this project's partition table.** Obtain the previous image/backup first if it must be preserved. Do not perform a full flash erase as a routine deployment step.

With the correct port and an approved build:

```sh
pio run -e m5stickc-plus2 -t upload --upload-port "$STICK_PORT"
```

PlatformIO rebuilds as needed and uses the ESP32 upload recipe, including boot/partition components. Keep power connected during flashing. Wait for successful upload, then verify boot and physical operation. Byte verification by the uploader is not functional validation.

Do not flash only `firmware.bin` at address zero. Prefer the PlatformIO recipe rather than guessed offsets or a binary for a different board.

## 6. Open the serial monitor

```sh
pio device monitor -p "$STICK_PORT" -b 115200
```

Exit with `Ctrl+C` before another upload. Re-list/update the port if it changes after reconnection.

The latest source prints this startup line:

```text
Bitchat Stick 1.0.00 | signed BLE mesh | authenticated InfoBot
```

If the monitor starts after this line, it may not appear until the next boot. Every ten seconds the firmware logs aggregate mesh/link/authentication/queue/drop/heap/clock counters. It does not log message bodies or keys. Use Settings → Diagnostics for on-screen counters.

## 7. Validate on the device

1. Confirm the English splash/name and controls and A entering Home.
2. Check A confirm, B up, C down, hold-B back, and hold-C power behavior.
3. Verify font readability, cream/black selection, grouped panels, green Nearby card, and no regular navigation footers.
4. Check actual advertisement observations/RSSI/age in Nearby. Matching UUIDs remain unverified candidates.
5. Exercise brightness presets, screen-off, and wake-only first press.
6. Compare voltage/estimate on USB and battery; no measured autonomy is established.
7. Check About credits/links and Diagnostics, then complete the [hardware checklist](09-getting-started.md#evidence-and-hardware-checklist).

Open Channels → Mesh → Messages for the live reader. InfoBot shows authentication readiness and a per-boot enable switch. Complete the [current device checklist](13-device-checklist.md); network visibility alone does not prove forwarding.

## 8. Artifacts and traceability

Under `.pio/build/m5stickc-plus2/` after a successful build:

| File | Purpose |
|---|---|
| `firmware.bin` | Application image |
| `firmware.elf` | Symbols/debugging |
| `bootloader.bin` | Boot component |
| `partitions.bin` | Compiled partition table |

These generated products are ignored by Git; they are not automatically a universal installation package. Record source snapshot/commit, local edits, build/partition configuration, Core version, resolved packages, target board, and upload/boot result.

```sh
pio pkg list -e m5stickc-plus2
```

The local annotated release tag `v1.0.00` identifies the 1.0.00 source snapshot. Check it out in a separate worktree when rebuilding that release; rebuilding generates a new clock floor, so do not assume the resulting binary is byte-identical to an earlier upload. Restore a known previous image using its matching flash/partition recipe; there is no OTA rollback.

## 9. Troubleshooting

| Symptom | Check / action |
|---|---|
| `pio: command not found` | Check PATH or use the full executable path above |
| Download/DNS failure | Check internet/proxy/registries, then retry without arbitrarily changing versions |
| No serial device | Try a known data cable/USB port; re-run device listing; consult vendor drivers |
| Port busy | Exit monitors and close software holding the port |
| Linux permission denied | Review device permissions and [PlatformIO udev rules](https://docs.platformio.org/en/latest/core/installation/udev-rules.html) |
| Connection/upload timeout | Check cable, power, and port; if needed temporarily try `upload_speed = 115200` in `platformio.ini` |
| Garbled serial output | Set the monitor to 115200; upload baud is independent |
| “Wrong board” | Verify the actual model/detection; do not remove the guard to force it |
| Screen blank after inactivity | Press/release A, B, or C; the 30-second display timeout is expected |
| No Bitchat candidate | Check actual app UUID/permissions/advertisements; passive scanning can miss scan responses |

The [Plus2 vendor guide](https://docs.m5stack.com/en/core/StickC-Plus2) describes a C-to-C port-recognition recovery: disconnect, power off using the power button until the green indicator, and reconnect. Consult the vendor procedure and applicable CH9102 driver. Do not assume A/B implement another ESP32 board's BOOT sequence.

## 10. Evidence and references

Build and native tests have passed during foundation development. The owner has used an earlier device build. Latest UI/language changes require another upload and physical verification; no upload is performed just by updating these documents.

CLI references: [pio run](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html), [device list](https://docs.platformio.org/en/latest/core/userguide/device/cmd_list.html), [device monitor](https://docs.platformio.org/en/latest/core/userguide/device/cmd_monitor.html).

The historical eleven-screen foundation sync passed the full firmware build (42,496 bytes static RAM; 858,801 bytes application flash), all three native suites with sanitizers, software bounds checks and repository-QR decoding. That was software evidence for the earlier foundation snapshot, before subsequent device uploads.

## Release version and tag

The firmware version is defined once in `include/Version.h` and used by the boot banner and Diagnostics. Version 1.0.00 intentionally retains the selected zero-padded patch component; its Git tag is `v1.0.00`.

```sh
git show v1.0.00 --no-patch
git worktree add --detach /tmp/bitchat-stick-v1.0.00 v1.0.00
```

The tag is annotated and local until explicitly pushed. It marks the source snapshot, not a hosted binary or completion of every device acceptance item. The existing [device checklist](13-device-checklist.md) remains applicable.
