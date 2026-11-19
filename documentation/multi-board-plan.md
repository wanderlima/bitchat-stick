# Multi-board firmware implementation plan

Status: **Plus2 shell implemented.** The shared core, Plus2 profile, display surface, and board/display guides are in the tree. Additional boards, a headless relay policy, and other layouts remain proposed.

Date: **2026-11-11**. Default language: **en-US**.

Decision record: [ADR-001](adr/adr-0001-shared-core-and-board-profiles.md).

## Objective

Make Bitchat Stick reusable across explicitly supported BLE-capable ESP32 boards. A headless profile runs as a bounded public-packet relay; a display profile adds the local chat interface, composition, controls and supported peripherals. Preserve the working M5StickC Plus2 behavior throughout the migration.

“Generic” means a common firmware core plus board-specific builds. It does not mean one binary for every ESP32, automatic detection of arbitrary displays, or an unlimited repeater for every Bitchat packet type. No multi-board support is claimed by this plan.

## Current evidence and implementation boundaries

The Plus2 currently has hardware evidence for discovery, Noise authentication and messaging; the owner has confirmed operation with the iPhone app. This is not controlled indirect-relay, range, endurance or complete receipt/reconnect evidence. The [wire contract](wire-contract.md) and [device checklist](device-checklist.md) remain the source of recorded results.

| Existing area | Portability constraint |
| --- | --- |
| `src/main.cpp` | Calls the selected board adapter. GPIO4 HOLD, M5Unified, the Plus2 board guard, button polling, power-off and JoyC status stay in `src/boards/m5stickc-plus2/` |
| `src/Ui.cpp` | Draws through the display surface. Coordinates are the 240×135 landscape layout. Only `src/boards/m5stickc-plus2/Display.cpp` talks to the panel |
| `src/Joyc.cpp`, `src/Feedback.cpp` | JoyC I2C and the speaker still use the Plus2 profile. JoyC is an optional peripheral, not a second firmware target |
| `src/Clock.cpp` | Shared UTC policy with a board RTC adapter. A bare DevKit normally has no equivalent RTC |
| `src/Scanner.cpp`, `src/Transport.cpp` | NimBLE initialization, bounded queues, peripheral GATT links, advertising restart and notification behavior must survive the split |
| `src/Mesh.cpp`, `include/Mesh.h` | Combines public forwarding, private sessions, message history, InfoBot and introductions; a relay needs an explicit application policy |
| `src/Identity.cpp`, `src/Settings.cpp` | Use Arduino Preferences/NVS; identity and preference compatibility must be preserved |
| `src/CryptoPrimitives.cpp` | Uses the selected ESP32 mbedTLS SHA-256 API; SDK changes require independent crypto-vector checks |
| `platformio.ini`, `partitions.csv` | Plus2-specific 8MB flash, partition limits, PSRAM/cache flags and M5 dependencies must not leak into a 4MB/no-PSRAM profile |
| `tools/test-native.sh` | Host regression runner currently locates crypto dependencies under the Plus2 environment; retain a reproducible test setup during migration |

The existing relay forwards eligible verified public traffic with TTL reduction, duplicate detection and ingress exclusion. Arbitrary routed Noise DMs are not implemented. A missing display must not silently enable new protocol capabilities.

## Scope and non-goals

The first milestone is two working profiles from one codebase:

- **`m5stickc-plus2`:** retain the current interface, public chat, direct-message composition, InfoBot, physical controls, power behavior and optional JoyC.
- **`esp32-devkit-relay` (proposed environment name):** one specifically identified classic ESP32 DevKit/WROOM board, initially targeting a verified 4MB flash layout without PSRAM. BLE discovery, identity, eligible public forwarding and bounded serial diagnostics; no display or input requirement.

C3/S3/C6 ports follow individually after the first headless profile passes hardware acceptance. Additional displays follow as explicit board/display/input combinations. Wi-Fi setup portals, OTA, Nostr, private groups, media, persistent conversation history, arbitrary encrypted-message routing and a full browser emulator remain separate work.

