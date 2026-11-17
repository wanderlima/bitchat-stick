# Architecture

## Proposed layers

```text
Buttons / display / power
          ↕ events and snapshots
Application: states, contexts, InfoBot, configuration
          ↕ authorized messages and metrics
Protocol: codec, identities, signatures, optional sessions
          ↕ bounded packets
Mesh: deduplication, TTL, scheduling, supported fragments
          ↕ RX/TX queues
BLE transport: scan, central/peripheral GATT, MTU, notify/write
```

The current firmware follows those boundaries with one shared core and a compile-time board profile. `src/main.cpp` calls the selected profile for power, polling, UI and diagnostics. Protocol modules own scanner, transport, codec, Noise, identity, settings, mesh and InfoBot. They do not include `M5Unified.h` or the text `M5StickC`. The Plus2 profile in `src/boards/m5stickc-plus2/` owns GPIO4 HOLD, M5 initialization, the board guard, RTC, buttons, battery, speaker and the 135×240 display surface. `src/Ui.cpp` draws only through `include/DrawSurface.h`. A second board needs its own profile and binary, as described in [adding a board](18-adding-a-board.md) and [adding a display](17-adding-a-display.md).

The current code contains a passive scanner, peripheral GATT transport, persistent identity, bounded framing/security/Noise modules, a single-owner mesh model, InfoBot, UI and input handling. A relay forwards verified public packets and must not expose private plaintext to the bot. Headless relay policy for a second board remains proposed. Do not treat the Plus2 participant image as that relay.

## Transport and codec investigation

Extract service/characteristic UUIDs, properties, CCCD behavior, write/notify procedures, MTU limits, and connection roles from pinned mobile commits. Advertisements are not a complete message transport. Concurrent scanning, advertising, and connections must be tested on the Plus2's classic ESP32.

The [official whitepaper](https://github.com/permissionlesstech/bitchat/blob/main/WHITEPAPER.md), sections 4–5, outlines binary framing, TTL, signatures, fragments, and Noise. Use it as a map; check every field against the actual serializers. Do not copy historical packet enum numbers. TTL changes must preserve the signature contract of the selected format. Deduplicate by protocol packet identity, not visible message text.

Proposed RX pipeline: validate envelope → enforce limits → verify authenticity where required → classify type/destination → deduplicate → schedule eligible forwarding → deliver only authorized content to the application. Unknown packets require an explicit safe policy; do not forward arbitrary bytes by default.

## Concurrency and state

BLE callbacks copy bounded events into a queue. Workers process protocol/routing; the UI consumes snapshots without concurrent mutation. Expiry/backoff use a monotonic clock rather than remote timestamps.

Implemented peer progression: observed → compatible service → connected → verified identity → session ready where required → disconnected/expired. Multi-hop reachability, direct radio proximity, and channel membership are different attributes.

The current implementation has one owner for PeerTable: the application loop. NimBLE callbacks only enqueue fixed-size observations. Input is shared across screens through `src/NavigationInput.h`; it suppresses short actions after long presses and after waking the display.

## Initial budgets

| Resource | Starting target |
|---|---|
| Observations | 32 entries; expire and replace the oldest |
| Concurrent links | 2 subscribed peripheral links |
| Current observation queue | 32 fixed-size events |
| Packet RX/TX queues | 12 × 512-byte inbound events; 6 × 4096-byte outbound frames |
| Visible history | 12 messages, public, private and outgoing together; 192 displayed bytes each. About 300 bytes per entry. Raising the cap is a memory-budget change and should stay within the 64-entry deduplication window |
| Deduplication | 64 canonical packet/signature digests |
| Bot contexts | One explicitly approved context |

These are local engineering budgets, not official protocol constants. Display truncation must never silently truncate outgoing packets. Fragment reassembly needs bounds on bytes, count, and lifetime; oversized input must cause a counted rejection rather than overflow. Do not copy mobile memory quotas into the MCU.

## Persistence and power

Observations, peer pins, messages and the bot enable switch are RAM-only. Brightness and local identity are versioned NVS records. Further preferences/reset controls remain future work. Never show or log private keys. Conversation history remains nonpersistent by default. Identity reset needs explicit local confirmation and an explanation that the identity changes.

The screen can turn off while scanning continues. Deep sleep interrupts radio operation. Respect Plus2 HOLD and button C behavior; do not assume the Plus model's PMIC. Reducing scan duty affects discovery and latency, so show/measure the profile. The bot must not silently change power mode.
