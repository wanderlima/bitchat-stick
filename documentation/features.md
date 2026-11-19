# Features

Firmware behavior below is what the current Plus2 source implements. Device acceptance that is still open is listed in [Known issues](known-issues.md) and [the device checklist](device-checklist.md).

## Discovery and sessions

Nearby shows three distinct states: service-advertisement candidates, signed peers, and Noise-authenticated peers. The passive scanner drops unrelated advertisements and queues both the Bitchat mainnet UUID and the testnet UUID as candidates. The Nearby label for either candidate is `Bitchat?`. The Stick itself advertises only the mainnet service. Addresses can rotate, names can collide, and advertisements can be forged. RSSI is a signal reading, not a distance. A peer learned through relay is not necessarily a radio neighbor. Inaccessible payloads stay unavailable; the UI does not invent their text.

The Stick stores a persistent Noise and signing identity. Sessions start automatically for identity-bound peers. A user-initiated send can start missing authentication and keeps the unsent draft. The private-session pool holds twelve records. Orphaned sessions are released. At most two phones can hold a subscribed GATT link at once.

## Public chat and relay

Channels separates **#bluetooth** from **Direct DMs**. `#bluetooth` is the shared public BLE context. It is not a new channel, and it is not a geographic or Nostr channel.

Signed public announcements and text are verified, deduplicated, and eligible for TTL-limited forwarding. The forward path does not echo the ingress link. Public channel-join greetings stay off, because discovery, a new nickname, reconnection, or a first message does not prove that someone entered a channel. There is no interoperable join event in the current contract. A later welcome would need that event, a bounded once-per-identity record, and the same rate limits. The fallback is a person sending `!info`.

Channels → #bluetooth offers Read chat and Write msg. Public drafts name the channel and send as signed public text. Replying to a person stays private.

## Direct messages

Nearby users and channel messages open one composer:

1. Reply, with 32 scrolling English short messages.
2. A QWERTY keyboard for lowercase, uppercase, digits, punctuation, space, and delete.
3. Welcome msg, which inserts the shared introduction and still requires confirmation.
4. Confirm, with Send and Cancel.

Drafts are RAM-only and at most 192 ASCII characters. Send requires an authenticated live session. Queued, Delivered, and Read are separate states. Receipts match both message ID and peer ID. Outgoing DMs do not generate local read receipts, and a late delivered receipt does not downgrade a message already marked read.

An envelope in the header means unread incoming messages. Home and each inbox show counts. Opening a DM schedules an encrypted read receipt. Unread state is RAM-only. New arrivals keep the current reading position. History keeps twelve entries. At most 192 bytes are rendered per entry, with an explicit truncation mark.

With Bot ON, the first fresh authenticated DM from each person in the current boot receives one private introduction that points at `!help` and `!about`. The introduction list holds 32 identities and does not evict. Public traffic does not trigger it. A queue failure can retry on the next new DM.

## InfoBot

Commands, with surrounding whitespace and ASCII case ignored:

| Command | Alias | Reply path |
| --- | --- | --- |
| `!info` |  | Public when the command is signed public text; private when it arrives on an authenticated DM |
| `!rules` | `!regras` | Same path rule |
| `!help` | `!ajuda` | Same path rule |
| `!about` |  | Same path rule. Includes the project URL |

`!info` answers `InfoBot: Bitchat Stick | ESP32 | #bluetooth local relay and private text DMs.`

A peer may trigger a reply at most once every three seconds. A separate one-second global limit applies. The command timestamp must be within two minutes of the local clock. The InfoBot screen shows the latest reply or block status. Command rows explain the words; they do not transmit a request. The on-device Bot switch takes effect immediately and lasts for the current boot only. Arguments, URLs, and shell text in a command are never executed.

## Controls and settings

| Input | Action |
| --- | --- |
| A short | Confirm or open |
| B short | Up |
| C short | Down. C is the Plus2 power button |
| B held at least 700 ms | Back. From Home, return to Start |
| C held 6 seconds | Power off. A long C press does not become Down on release |
| JoyC left / right | Back / confirm, after recentering. On the keyboard, left and right move between keys |
| JoyC up / down | Move, repeating every 0.5 seconds while held. On the keyboard every direction repeats |

The splash screen shows the name and `A Start`. Later screens keep a one-line footer for B/C, hold-B, and A. The first press after the screen turns off only wakes it. The status bar shows BLE state, the civil clock as `HH:MM`, the battery, and a green envelope while any incoming message is unread.

Settings includes brightness at 5, 25, 50, 75, and 100 percent, default 50 percent. Sound has independent persisted checkboxes for interaction beeps, DM alerts, and `#bluetooth` alerts. JoyC HAT settings add optional click confirmation and LED pulse, both off by default. Date & time edits the civil clock written to the RTC. An invalid date is rejected. A failed save shows Not saved and does not change the clock already in use. About credits Wander Lima and shows a QR code for `https://wanderlima.github.io/bitchat-stick/`. Diagnostics shows counters, not message bodies.

Drawing uses a 240 × 135 offscreen framebuffer before the panel updates. The panel rotation is 1.

## Screens

`src/Ui.cpp` implements Splash, Home, Nearby, Channels, Messages, InfoBot, Settings, Brightness, Battery, About, Diagnostics, Sound, JoyC HAT, Date & time, Reply, Short msgs, Keyboard, and Confirm.

## Website

The site explains the product, shows a conceptual mesh animation, and demonstrates physical controls with simulated screens. The hero does not claim Delivered or Read while the illustrated state is Queued. Reduced motion is available from the footer and from the operating-system preference. GA4 and the GTM container load only in production builds and only off localhost.

## Not in this firmware

Private media, private groups, Nostr, gossip or history sync, arbitrary routed Noise forwarding, OTA, and persistent chat history are out of scope. Wi-Fi and LoRa are not transports in this image. The Plus2 has no built-in LoRa radio.

## Related Documentation

- [Architecture](architecture.md)
- [BLE and command interface](api.md)
- [Known issues](known-issues.md)
- [Screens and controls](screens.md)
- [Composer checklist](direct-message-composer.md)
- [InfoBot](infobot.md)
- [Website guide](../website/README.md)