## Architecture and decisions

```text
                 Board profile + node mode
                            |
                  Application lifecycle
                  /                   \
         Headless relay policy     Participant policy
         counters / health         chat / bot / composer
                  \                   /
              Shared Bitchat protocol and mesh core
          codec / signatures / identity / bounded forwarding
                            |
                  ESP32 BLE transport adapter
                            |
                 Supported BLE controller / SDK

Board services: early power init, clock, entropy, storage,
optional display, input, battery, speaker and HAT adapters.
```

### Board profile and node mode are separate choices

A board profile describes real hardware and resource limits. A node mode describes permitted behavior. The first mapping is headless → relay and Plus2 → participant; keep these concepts separate so a future screen-equipped device can deliberately run as a relay.

Select the profile at build time. Use capability flags for `display`, `input`, `battery`, `speaker`, `rtc` and `hat`, with explicit dependencies. Display support also needs a known driver, resolution, orientation and pins. Interactive composition requires usable input. Do not probe unknown GPIOs or assume a connected screen identifies its controller.

A display initialization failure may fall back to headless operation only under a documented profile policy, after essential power, identity, time and radio checks succeed. Report degraded status. Never turn a protocol/security initialization error into an apparently healthy relay.

### Keep interfaces small

Use narrow board-service functions or lightweight structs and explicit lifecycle calls. Avoid a generic hardware framework, dynamic plugin discovery, or virtual interfaces for every function. Keep one owner of mesh state; BLE callbacks continue to enqueue bounded events.

Initially preserve the existing file locations and public APIs where practical. Introduce adapters incrementally rather than moving every module in one change. Remove unused hardware dependencies from headless builds at compile/link time, rather than retaining the entire UI behind runtime checks.

### Proposed source organization

```text
src/
  main.cpp                    # board-neutral orchestration after migration
  app/                        # lifecycle, node policies, health snapshots
  platform/
    esp32/                    # shared BLE, entropy, NVS and clock services
  boards/
    m5stickc-plus2/            # HOLD, M5 display/input/power/RTC/HAT adapters
    esp32-devkit/             # exact verified DevKit profile
  ui/                         # optional presentation and shared navigation
  ...                         # existing protocol modules retained until needed
include/                      # headers and contracts
platformio.ini                # configuration only
partitions/                   # verified per-flash-size partition tables
```

This is a destination, not an instruction to relocate all files immediately. Application source stays inside `src/`; root-level files remain configuration/documentation. Keep native tests in `test/` and tooling in `tools/`, following the repository's existing conventions.

## Modes and capability matrix

| Capability | Headless relay default | Plus2 participant |
| --- | --- | --- |
| Compatible BLE discovery and GATT transport | Required | Required |
| Persistent local identity and signed announcements | Required | Required |
| Signature validation, deduplication, bounded public forwarding | Required | Required |
| Public conversation history and unread state | Disabled | Existing RAM-only behavior |
| Local public-message composition | Disabled | Existing behavior |
| Local encrypted DM sessions and composition | Disabled in the initial relay policy | Existing authenticated behavior |
| Forwarding arbitrary addressed Noise traffic | Unsupported | Unsupported |
| InfoBot and automatic private introductions | Off | Existing configurable behavior |
| Display, buttons, JoyC, speaker, battery/RTC peripherals | Only explicitly configured hardware | Existing supported hardware |
| Health/counter diagnostics | Serial; optional configured status LED | Serial and existing UI |
| Wi-Fi, deep sleep while relaying, persistent chat history | Outside initial scope | No new behavior |

An announced headless node still owns an identity; “relay” does not mean anonymous raw-byte rebroadcast. Do not invent protocol capability fields or alter identity binding to indicate the mode. A relay name can clarify its role while retaining the established wire format.

