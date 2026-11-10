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

These are architectural boundaries, not a list of implemented capabilities. The current code contains a passive scanner, peripheral GATT transport, persistent identity, bounded framing/security/Noise modules, a single-owner mesh model, InfoBot, UI, and input handling. A relay should be able to forward ciphertext without exposing it to the bot. Separate scanner, relay, and participant permissions. Do not advertise implemented encryption before vectors and cross-platform tests pass.

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
| Future packet RX/TX queues | 12 × 512-byte inbound events; 6 × 4096-byte outbound frames |
| Future visible history | 12 messages; 192 displayed bytes each |
| Future deduplication | 64 canonical packet/signature digests |
| Bot contexts | One explicitly approved context |

These are local engineering budgets, not official protocol constants. Display truncation must never silently truncate outgoing packets. Fragment reassembly needs bounds on bytes, count, and lifetime; oversized input must cause a counted rejection rather than overflow. Do not copy mobile memory quotas into the MCU.

## Persistence and power

Observations, peer pins, messages and the bot enable switch are RAM-only. Brightness and local identity are versioned NVS records. Further preferences/reset controls remain future work. Never show or log private keys. Conversation history remains nonpersistent by default. Identity reset needs explicit local confirmation and an explanation that the identity changes.

The screen can turn off while scanning continues. Deep sleep interrupts radio operation. Respect Plus2 HOLD and button C behavior; do not assume the Plus model's PMIC. Reducing scan duty affects discovery and latency, so show/measure the profile. The bot must not silently change power mode.
