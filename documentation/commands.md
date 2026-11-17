# Commands

Run firmware commands from the repository root. Run website commands from `website/` unless a script says otherwise. Website scripts do not upload firmware.

## Firmware

| Command | Purpose |
| --- | --- |
| `pio run -e m5stickc-plus2` | Compile the Plus2 profile. Does not flash |
| `pio run -e m5stickc-plus2 -t clean` | Delete that environment's build outputs |
| `pio device list` | List serial ports |
| `pio run -e m5stickc-plus2 -t upload --upload-port "$STICK_PORT"` | Build if needed and flash over USB |
| `pio device monitor -p "$STICK_PORT" -b 115200` | Serial log. Leave with `Ctrl+C` |
| `pio pkg list -e m5stickc-plus2` | Show resolved platform and library versions |
| `pio pkg install -e m5stickc-plus2` | Install pinned libraries when host crypto tests cannot find Monocypher |
| `sh tools/test-native.sh` | Compile and run the host suites, then the Swift signature check |
| `pio run -e settings-selftest -t upload --upload-port "$STICK_PORT"` | Temporary NVS brightness self-test. Flash `m5stickc-plus2` again afterward |

## Native tests

`tools/test-native.sh` builds these host programs, in order: `profile`, `clock`, `peers`, `navigation`, `composer`, `joyc-navigation`, `joyc-driver`, `feedback`, `settings`, `selection`, `scanner`, `codec`, `fragments`, `crypto`, `noise`, `packets`, `identity`, `mesh`, `bot`, `ui`, and `transport`. The `packets` program writes local signatures that `tools/VerifyPacketSignatures.swift` checks. The script then rejects shared-core references to `M5StickC` or `M5Unified`.

Asset generators under `tools/` (`generate-ui-fonts.py`, `generate-ui-bitmaps.py`, `render-previews.py`, fixture generators) are for regenerating committed assets. A normal firmware build does not require them.

## Website

| Command | Purpose |
| --- | --- |
| `npm ci` | Install from `package-lock.json` |
| `npm run dev` | Astro dev server on `127.0.0.1:4321` |
| `npm run check` | `astro check` |
| `npm run lint` | ESLint |
| `npm run format` | Prettier write |
| `npm run format:check` | Prettier check |
| `npm run audit` | Fail on high or critical advisories outside the documented picomatch allowlist |
| `npm run build` | Write `website/dist/` |
| `npm run preview` | Serve the production build on `127.0.0.1:4321` |
| `npm test` | Playwright against the production preview |
| `npm run render:social` | Export `public/social-card.png` from a preview on port 4322 |
| `npm run build:wire-model` | Rebuild `public/models/bitchat-stick-wire.glb` from approved CAD outside this build |
| `npm run build:demo-screens` | Rebuild simulated 135 × 240 screen PNGs from the firmware UI |

`npm run build:wire-model` reads solids from `../../m5stickc-plus2-models/`. That directory is outside this repository. The committed GLB is what normal website builds use.

`npm run build:demo-screens` needs a native C/C++ compiler and the pinned PlatformIO libraries under `.pio/libdeps/m5stickc-plus2/`. It compiles `test/ui_preview.cpp` against the real UI. Normal website builds use the committed screenshots.

## CI

| Workflow | Trigger | Result |
| --- | --- | --- |
| `.github/workflows/website.yml` | Pushes and pull requests that touch `website/**` or that workflow; `workflow_dispatch` | `npm ci`, audit, check, lint, build, Playwright. Deploys Pages only from `main` and only when the event is not a pull request |
| `.github/workflows/documentation.yml` | Pushes and pull requests that touch `documentation/**` or that workflow | Fails when a file under `documentation/` breaks the kebab-case rule |

No workflow flashes a device.

## Related Documentation

- [Setup](setup.md)
- [Environments](environments.md)
- [Documentation standards](documentation-standards.md)
- [Build and USB deployment](setup.md)
- [Website guide](../website/README.md)