Before disabling Noise in a headless build, verify how the phone apps behave when the announced node does not accept local DMs. Count unsupported local private traffic; never acknowledge a message as delivered/read, decrypt transit DMs, broadcast a failed DM as plaintext, or silently treat private traffic as public. If interoperability requires a different relay-local policy, record the evidence and a separate decision before widening scope.

## Hardware support policy

| Family/profile | Initial status | Promotion requirement |
| --- | --- | --- |
| M5StickC Plus2 / classic ESP32 | Current supported project target | Existing hardware behavior retained after every migration stage |
| One exact ESP32 DevKit/WROOM model | First proposed headless target | Identify module, flash, USB/serial path, pins and power behavior; build and test on that physical board |
| ESP32-C3 | Later candidate | Compatible pinned SDK/NimBLE/crypto build, measured memory and hardware acceptance |
| ESP32-S3 | Later candidate | Same gates; separately identify flash/PSRAM variant |
| ESP32-C6 | Later candidate | Verify toolchain/platform support instead of inheriting the current classic-ESP32 flags |
| ESP32-S2 | Excluded as a standalone BLE target | No integrated Bluetooth; an external-radio design would be a different project scope |
| Unlisted boards or arbitrary displays | Unsupported until profiled | Board manifest, peripheral mapping and recorded hardware evidence |

Espressif's [BLE support table](https://docs.espressif.com/projects/esp-idf/en/v5.2.4/esp32c6/api-guides/ble/overview.html) distinguishes the chip families. [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino#supported-mcus) lists multiple supported MCUs, but library support alone does not establish compatibility with this repository's pinned platform, crypto APIs or Bitchat clients. Upstream ESP-BLE-MESH is a different protocol from this application's Bitchat forwarding.

## Resource, identity and clock rules

- Measure firmware size, free/minimum heap, largest free block, stack high-water marks, peer/session counts, queue occupancy and drop counters before choosing smaller-board limits. No minimum RAM/flash claim is established by this plan.
- The first headless target must work without PSRAM. Do not blindly inherit `BOARD_HAS_PSRAM`, the cache workaround, 8MB partitions or Plus2 upload limits. Keep bounded backpressure; do not solve capacity failures by allocating unbounded queues.
- Preserve the Plus2 NVS namespace and record formats. Changing a profile must not silently regenerate identity. Never distribute a pre-generated identity, copy private keys between devices, or log keys/message contents in routine diagnostics.
- Keep monotonic expiry/backoff separate from UTC packet timestamps. Forward packets with their existing signed timestamp; do not re-date them to match the relay clock.
- A board without an RTC needs an explicit, tested clock strategy. Start with the existing build-epoch fallback for bring-up, and design a trusted USB/serial UTC-setting path if necessary. A stale build clock can break freshness checks after power loss. Unsigned advertisements or arbitrary peer timestamps must not become trusted time.
- If time is unusable, report `clock unsynchronized` and fail closed for operations requiring valid timestamps. Any future learning of time from authenticated peers needs a documented threat model and interoperability evidence.
- Keep Plus2 power/HOLD initialization in its board adapter and early in boot. Generic boards must never inherit GPIO4 output writes or long-press power behavior by accident.
- Preserve `Bitchat Stick` as the project name on every board and in every mode. Do not announce a different relay name. Devices are distinguished by their public identity, not by a product-name suffix. `!info` reports `ESP32`, not the board model.

## Ordered implementation tasks

All tasks are unchecked because this document does not implement them. Scope labels describe relative complexity, not delivery dates. Paths below are likely touch points; split a task further if it grows beyond one focused change.

### T1 — Record a reproducible Plus2 baseline [S]

Dependencies: none. Likely files: `documentation/device-checklist.md`, a new board acceptance record.

