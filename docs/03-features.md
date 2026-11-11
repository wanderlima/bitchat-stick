# Features and behavior

This document describes the intended product unless a capability is explicitly identified as implemented or documented upstream. See [current implementation status](09-getting-started.md).

## BLE scanner and nearby observations

The initial firmware displays RSSI in dBm, last-seen age, a sanitized advertised name, address/type details, and a candidate classification. A matching service UUID is a **Bitchat candidate**, not an authenticated user. Addresses may rotate and nicknames may collide. RSSI is not a distance measurement. A peer learned through relay is not necessarily a physical neighbor.

The current Nearby screen groups the selected observation in a dark-green card with a bright-green left border. Large text and the address detail view avoid shrinking important information.

## Mesh relay — implemented public subset

Forward compatible packets with TTL control, deduplication, queue limits, and ingress-link exclusion. Prevent loops, echoes, and replay; use backoff/jitter consistent with the validated protocol. Track received, accepted, duplicate, expired, rejected, queued, transmitted, and failed packets separately. Do not show “Delivered” without appropriate acknowledgment evidence. The current relay forwards verified signed public announcements/text; it does not implement arbitrary opaque Noise routing or media forwarding. Hardware indirect-path acceptance remains pending.

## Channels and messages — local BLE implemented

The initial candidate context is the local BLE `#bluetooth` chat described in the [official README](https://github.com/permissionlesstech/bitchat#channel-types). Geographic Nostr channels require internet and are outside the MVP. Show each context's transport and availability. Configuring a local name does not create a network channel.

Do not promise discovery of every channel or a complete participant list. Show observed peers rather than members unless association is established. Arbitrary channels, groups, and passwords depend on mobile versions and message paths; old slash commands are not a stable API. For inaccessible payloads, show “Encrypted/unavailable,” never invented content.

## InfoBot — planned

The device replies under its advertised Bitchat Stick identity. Bot defaults to ON for each boot and can be disabled locally. The installed bot behavior is described below; physical acceptance of newly added paths is recorded separately.

InfoBot implements `!info`, `!rules`, `!help`, `!about`, and aliases `!regras`/`!ajuda`. It accepts a complete command with surrounding whitespace and ASCII case differences. Public replies require a verified signed public packet; private commands require an authenticated Noise session and receive encrypted private replies. Arguments, URLs and shell text are never executed.

The peer cooldown is three seconds and the global interval is one second. Commands must be within two minutes of the local clock. Duplicate/history/self traffic must not cause repeated replies. Failed enqueue shows Queue busy and does not create an unbounded retry backlog. The on-device switch is ON by default after boot. See [InfoBot usage and troubleshooting](15-infobot.md).

## Conditional welcome messages

**Disabled by default and blocked until actual channel association can be detected reliably.** This investigation has not established an interoperable join event.

Require an authenticatable event carrying identity and context. Discovery, announces, reconnection, and history restoration must not trigger it. Test exit/rejoin, reboot, replay, and duplicates. If supported later, welcome at most once per identity/context per 24 hours, subject to global bot limits and a bounded cache. If required retention cannot survive reboot safely, leave the feature off after restart. A first message is not a join; a first-message response would be a separate explicitly named feature.

The preferred fallback is a user-requested `!info` response, not unsolicited messages to every nearby radio.

## Initial Pocket Messenger — implemented

Nearby/Channels entry, 32 short replies, a compact QWERTY editor, and explicit recipient/text confirmation are implemented. B/C navigate, A selects, and hold B returns/cancels; JoyC uses the same actions. Opening a message or choosing a phrase never transmits it. See [the composer](14-direct-message-composer.md).

## Current InfoBot control

The screen lists four commands and Bot: ON/OFF. Command rows explain usage; they do not transmit a request. The top status distinguishes Waiting, queued Public/DM replies, cooldown, disabled state, clock mismatch and queue failure. The former Welcome: OFF footer was not editable and has been removed; public channel-join welcomes remain disabled. The first fresh authenticated DM can receive a one-time private introduction with command hints; see [the introduction rules](15-infobot.md#first-dm-introduction).
