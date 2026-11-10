# Features and behavior

This document describes the intended product unless a capability is explicitly identified as implemented or documented upstream. See [current implementation status](09-getting-started.md).

## BLE scanner and nearby observations

The initial firmware displays RSSI in dBm, last-seen age, a sanitized advertised name, address/type details, and a candidate classification. A matching service UUID is a **Bitchat candidate**, not an authenticated user. Addresses may rotate and nicknames may collide. RSSI is not a distance measurement. A peer learned through a future relay is not necessarily a physical neighbor.

The current Nearby screen groups the selected observation in a dark-green card with a bright-green left border. Large text and the address detail view avoid shrinking important information.

## Mesh relay — planned

Forward compatible packets with TTL control, deduplication, queue limits, and ingress-link exclusion. Prevent loops, echoes, and replay; use backoff/jitter consistent with the validated protocol. Track received, accepted, duplicate, expired, rejected, queued, transmitted, and failed packets separately. Do not show “Delivered” without appropriate acknowledgment evidence. Until validated, show relay as unavailable/off.

## Channels and messages — planned

The initial candidate context is the local BLE `#bluetooth` chat described in the [official README](https://github.com/permissionlesstech/bitchat#channel-types). Geographic Nostr channels require internet and are outside the MVP. Show each context's transport and availability. Configuring a local name does not create a network channel.

Do not promise discovery of every channel or a complete participant list. Show observed peers rather than members unless association is established. Arbitrary channels, groups, and passwords depend on mobile versions and message paths; old slash commands are not a stable API. For inaccessible payloads, show “Encrypted/unavailable,” never invented content.

## InfoBot — planned

Enable only by local action after compatible reception and transmission have been demonstrated for the selected context. Use a clearly identified bot identity, such as `StickInfoBot`; do not impersonate a phone user.

| Command token | Proposed configurable response |
|---|---|
| `!info` | “About this context: [description]. Use !rules or !help.” |
| `!rules` | “Respect others. Avoid spam. Do not share other people's personal data.” |
| `!help` | “Commands: !info, !rules, !help. Automated replies from StickInfoBot.” |

English command tokens and reply templates are the new default. The originally requested `!regras` and `!ajuda` may be retained as optional compatibility aliases for `!rules` and `!help` when the bot is implemented. None of these are official Bitchat protocol commands.

Trim outer whitespace and accept only the complete command. Do not execute arguments, URLs, or shell text. Ignore synchronized history, duplicate relays, own messages, recognized automated replies, private messages, and unapproved contexts. Reply only in the validated input context. Keep counters rather than message content by default.

Starting spam policy: one reply per sender/context per 30 seconds, no more than six replies globally per minute, one short message per response, and a proposed 160-byte UTF-8 ceiling without splitting code points. Validate that ceiling against the actual codec. Drop/count automated replies when queues fill; do not accumulate an unlimited backlog across reconnections. Prefer verified identities for limits. Without them, leave the bot disabled or require an explicit conservative global policy.

Configuration will be local in a future phase. No command grants privileges or changes the rules. Informational rules are not network moderation.

## Conditional welcome messages

**Disabled by default and blocked until actual channel association can be detected reliably.** This investigation has not established an interoperable join event.

Require an authenticatable event carrying identity and context. Discovery, announces, reconnection, and history restoration must not trigger it. Test exit/rejoin, reboot, replay, and duplicates. If supported later, welcome at most once per identity/context per 24 hours, subject to global bot limits and a bounded cache. If required retention cannot survive reboot safely, leave the feature off after restart. A first message is not a join; a first-message response would be a separate explicitly named feature.

The preferred fallback is a user-requested `!info` response, not unsolicited messages to every nearby radio.

## Future Pocket Messenger

Paged reading, canned replies such as “Received” or “On my way,” context/recipient selection, and explicit confirmation before transmission. Keep the same controls: B up, C down, A select/confirm, hold B to return/cancel. Opening a message must not send anything. Free-text composition and DMs follow only after input usability, identity, sessions, and destination confirmation are validated.