- [ ] Record firmware commit/image hash, dependency versions, current native-test results and the actual Android/iPhone app versions.
- [ ] Capture boot, battery-only HOLD, navigation/HAT, public chat and direct-message behavior with current evidence separated from untested cases.
- [ ] Record flash/heap/queue measurements without changing packet limits.

Verification: `sh tools/test-native.sh`, `pio run -e m5stickc-plus2`, and the relevant manual device checks. Save logs/counters without private payloads or keys.

### T2 — Introduce board capabilities with unchanged Plus2 behavior [M]

Dependencies: T1. Likely files: `include/BoardProfile.h` (new), `src/boards/m5stickc-plus2/Profile.cpp` (new), `src/main.cpp`, native profile test.

- [ ] Define the minimal capability/budget contract and select the existing Plus2 profile by default.
- [ ] Keep the current entry point, radio ordering, display and protocol behavior unchanged.
- [ ] Reject contradictory configurations at compile time where possible.

Verification: native profile-contract tests, Plus2 build and a boot/navigation smoke test.

### T3 — Isolate boot, power and diagnostics from M5Unified [M]

Dependencies: T2. Likely files: `src/main.cpp`, Plus2 profile adapter, board-service header, board-service native test.

- [ ] Move HOLD, M5 configuration/detection, button power-off and peripheral-specific diagnostics behind the Plus2 adapter.
- [ ] Leave the application loop responsible for the same ordered scanner/mesh/transport processing.
- [ ] Keep the Plus2 guard for its own profile; ensure generic orchestration does not poll JoyC or write Plus2 pins.

Verification: native adapter tests, Plus2 build, USB/battery boot and six-second power-off checks.

### T4 — Make clock services usable without the M5 RTC [M]

Dependencies: T3. Likely files: `src/Clock.cpp`, `include/Clock.h`, Plus2 RTC adapter, generic clock test.

- [ ] Separate optional RTC access from shared UTC/monotonic behavior while preserving the existing Plus2 clock path.
- [ ] Define missing/invalid/stale clock states and the trusted setup mechanism needed by the first headless target.
- [ ] Test restart, invalid RTC, stale build time and monotonic rollover; do not trust unauthenticated peer time.

Verification: deterministic native clock tests and phone interoperability checks after USB boot, power loss and an intentionally stale build epoch.

### Checkpoint A — Preserve the working device

The Plus2 must still build and pass the T1 smoke path. If it regresses, restore the known working adapter behavior before proceeding. No generic-board support is advertised at this checkpoint.

### T5 — Detach UI lifecycle and optional peripherals [M]

Dependencies: T3–T4. Likely files: `src/main.cpp`, `include/Ui.h`, `src/Ui.cpp`, a no-UI adapter, adapter test.

- [ ] Introduce a small presentation lifecycle that can initialize/update the existing UI or a headless implementation.
- [ ] Make no-display/no-input behavior explicit; do not call M5 graphics, speaker, battery or HAT APIs from the headless path.
- [ ] Preserve settings formats and current Plus2 interaction/sound behavior.

Verification: native lifecycle tests, Plus2 UI bounds/navigation suites and an M5-free link smoke test for the no-UI adapter.

### T6 — Add the initial relay policy [M]

Dependencies: T2, T5. Likely files: `include/Mesh.h`, `src/Mesh.cpp`, a node-policy header, focused mesh-policy tests.

- [ ] Select relay versus participant behavior explicitly; default the existing Plus2 build to participant.
- [ ] Keep verification, TTL, canonical deduplication, ingress exclusion and queue limits; suppress local history, bot/introduction output and local DM processing in the initial relay policy.
- [ ] Preserve signed announcements and identity semantics; document phone behavior for unsupported local private traffic.

Verification: native fixtures for valid/invalid public packets, TTL boundaries, duplicate/replay handling, self traffic and forbidden DM/bot output. Re-run existing Noise/crypto/InfoBot suites for participant mode.

### T7 — Build the exact DevKit relay profile [M]

