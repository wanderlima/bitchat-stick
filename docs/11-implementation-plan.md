# Implementation plan by screen

Updated: **2026-11-10**. Screens below are ordered from lowest to highest implementation complexity. Tasks within each screen follow the same order. This is an engineering sequence; it does not reorder the device menu or Figma artboards.

Complexity measures the remaining work, including protocol integration and validation, rather than the number of visual elements. Dependencies take precedence over the ranking. Estimates are relative: **S** local UI/configuration, **M** device behavior or state handling, **L** transport or protocol integration, **XL** security, mesh or interoperability work.

Status: **Done** = implemented in source; **Mock** = local preview/unavailable state without real transport; **Pending** = not implemented. A successful build or host test is not evidence of Bitchat interoperability. Physical checks remain required for each firmware revision.

## Recommended screen order

| Order | Screen | Current state | Next work, easiest → hardest | Complexity |
|---|---|---|---|---|
| 1 | Start | Done | Physical readability check; final logo when supplied | S |
| 2 | About | Done | Camera test of repository QR on the display; optional version/build metadata | S |
| 3 | Settings | Done | Persistence policy; save/restore supported preferences | S–M |
| 4 | Brightness | Done | USB/battery checks for all presets; persistent applied value | S–M |
| 5 | Battery | Done, approximate | Unknown/low-voltage device tests; supported charging indication; calibrated estimates; measured autonomy | M |
| 6 | Diagnostics | Done | Counter explanations; richer BLE failures; bounded long-running telemetry | M |
| 7 | Home | Done | Validate candidate totals/state transitions; aggregate future transport/relay states | M–L |
| 8 | Nearby | Done, advertisement candidates only | Candidate filter/device tests; discovery reliability; stable selection; authenticated peers | M–XL |
| 9 | Messages | Mock | Preview/empty/error states; codec; GATT; authenticated sessions; real message reading/sending | L–XL |
| 10 | Channels | Mock | Disabled/channel states; real context model; subscriptions/membership; encrypted channel participation | L–XL |
| 11 | InfoBot | Mock | Local command examples; trusted command context; actual replies; anti-spam; reliable welcome detection | L–XL |

## 1. Start

- **Done · S:** project name, en-US control instructions and A to enter Home.
- **Done · S:** no blocking splash delay; scan scheduling continues.
- **Pending · S:** check readability, wake behavior and startup on USB/battery after each UI update.
- **Pending · S:** add the final logo when an approved asset exists. The current design intentionally omits the placeholder.

## 2. About

- **Done · S:** Wander Lima credits and local QR for `https://github.com/wanderlima/bitchat-stick`.
- **Done · S:** decode the QR from the software-rendered About screen.
- **Pending · S:** verify camera reading directly from the Plus2 display at practical brightness/distance.
- **Pending · S:** optionally include build/version information without reducing QR readability.

## 3. Settings

- **Done · S:** working destinations for Brightness, Battery, About and Diagnostics.
- **Done · S:** shared A/B/C semantics, selection highlighting, row separators and hold-B navigation.
- **Pending · S:** define persisted preferences and reset/default behavior.
- **Pending · M:** implement bounded, versioned preference storage and recovery from invalid values. Settings currently live only in RAM.

## 4. Brightness

- **Done · S:** 5/25/50/75/100% presets; distinct focus and applied radio mark; A applies; hold-B cancels.
- **Done · M:** PWM application and wake restoration; default 50%.
- **Pending · M:** test all presets on USB and battery, including 5% readability and wake.
- **Pending · M:** persist the applied value after the Settings storage policy is defined. Avoid writing flash on every navigation event.

## 5. Battery

- **Done · S:** real voltage, approximate percentage, unknown state and explicit estimate limitation.
- **Done · M:** five-second measurement refresh; low-battery color indication in the header.
- **Pending · M:** validate measurements on USB and battery; handle supported power-source/charging information without inference.
- **Pending · M:** calibrate estimate and hysteresis from observed hardware data.
- **Pending · M:** measure runtime before displaying remaining-time estimates or enabling automatic low-battery policies.

## 6. Diagnostics

- **Done · S:** advertisement-report count, queue drops, free heap, uptime and version.
- **Done · M:** all BLE reports are counted, even when the advertisement is rejected by the Bitchat filter. Queue drops concern only queued matching observations.
- **Pending · S:** clarify report/candidate/drop terminology in any expanded diagnostics view.
- **Pending · M:** expose scanner initialization/restart failures and supported connection errors.
- **Pending · M:** run extended scan/load tests with bounded telemetry; detect memory degradation and queue pressure.

## 7. Home

