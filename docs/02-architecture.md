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

These are architectural boundaries, not a list of implemented capabilities. The current code contains only passive scanning, a bounded observation model, UI, and input handling. A relay should be able to forward ciphertext without exposing it to the bot. Separate scanner, relay, and participant permissions. Do not advertise implemented encryption before vectors and cross-platform tests pass.

## Transport and codec investigation

Extract service/characteristic UUIDs, properties, CCCD behavior, write/notify procedures, MTU limits, and connection roles from pinned mobile commits. Advertisements are not a complete message transport. Concurrent scanning, advertising, and connections must be tested on the Plus2's classic ESP32.

The [official whitepaper](https://github.com/permissionlesstech/bitchat/blob/main/WHITEPAPER.md), sections 4–5, outlines binary framing, TTL, signatures, fragments, and Noise. Use it as a map; check every field against the actual serializers. Do not copy historical packet enum numbers. TTL changes must preserve the signature contract of the selected format. Deduplicate by protocol packet identity, not visible message text.

Proposed RX pipeline: validate envelope → enforce limits → verify authenticity where required → classify type/destination → deduplicate → schedule eligible forwarding → deliver only authorized content to the application. Unknown packets require an explicit safe policy; do not forward arbitrary bytes by default.

## Concurrency and state

BLE callbacks copy bounded events into a queue. Workers process protocol/routing; the UI consumes snapshots without concurrent mutation. Expiry/backoff use a monotonic clock rather than remote timestamps.

Future peer progression: observed → compatible service → connected → verified identity → session ready where required → disconnected/expired. Multi-hop reachability, direct radio proximity, and channel membership are different attributes.

The current implementation has one owner for PeerTable: the application loop. NimBLE callbacks only enqueue fixed-size observations. Input is shared across screens through `src/NavigationInput.h`; it suppresses short actions after long presses and after waking the display.

## Initial budgets

| Resource | Starting target |
|---|---|
| Observations | 32 entries; expire and replace the oldest |
| Concurrent links | Start with 2 when GATT is implemented |
| Current observation queue | 32 fixed-size events |
| Future packet RX/TX queues | Initially 16 each, byte ceiling defined after codec work |
| Future visible history | 20 messages; up to 256 rendered UTF-8 bytes each |
| Future deduplication | Initially 128 expiring records, validated under load |
| Bot contexts | One explicitly approved context |

These are local engineering budgets, not official protocol constants. Display truncation must never silently truncate outgoing packets. Fragment reassembly needs bounds on bytes, count, and lifetime; oversized input must cause a counted rejection rather than overflow. Do not copy mobile memory quotas into the MCU.

## Persistence and power

Current observations and settings are RAM-only. Future persistence may include configuration version, profile, brightness, bot text, and identity. Never show or log private keys. Conversation history remains nonpersistent by default. Identity reset needs explicit local confirmation and an explanation that the identity changes.

The screen can turn off while scanning continues. Deep sleep interrupts radio operation. Respect Plus2 HOLD and button C behavior; do not assume the Plus model's PMIC. Reducing scan duty affects discovery and latency, so show/measure the profile. The bot must not silently change power mode.