Dependencies: T4–T6. Likely files: `platformio.ini`, `partitions/esp32-devkit.csv` (new), DevKit profile adapter, board contract test.

- [ ] Identify one physical classic ESP32 DevKit/WROOM variant; select its verified manifest, flash layout and USB/serial upload configuration.
- [ ] Add the proposed `esp32-devkit-relay` environment without M5Unified/M5GFX, Plus2 power logic, display assets or mandatory PSRAM flags.
- [ ] Preserve the original default environment and existing Plus2 partition behavior.

Verification: both target builds, map/dependency inspection confirming no M5 UI code in the headless artifact, and partition/flash-size checks. The proposed environment command is not available until this task is implemented.

### T8 — Validate headless boot and unattended operation [M]

Dependencies: T7. Likely files: DevKit adapter, shared health reporter, board acceptance record.

- [ ] Boot from USB power with no display, buttons, HAT or attached serial monitor; remain discoverable and service two phone links within measured limits.
- [ ] Record safe serial identity/name, board/mode, clock health and bounded radio/relay/queue counters; make the time setup path usable if required.
- [ ] Exercise reconnect, advertising restart, queue saturation, reset and a one-hour powered soak without a crash or unbounded memory growth. One hour is an initial gate, not an endurance claim.

Verification: the actual identified board, phone builds and a timed observation report with memory high-water data.

### Checkpoint B — A headless participant in the radio network

Both profiles build. The DevKit runs without a UI or PSRAM and reports truthful health. Successful discovery or two subscribed phones is still not evidence that a message crossed an indirect path.

### T9 — Prove the headless public relay path [M]

Dependencies: T8. Likely files: `documentation/testing.md`, a board acceptance record; code changes only for demonstrated defects.

- [ ] Pass the controlled A → relay → B and B → relay → A experiment below, with unique messages and ingress/egress evidence.
- [ ] Verify no forwarding for duplicates, invalid signatures, exhausted TTL, prohibited private packets or the ingress link.
- [ ] Record background/range limitations separately; keep unsuccessful scenarios marked unverified.

Verification: physical Android/iPhone experiment with negative controls, recorded versions and packet/counter evidence. Do not mark this task done from host tests or software-rendered screens.

### T10 — Add repeatable build gates and release documentation [M]

Dependencies: T9. Likely files: a firmware CI workflow, `tools/test-native.sh`, `documentation/setup.md`, board support matrix.

- [ ] Build each supported environment and run relevant host regressions; keep the website workflow independent.
- [ ] Publish clearly named, separate artifacts with board/mode, flash layout, version and checksum, plus exact USB instructions. Never attach one binary as a universal ESP32 image.
- [ ] Link each supported board to a hardware evidence record and explain rollback, configuration and remaining limits.

Verification: reproducible clean builds and checksum comparison. Release/tag creation and physical uploads remain explicit implementation/release actions, not actions performed by this plan.

### T11 — Add one C3 or S3 headless profile [M per board]

Dependencies: T10. Likely files per slice: `platformio.ini`, one profile adapter, one partition table, one acceptance record.

- [ ] Choose an available physical board and pin a compatible platform/SDK/NimBLE/crypto combination without silently upgrading the Plus2 stack.
- [ ] Build without assuming PSRAM and verify crypto vectors plus measured radio/memory limits.
- [ ] Repeat T8–T9 on that board before promoting it to supported.

Verification: board-specific build, native vector regressions and physical acceptance. C6 is another independent slice with the same gates, not an automatic consequence of the C3/S3 port.

### T12 — Add one additional display/input profile [M per combination]

Dependencies: T10 and a validated BLE board profile. Likely files per slice: display adapter, profile configuration, input adapter, UI/layout tests, acceptance record.

- [ ] Specify controller, resolution, orientation, wiring and usable input; a display alone does not guarantee a usable composer.
- [ ] Separate the display driver from presentation logic; initially retain 135×240 layout support, then add explicit layouts for other resolutions.
- [ ] Verify readable text, navigation, destination confirmation, no flicker, battery reporting only where measured, and documented fallback behavior for a missing configured display.

