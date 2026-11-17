# InfoBot usage and troubleshooting

InfoBot responds to commands received by **Bitchat Stick**, either in the shared public **#bluetooth** chat or in a direct message to the Stick. On the device, open Home → InfoBot and keep **Bot: ON**. Select Bot: ON/OFF with A or JoyC right to toggle it. The switch defaults to ON after restart.

The command rows on that screen explain each command; selecting them does not send a command to another user. Type the command in a phone's Bitchat chat to request a reply.

| Command | Reply |
|---|---|
| `!info` | `InfoBot: Bitchat Stick | ESP32 | #bluetooth local relay and private text DMs.` |
| `!rules` / `!regras` | Public/private context and basic conduct |
| `!help` / `!ajuda` | Commands and usage |
| `!about` | Project description, Wander Lima credit and https://github.com/wanderlima/bitchat-stick |

Commands accept ASCII case differences and surrounding whitespace, but must be the whole message. `!HELP` works; `please !help` is ordinary chat.

## Reply conditions

- A public command must be a valid signed packet from an identified Bitchat peer. A private Noise session is not required for public replies.
- A DM must be decrypted through an authenticated, identity-bound Noise session. Its bot reply stays encrypted and addressed to the same peer. A private command never generates a public reply.
- The command timestamp must be within two minutes of the Stick's clock, in addition to normal packet validation. Old history is not treated as a live request.
- Allow three seconds between commands from the same peer. A shared one-second interval also limits replies across peers. Public and private commands share the peer limit.
- Duplicate packets do not trigger another bot reply. Queue acceptance is not proof that the phone received it.

The status at the top of InfoBot shows **Waiting**, **Public reply**, **DM reply**, **Wait 3s**, **Bot OFF**, **Check clock**, or **Queue busy**, or **Intro sent**. Public reply/DM reply means a reply was queued. If the original packet is rejected before command processing, check that ordinary chat arrives and that the peer/connection is present; a command status is not available for an unreadable packet. USB telemetry includes `botState` without logging message text.

## Why commands did not reply before this change

The earlier implementation intentionally ignored DM commands and required private authenticated peer state even for signed public commands. It also imposed a 30-second peer cooldown and a narrower timestamp window. Native regression reproduced the signed-public/no-session failure before the correction. The current implementation accepts signed public commands without that extra session requirement, supports private replies, and exposes the remaining command conditions on screen.

## Automatic welcomes

The former **Welcome: OFF** footer described a disabled automatic greeting feature; it was not an editable setting. BLE discovery/reconnection does not reliably prove that someone joined the chat. Automatic welcomes therefore remain disabled, and the misleading footer has been removed. Bot: ON controls command replies and the private introduction described below.

## Device checks

On 2026-11-17 the owner confirmed that InfoBot replies correctly on the flashed Plus2 profile. That report does not by itself check Bot OFF, the three-second cooldown or every alias.

- [ ] Connect Bitchat, verify ordinary incoming messages, and keep Bot: ON.
- [ ] In #bluetooth, send `!help`; allow three seconds, then send `!about`. Check the public reply and project link.
- [ ] DM the Stick with `!info`, then `!about` after three seconds. Verify replies appear only in that DM.
- [ ] Send repeated commands quickly: Wait 3s appears and no flood of replies occurs.
- [ ] Switch Bot OFF: commands receive no reply. Switch ON and retry after the interval.
- [ ] Check clock/Queue busy statuses if the message arrived but no reply could be queued.

The private composer was previously confirmed working by the owner. These new InfoBot paths require separate device acceptance after this firmware update.

## First-DM introduction

With Bot ON, the first fresh, authenticated DM from a person queues this private introduction:

> Hi! I am Bitchat Stick, an open-source Bitchat device. Send !help for commands or !about for the project.

This is a reply to contact initiated by that person, not a public channel-join greeting. Public messages, BLE advertisements, reconnects, old history and duplicate DMs do not trigger it. Identity uses the authenticated peer ID rather than a nickname. The identity is remembered separately from expiring peer records, so disconnect/reconnect or peer expiry cannot repeat the introduction during the same boot.

Up to 32 distinct introduced identities are retained in RAM without eviction. Once full, new introductions stop until restart; normal messages and commands continue. Reboot clears this list, so the next DM may receive the introduction again. Bot OFF suppresses it and does not mark the person as introduced. A failed enqueue may be retried on the person's next fresh, nonduplicate DM; there is no timed resend. Successful enqueue counts as introduced even if radio delivery later fails.

A first DM containing a recognized command still receives its normal private command response plus the one-time introduction. The introduction does not consume the command cooldown. Delivery receipts are queued first. The USB introduction counter is separate from command-reply totals; Intro sent means queued, not confirmed delivered.

- [ ] Send ordinary text as a first DM with Bot ON: receive the introduction privately.
- [ ] Send another DM and reconnect: no second introduction during the same boot.
- [ ] Try Bot OFF, then ON and another new DM: introduction is suppressed while OFF.
- [ ] Test !help as first contact: command response and introduction both stay private.
- [ ] Restart: the next fresh DM can introduce the Stick again.

## Manual Welcome msg

The composer also offers Welcome msg to send this introduction manually. It uses the ordinary destination/text confirmation and may be sent publicly or privately by explicit user action. It is independent of the automatic introduction cache.

Unlike the automatic introduction, manual Welcome msg is available with Bot OFF because it is a user-confirmed ordinary message. It neither marks a peer as automatically introduced nor consumes the command cooldown. Cancel/Back send nothing. Use [the composer checklist](direct-message-composer.md#manual-welcome-msg) for private/public acceptance.
