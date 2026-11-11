# Message composer — private and public

## Scope and acceptance

Selecting an identified user in Nearby or replying to a message in Channels opens the private-message flow. Channels → #bluetooth → Write msg opens a new public message using the same editor screens. Read chat opens the public history. BLE advertisements alone are not a verified user and keep their existing inspection view. The recipient is copied by its eight-byte peer ID; changing list order or arriving messages must never change it.

Four screens use the existing cream/green FreeMono style and 135×240 portrait display:

1. **Reply**: recipient, Short msg, Keyboard. Back returns to the exact source screen.
2. **Short msgs**: 32 English presets with a sliding five-row viewport and scrollbar. Selecting one opens confirmation.
3. **Keyboard**: draft above a small three-row QWERTY grid. A/right selects; B/C or up/down traverse keys in reading order. A fourth row offers case/numbers, space, delete and Done. Back returns to the method picker and preserves the draft. Limit: 192 ASCII characters; empty/space-only messages cannot proceed.
4. **Confirm**: recipient, scrollable full message, Send/Cancel. Cancel is initially selected. Back returns to editing without sending. Cancel discards and returns to the source. Private Send requires a current authenticated Noise session and successful transport enqueue. Public Send requires an available subscribed BLE link and queues a signed public message. The destination is explicitly #bluetooth/Public chat for public drafts and a named peer/ID for DMs. Failure keeps the draft for retry. Queued means transport accepted, not delivered. The same screen displays authenticated delivery/read receipts if received.

Message-reading behavior remains: A opens text scrolling; A again starts a reply. Outgoing DMs retain their recipient for subsequent replies. The recipient's short ID is visible on the method and confirmation screens to disambiguate duplicate names. Unknown/offline recipients can be composed for, but cannot send without an authenticated session. No plaintext/public fallback and no automatic retry after a successful enqueue.

## Implementation and checks

Firmware lives in `src/`, public mesh contracts in `include/`, native checks in `test/`; dependencies remain pinned in `platformio.ini`. Drafts/history are RAM-only. Use fixed buffers and validate text before encryption. Emit the official Noise private type 0x01 containing TLV 0x00 message ID and 0x01 content, inside an addressed 0x11 packet. Generate a random 128-bit hex message ID and correlate incoming 0x02/0x03 receipts with both recipient and message ID. Outgoing history never triggers local read receipts.

Reference: [official Android NoiseEncrypted.kt](https://github.com/permissionlesstech/bitchat-android/blob/main/app/src/main/java/com/bitchat/android/model/NoiseEncrypted.kt). This confirms the wire format. The owner subsequently confirmed successful use of the installed sending flow; platform-specific delivery checks remain to be recorded.

Completed implementation order: encrypted sending and receipt tests; composer/UI integration and bounds tests; preview/docs and target build/upload. Each group is reviewed, tested and committed without pushing.

Commands:

```sh
sh tools/test-native.sh
sh tools/render-device-previews.sh
pio run -e m5stickc-plus2
pio run -e m5stickc-plus2 -t upload --upload-port /dev/cu.usbserial-58690062391
```

Always retain recipient identity across list changes, preserve drafts after send failure, bound text and rendering, and keep public/DM traffic separate. Never log private drafts, claim delivery from queue acceptance, add dependencies without need, or persist plaintext drafts.

## Validation status — 2026-11-11

On 2026-11-11, the owner reported that the installed message-sending flow worked well on the device. This confirms the tested flow at owner-observed level; the report did not identify which phone/OS, entry path, input method or receipt state was exercised. Platform-specific and edge-case checks below remain open until individually recorded.

## Device checklist

- [x] Owner confirmed that the installed message-sending flow worked well.

- [ ] Nearby → user → Short msg → scroll through movement/help presets → Cancel sends nothing.
- [ ] Nearby → user → Short msg → Send arrives in the intended Android DM.
- [ ] Channels → open DM → Reply → Keyboard → type, switch case/numbers, space/delete → Done → Send arrives on iOS.
- [ ] New inbound messages while composing do not change recipient/draft.
- [ ] Disconnect before Send: failure shown, draft retained; reconnect/authenticate and retry.
- [ ] Queued changes only after an authenticated delivery/read receipt; duplicate confirmation does not send twice.
- [ ] Verify every key and the 192-character limit using buttons and JoyC.
- [x] Owner confirmed HAT detection and all four directions before this feature.

The 32 presets include Come here, Follow me, On my way, Turn right, Turn left, Help, SOS and Emergency. These send normal private text after the same confirmation step; selecting a preset alone sends nothing.

## Session recovery and channel writing

A known user without an authenticated session now triggers negotiation when Send is selected. The draft remains unsent with Connecting; wait for authentication and explicitly select Send again. Peer gone means the recipient has expired from the table; Link / queue means the current link or bounded outgoing queue did not accept the packet. No failed private draft is sent publicly.

The private-session pool now matches the twelve-peer table rather than the two physical BLE connections. Expired/orphaned sessions are released. The two-session configuration was reproduced failing a three-identity regression; the twelve-session configuration passes it. This source-level correction does not by itself confirm a particular phone exchange.

Channels → #bluetooth offers Read chat and Write msg. Public confirmation shows #bluetooth and Public chat, never a peer ID. Reply from a received message still selects that sender privately. Cancel returns to the original menu. Queued is not delivered; public messages have no private delivered/read receipt.

- [ ] With at least three discovered users, select a signed-only user and try a DM. If Connecting appears, allow negotiation and retry Send; confirm actual phone reception.
- [ ] Receive a fresh DM and verify body, receipt and introduction behavior after recovery.
- [ ] #bluetooth → Write msg → choose/type text → Send: all connected phones receive a public message.
- [ ] Public Cancel sends nothing; user Reply still sends only a DM.
