# InfoBot usage and troubleshooting

InfoBot responds to commands received by **Bitchat Stick**, either in the shared public **#bluetooth** chat or in a direct message to the Stick. On the device, open Home → InfoBot and keep **Bot: ON**. Select Bot: ON/OFF with A or JoyC right to toggle it. The switch defaults to ON after restart.

The command rows on that screen explain each command; selecting them does not send a command to another user. Type the command in a phone's Bitchat chat to request a reply.

| Command | Reply |
|---|---|
| `!info` | Stick hardware and supported local features |
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

The status at the top of InfoBot shows **Waiting**, **Public reply**, **DM reply**, **Wait 3s**, **Bot OFF**, **Check clock**, or **Queue busy**. Public reply/DM reply means a reply was queued. If the original packet is rejected before command processing, check that ordinary chat arrives and that the peer/connection is present; a command status is not available for an unreadable packet. USB telemetry includes `botState` without logging message text.

## Why commands did not reply before this change

The earlier implementation intentionally ignored DM commands and required private authenticated peer state even for signed public commands. It also imposed a 30-second peer cooldown and a narrower timestamp window. Native regression reproduced the signed-public/no-session failure before the correction. The current implementation accepts signed public commands without that extra session requirement, supports private replies, and exposes the remaining command conditions on screen.

## Automatic welcomes

The former **Welcome: OFF** footer described a disabled automatic greeting feature; it was not an editable setting. BLE discovery/reconnection does not reliably prove that someone joined the chat. Automatic welcomes therefore remain disabled, and the misleading footer has been removed. Bot: ON controls command replies only.

## Device checks

- [ ] Connect Bitchat, verify ordinary incoming messages, and keep Bot: ON.
- [ ] In #bluetooth, send `!help`; allow three seconds, then send `!about`. Check the public reply and project link.
- [ ] DM the Stick with `!info`, then `!about` after three seconds. Verify replies appear only in that DM.
- [ ] Send repeated commands quickly: Wait 3s appears and no flood of replies occurs.
- [ ] Switch Bot OFF: commands receive no reply. Switch ON and retry after the interval.
- [ ] Check clock/Queue busy statuses if the message arrived but no reply could be queued.

The private composer was previously confirmed working by the owner. These new InfoBot paths require separate device acceptance after this firmware update.
