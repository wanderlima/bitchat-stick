# Screen previews — Bitchat Stick

Default language: **English (en-US)**. These software renders come from the current UI drawing calls, pinned FreeMono font bitmaps, and simulated observations/battery values. They are not photographs, device screenshots, or evidence of Bitchat messaging compatibility. The lightweight display fake approximates hardware rendering; confirm the final appearance on the Plus2.

Individual JPEGs: 540 × 960 pixels, a 4× nearest-neighbor enlargement of the 135 × 240 layout. The overview is 4480 × 2220 pixels. Text intentionally retains the pixel grid.

The current revision restores grouped panels, the green Nearby card and left border, a live-state header square and battery icon, native 16px bold/14px compact FreeMono text, and high-contrast selection. Regular-screen navigation footers have been removed. Controls appear only on Start: A confirm, B up, C down, hold B back. C-long retains power behavior.

## Overview

![All screens](00-overview.jpg)

## Start

![Splash screen](07-splash.jpg)

## Home

![Home dashboard](01-dashboard.jpg)

## Nearby Peers

![Nearby green observation card](02-nearby-peers.jpg)

## Inbox

![Mesh and Direct DMs inboxes](03-channels.jpg)

## Messages

![Simulated public message reader](04-channel-messages.jpg)

## InfoBot

![InfoBot controls and authentication state](05-infobot.jpg)

## Settings

![Settings menu](06-settings.jpg)

## Brightness

![Five brightness presets](08-brightness.jpg)

## About

![Author credits and repository QR](09-about.jpg)

The QR points to https://github.com/wanderlima/bitchat-stick. It is generated from the local Figma SVG and has been decoded successfully from the software render. Physical-display camera reading remains to be checked.

Firmware uses FreeMono Regular/Bold. Primary cream is #EBDCBA (RGB565 0xEEF7 on the display). The Figma file uses editable Courier Prime because FreeMono is not available through its font service.

## Battery

![Live battery measurements](10-battery.jpg)

## Diagnostics

![Scanner and device diagnostics](11-diagnostics.jpg)

## Reproduce the renders

Build once with PlatformIO to install the pinned M5GFX font sources. From the root, on macOS with Python 3, a C++ compiler and Swift:

```sh
python3 tools/generate-ui-bitmaps.py
python3 tools/generate-ui-fonts.py
mkdir -p /tmp/bitchat-ui-previews
c++ -std=c++17 -Wall -Wextra -Werror -I test/fakes -I include test/ui_preview.cpp src/PeerTable.cpp src/PeerSelection.cpp src/Settings.cpp -o /tmp/bitchat-ui-preview
/tmp/bitchat-ui-preview /tmp/bitchat-ui-previews
python3 tools/render-previews.py /tmp/bitchat-ui-previews
swift -module-cache-path /tmp/bitchat-swift-cache tools/export-preview-jpegs.swift /tmp/bitchat-ui-previews docs/screens
```

All eleven views use the actual drawing code. Simulated measurements are confined to the host harness; the firmware never seeds its observation table with mock peers.

## Reproduce exports

After installing the pinned PlatformIO dependencies, run `sh tools/render-device-previews.sh` on macOS. It renders the real UI calls using the test display, simulated observations/battery and synthetic signed peer/public-message traffic, then updates the JPEGs. No message is transmitted and no real identity or conversation is embedded. These previews intentionally do not claim Android/iOS interoperability.