Verification: render/layout tests plus physical public/DM and input checks on the exact combination. Do not imply that every attached screen is automatically supported.

## Controlled two-phone relay experiment

1. Record firmware hash, board/module/flash, phone models/OS/app versions, time source and power source. Use the BLE `#bluetooth` context and remove unrelated mesh participants; disable internet paths that could confuse observations.
2. Establish that phone A cannot exchange the test traffic directly with phone B under the selected RF conditions. Distance alone is insufficient; use shielding/attenuation and a relay-off negative control. If isolation cannot be established, the result is inconclusive.
3. With the relay off, send unique public messages in both directions and confirm no delivery at the other phone.
4. Power the relay and verify A→relay and B→relay links separately. Send fresh unique messages. Correlate ingress from one link, eligible forwarding to the other link, preserved sender/content/signature semantics and delivery at the destination. Capture evidence beyond a generic `relayed` counter when attributing the path.
5. Repeat in the reverse direction, then remove the relay again. Delivery must stop under otherwise equivalent conditions. Account for stale queues, duplicates and other mobile forwarding paths.
6. Exercise TTL=1, repeated packets, invalid signatures, ingress exclusion and a disconnected destination. Verify bounded retries/drops and no false success or private-to-public fallback.
7. Save the result as **passed**, **failed** or **inconclusive** with the evidence. Separate direct operation with iPhone from this specific indirect-relay claim.

## Risks and mitigations

| Risk | Mitigation / stop condition |
| --- | --- |
| Loss of current Plus2 behavior during abstraction | Small sequential changes, unchanged default target and Checkpoint A before generic work |
| Generic profile boots with wrong power/pin assumptions | Board-local early initialization; no peripheral probing of unknown pins |
| Generic flash layout inherits Plus2's 8MB settings | Verify actual module/flash and each partition boundary before upload |
| Small target runs out of heap or creates too many sessions | Measure before budgeting; relay policy excludes local chat/session features; keep bounded queues and backpressure |
| No RTC causes stale packet timestamps or freshness rejects | Explicit clock health, trusted setup and cold-boot tests; never weaken authenticity/freshness checks to make a demo pass |
| SDK port changes SHA-256/NimBLE behavior | Pin each tested stack and run independent vectors; do not couple a new-chip migration to an unreviewed Plus2 upgrade |
| “Repeater” is interpreted as arbitrary DM forwarding | Public-only policy and explicit unsupported-type tests; separate routing project for private traffic |
| Phones communicate directly or through another node | RF isolation and relay-off/on/off negative controls with path evidence |
| Display failure quietly disables security or identity | Capability fallback only after core health checks; explicit degraded/error diagnostics |
| Network names collide or identity resets | Stable NVS identity per physical device; distinguish relay names without exposing secrets |

## Completion and support gates

The first multi-board milestone is complete only when the Plus2 remains operational, the exact DevKit build runs without a display/PSRAM, and controlled public relay has passed on that physical board. Each additional board has its own gate; successful compilation is **build-only evidence**, not hardware support.

Documentation and future website copy must distinguish **proposed**, **build-only**, **hardware-tested** and **unsupported** profiles. Keep release names and instructions board-specific, with current limitations intact.

## Inputs needed before hardware implementation

- Exact DevKit board/module, flash size and USB/serial details; physical access for acceptance.
- Which C3/S3/C6 board is available next, and whether an additional display/input combination is actually desired.
- Acceptable trusted time setup for unattended headless nodes without an RTC.
- Controlled RF test arrangement and current Android/iPhone versions.

These inputs do not block documenting the plan. They must be resolved before claiming support or choosing unverified pin/partition settings. The next implementation action is **T1**, followed by the Plus2-preserving adapter work; this document does not authorize or perform a device flash.
