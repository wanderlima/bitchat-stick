# Environments

No application secrets are required to build the firmware or the website. `.env` is gitignored. The website reads Astro's built-in `import.meta.env.PROD`, `BASE_URL`, and `site`. It does not load a project env file.

Public analytics identifiers already shipped in the website are measurement ID `G-VHFJLDXMVC` and GTM container `GTM-K58ZNCMN`. They are client-side measurement IDs, not server credentials. Development and localhost previews do not load them.

## Firmware

| Environment | When to use | Flash effect |
| --- | --- | --- |
| `m5stickc-plus2` | Normal build, host-test dependency install, and USB deployment | Replaces the application with the Plus2 participant image and writes `partitions.csv` |
| `settings-selftest` | Temporary brightness NVS check. `platformio.ini` says to reinstall `m5stickc-plus2` afterward | A reduced image. It is not the messaging firmware |

Both environments extend the same platform, board overrides, and library pins. The self-test adds `-DSTICK_SETTINGS_SELF_TEST` and compiles only `src/Settings.cpp` and `src/hardware/SettingsSelfTest.cpp`.

There is no staging server, no OTA channel, and no separate testnet firmware image. The scanner recognizes the Bitchat testnet UUID, but the Stick advertises the mainnet service.

Serial output is a diagnostic channel at 115200 baud. It prints the boot banner and periodic counters.

## Website

| Environment | How it is produced | Analytics |
| --- | --- | --- |
| Local dev | `npm run dev` on `127.0.0.1:4321`, base `/bitchat-stick/` | Off. `import.meta.env.PROD` is false |
| Local production preview | `npm run build` then `npm run preview`, or Playwright on port 4322 | Off on localhost, `127.0.0.1`, and IPv6 loopback even though the build is a production build |
| GitHub Pages | `Website` workflow on `main`, artifact `website/dist` | On for the deployed hostname |

Pull requests run the same website checks and do not deploy.

Changing `site` or `base` in `website/astro.config.mjs` changes canonical URLs, sitemap, robots, and asset prefixes together.

## Related Documentation

- [Setup](setup.md)
- [Commands](commands.md)
- [Known issues](known-issues.md)
- [Website guide](../website/README.md)
