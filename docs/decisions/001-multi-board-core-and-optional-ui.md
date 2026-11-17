# ADR-001: Shared Bitchat core with board profiles and optional UI

Status: **Accepted for the shared core and the Plus2 profile.** Other boards remain proposed. Guides: [adding a display](../17-adding-a-display.md), [adding a board](../18-adding-a-board.md).

Date: **2026-11-11**.

Plan: [Multi-board implementation plan](../16-multi-board-plan.md).

## Context

The owner wants the same project to run as a relay on an ESP32 without a screen and offer local messaging features on supported devices with a screen and controls. Before this split, the firmware booted only as an M5StickC Plus2 and called its power, display, RTC, speaker and HAT APIs from the core. Protocol modules were already partly separated, but mesh processing also owns local chat history, Noise sessions and InfoBot behavior.

A working Plus2 build and owner-confirmed mobile operation must be preserved. Generic hardware support is not established, and indirect relay still needs a controlled physical experiment. Chip-level BLE support does not identify board pins, flash layout or display wiring.

## Proposed decision

1. Retain one shared protocol/security/forwarding core and the existing bounded, single-owner processing model.
2. Introduce small board-service adapters and build-time profiles. Keep all application implementation under `src/`; preserve the current default environment during migration.
3. Separate hardware capabilities from node policy. Start with Plus2 participant mode and one exact classic ESP32 DevKit headless relay mode.
4. Make display, input, speaker, battery, RTC and HAT optional. Exclude M5/UI dependencies from the headless artifact rather than calling an absent display at runtime.
5. Keep the initial relay policy public-only, authenticated/verified as required by the existing wire contract, and bounded by TTL/deduplication/ingress exclusion. Do not add arbitrary encrypted-DM routing or infer it from the word “relay.”
6. Produce separate, clearly named firmware images for validated boards. Promote each profile from proposed → build-only → hardware-tested using recorded evidence.

## Alternatives considered

| Alternative | Assessment |
| --- | --- |
| Fork a separate repeater repository | Fast initial isolation, but duplicates crypto/protocol fixes and makes interoperability drift likely; not the preferred first approach |
| One universal binary that detects every ESP32/display | Cannot bridge different chip targets/toolchains, unknown pin assignments and arbitrary display drivers safely; reject |
| Keep M5Unified and hide the screen at runtime | Leaves generic startup, memory and peripheral coupling unresolved; insufficient for a bare DevKit |
| Rewrite everything on ESP-IDF before supporting another board | Adds a large platform migration to a hardware abstraction task; defer unless measured constraints justify it |
| Build a large generic hardware framework | Adds abstraction cost beyond the two initial profiles; start with small interfaces and explicit capabilities |

## Consequences

The project gains reusable firmware components and explicit hardware support boundaries. It also gains a build/acceptance matrix, board-specific flash/power/clock responsibilities and more regression coverage. The Plus2's NVS formats, identity and supported UI behavior must stay compatible.

Headless does not eliminate identity, clock or protocol validation requirements. A board without a trustworthy clock strategy or enough memory must remain unvalidated rather than bypassing checks. Additional displays require drivers, layout and input acceptance, not just a successful BLE port.

## Acceptance and lifecycle

The shared core and the Plus2 profile are accepted and present in the tree. Headless relay and every other board remain proposed. On 2026-11-17 the owner confirmed InfoBot replies and correct delivered/read checks on the flashed Plus2 image. That does not validate a second board. If a later decision changes the design, add a superseding ADR instead of deleting this rationale.