- **Done · S:** working Nearby, Channels, InfoBot and Settings navigation.
- **Done · M:** Nearby candidate count, explicit unverified status, real scanner/battery header; relay OFF.
- **Pending · M:** verify totals across expiry, scan pauses, backgrounded phones and rotating addresses.
- **Pending · L:** replace relay/transport unavailability with actual state only after the protocol gates pass. A successful advertisement match is not a transport connection.

## 8. Nearby

- **Done · S:** device name, RSSI, last-seen age, paging and address/type details.
- **Done · M:** list only advertisements matching the configured Bitchat mainnet/testnet service UUIDs; exclude unrelated BLE observations from the queue/table and from UI paging. Keep raw advertisement counts in Diagnostics.
- **Done · M:** bounded 32-entry table, 60-second expiry, sanitization and candidate-only indexing.
- **Pending · M:** verify the filter using the owner's Bitchat phone alongside unrelated BLE devices; confirm both supported UUID contexts with pinned mobile versions.
- **Pending · M:** investigate passive-scan misses, including service UUIDs available only in scan responses. Evaluate active scanning separately; it is not enabled by this filter.
- **Pending · M:** preserve selected device by address/type as the table changes; test rotating addresses and duplicate names.
- **Pending · XL:** establish authenticated identity through the real protocol. Until then use `Bitchat? / Not verified`, never claim confirmed users or reliable arrivals.

## 9. Messages

- **Mock · S:** unavailable state, No RX/TX warning, separators and return to Channels. No invented conversation is shown.
- **Pending · S–M:** define connection/loading/empty/error states, paged reading and long/unsupported-text behavior.
- **Pending · L:** implement and test packet encoding/decoding against pinned upstream vectors, including malformed input and fragmentation.
- **Pending · L:** establish GATT discovery, connection lifecycle, notification/write handling and bounded queues.
- **Pending · XL:** validate identity, signatures and Noise/session requirements before presenting protected messages as readable/authenticated.
- **Pending · XL:** receive/send real messages and prove two-phone interoperability. Preserve reading position as traffic arrives.

## 10. Channels

- **Mock · S:** neutral `No channels / Chat is not ready / Scanner only` panel and reference to Nearby. A opens the Messages status preview; no channel is joined.
- **Pending · S–M:** define disabled, loading, joined, disconnected and unavailable channel states without implying a live channel.
- **Pending · L:** model the contexts actually supported by the selected upstream version after Messages transport is functional.
- **Pending · L:** connect channel discovery/subscription and membership events to real protocol evidence.
- **Pending · XL:** support protected-channel participation only where the Stick has the necessary keys/context.
- **Pending · XL:** evaluate Nostr as a separate internet-dependent feature; do not conflate it with offline BLE channels.

## 11. InfoBot

- **Mock · S:** selectable `!info`, `!rules`, `!help` examples labeled `Preview only`; welcome bot OFF.
- **Pending · S:** define editable information/rules/help content and configuration limits.
- **Pending · L:** parse commands only from valid incoming messages and allowed contexts; require functional Messages/Channels first.
- **Pending · L:** send real responses through the transport, with deduplication, rate limits and loop prevention.
- **Pending · XL:** enable welcomes only after a reliable, authenticated entry event can be distinguished from rediscovery, reconnection or replay. Advertisements alone are insufficient.

## Cross-screen dependencies and later milestones

- **Done · S–M:** shared header, native FreeMono sizes, local assets, navigation, 30-second display timeout and wake-only first press.
- **Pending · M:** repeat the physical UI/power test pass after uploads; validate button long-press behavior and screen-off scanning.
- **Pending · L–XL:** complete codec → GATT → identity/session → real messaging before enabling bot or channel participation. See [compatibility gates](07-compatibility.md) and [two-phone tests](06-testing.md).
- **Pending · XL:** mesh relay: TTL, deduplication, forwarding, bounded buffers and phone A → Stick → phone B tests with the direct route excluded. UI relay status must remain OFF until forwarding is functional.
- **Pending · XL:** Pocket Messenger: extend the working Messages experience with paged reading and confirmed canned replies after transport/security gates pass.
- **Pending · L:** OTA, persistent history and broader Unicode support are separate workstreams; they are not hidden switches in the current firmware.

## Next execution sequence

1. Validate the candidate-only Nearby filter on the physical device.
2. Finish the physical checks for Start, About, Settings, Brightness, Battery and Diagnostics.
3. Implement preference persistence and candidate-selection stability.
4. Pin mobile protocol versions and establish codec test vectors.
5. Add and validate transport/security, then real Messages.
6. Enable real channel contexts and relay only after their individual gates pass.
7. Connect InfoBot; gate welcomes separately on reliable entry detection.
8. Build Pocket Messenger on the validated messaging foundation.
