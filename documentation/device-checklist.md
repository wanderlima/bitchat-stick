# Device acceptance checklist — v1.0.00

Updated: **2026-11-17**. Device name: **Bitchat Stick**. Supported context: public BLE **#bluetooth** and authenticated direct text messages. Record the firmware commit/image hash, both phone models/OS/app versions, power source and test time. Start with both apps in the foreground and Bluetooth permissions granted.

## Evidence already recorded

- [x] Owner confirmed DM recovery and new public writing in #bluetooth after the session-pool correction.

- [x] Production target builds and uploads with image hash verification.
- [x] Native regression suites cover protocol, crypto/Noise vectors, persistence, mesh, UI and GATT bounds.
- [x] Reproduced startup reboot loop fixed by pausing scanning during GATT registration.
- [x] Android 2.2.0 recognized the Stick's signed announcement and established Noise; Stick reported an authenticated peer with zero protocol rejects during observation.
- [x] Owner confirmed that the device appeared in the network.
- [x] Final short USB observation showed two subscribed links and two authenticated peers; this is not indirect-relay proof.
- [x] Owner confirmed Android DM delivery status and reading the DM on the Stick.
- [x] Owner confirmed JoyC HAT detection and correct navigation in all four directions.
- [x] Owner confirmed the installed message-sending flow worked well; phone platform, individual input paths and receipt states were not specified.
- [x] Owner confirmed that Bitchat Stick works with the iPhone app. Exact app/OS versions were not provided with that report.
- [x] On 2026-11-17 the owner confirmed InfoBot replies and correct delivered/read checks while testing Android and iPhone against the flashed Plus2 profile. App versions, Bot OFF, command aliases and reconnect-before-open were not separately recorded.
- [ ] Final display, detailed reconnect scenarios, controlled relay and endurance below.

## 1. Display and navigation

- [ ] Boot from USB, then battery only: one splash, no repeated resets.
- [ ] Remain on Home/Nearby for at least one minute: no visible black erase/redraw flashes; text and header remain stable.
- [ ] A confirms/selects, B moves up, C moves down; hold-B backs out; hold-C for six seconds powers off without an extra down action.
- [ ] Open all seventeen screens; verify grouping, dividers, cream text and the green Nearby border.
- [ ] Wait 30 seconds: display turns off. First press only wakes it; selected screen/brightness remain intact.

## 1a. Mini JoyC HAT

- [ ] Boot without the HAT: the normal buttons and BLE still work. Boot again with the Mini JoyC attached and centered.
- [ ] In Diagnostics, verify left/right are negative/positive X and up/down are negative/positive Y using `J:X,Y`.
- [ ] Move within −80…80: no navigation. Cross −80 or 80: the intended direction acts.
- [ ] Hold right or left for several seconds: only one action. Move directly to the opposite side: no second action until both axes return to −10…10.
- [ ] Hold up and down separately: an immediate step, then one step per 1.5 seconds. Move out of the active range to stop repeating.
- [ ] With the display off, tilt once to wake only. Center and tilt again to navigate. Check the physical buttons still work.

## 1b. Sound and JoyC options

- [ ] Start with all Sound and JoyC checkboxes off. Confirm normal navigation with the HAT attached, then disconnected.
- [ ] Sound → Interact plays one short beep on movement/back and two on confirmation.
- [ ] Test DM alert and Channel independently with fresh incoming messages. Duplicate transmissions and own replies stay quiet.
- [ ] Confirm Click is off by default. A stuck button must not prevent axis or physical-button navigation while Click is off.
- [ ] Enable Click, release it, then press again to confirm once. Holding it must not repeat. Test a press held while enabling.
- [ ] Enable LED pulse and interact. Check a short green pulse and return to off. Disable it and verify pulses stop.
- [ ] Restart and verify all five choices persist, along with the existing brightness setting.

## 2. Settings, battery and QR

- [ ] Apply each brightness: 5%, 25%, 50%, 75%, 100%; check readability and radio mark versus focus highlight.
- [ ] Apply a level, fully power off/on, and verify persistence. Back out without applying a different level and verify no change.
- [ ] Compare voltage/estimated percentage on USB and battery. Treat percentage as approximate; do not expect remaining-runtime or charging claims.
- [ ] Scan the About QR: it opens https://github.com/wanderlima/bitchat-stick and credits Wander Lima.

## 3. Discovery and authentication — repeat for each phone

- [ ] Open Bitchat with Bluetooth enabled. Other users see **Bitchat Stick**.
- [ ] Nearby excludes unrelated BLE radios. Candidates say unverified; signed peers and **Noise auth** remain distinct.
- [ ] Wait for automatic Noise authentication. If it remains **Signed only**, record phone/app version, transport status and Diagnostics counters; do not count it as authenticated.
- [ ] Close/reopen the app and toggle Bluetooth. The Stick remains responsive, clears disconnected authentication and recovers without duplicate welcome messages.
- [ ] Repeat with app foreground, background and screen locked; record differences rather than assuming equal discovery.

## 4. Messages and InfoBot

Use your own test phones and synthetic content. Public BLE messages are visible to nearby participants.

- [ ] In Channels, verify **#bluetooth** and **Direct DMs** are separate. The public inbox is the shared local conversation, not a room created by the Stick.
- [ ] From Android, send a unique public message such as `android-stick-001` in #bluetooth; it appears in Channels → #bluetooth on the Stick.
- [ ] Repeat from iPhone with `ios-stick-001`. Verify sender name and actual content.
- [ ] B/C changes message; A enters text scrolling; B/C scrolls long text; hold-B leaves detail, then Messages.
- [ ] Send long text, accents and emoji: no crash, unsupported display glyphs are substituted, and truncation is indicated when applicable.
- [ ] With a signed public peer and Bot ON, send `!info`; receive one signed public reply from Bitchat Stick.
- [ ] Wait at least three seconds between commands from the same peer; check `!rules`, `!help`, `!about`, and aliases `!regras`, `!ajuda`.
- [ ] Send ordinary text, `!info extra`, and repeated commands: no unintended replies or reply flood.
- [ ] In InfoBot select Bot ON/OFF with A. OFF suppresses replies immediately; ON restores them after cooldown. The switch returns to ON after reboot.
- [ ] Reconnect/reopen apps: no welcome spam and no replay-triggered replies. Public join greetings stay disabled; private introductions are once per peer per boot.

## 4a. Private-message reader

- [ ] Wait for **Noise auth**, then send a new DM from Android directly to **Bitchat Stick**.
- [ ] Open Channels → Direct DMs: the message appears with a **DM** label and correct sender/content.
- [ ] Before opening the DM on the Stick, Android should show delivered only. Opening it sends the encrypted read receipt; verify Android changes to read within a few seconds while connected.
- [ ] Repeat from iPhone; test long text and reconnect. Earlier DMs sent before the update are not automatically recovered.
- [ ] Send `!info` as a DM: it is displayed privately and receives a private InfoBot reply, never a public reply.
- [ ] A new incoming message shows the envelope on every screen, including Start, and increments unread counts. Reading one entry clears only that entry; opening #bluetooth must not clear unread DMs.
- [ ] While reading an older message, a new arrival preserves focus and remains unread.
- [ ] Repeat the delivered → read transition on iPhone; test reconnect before opening the DM.

## 5. Controlled relay — do not skip negative controls

- [ ] Turn Wi-Fi/cellular off on both phones; keep Bluetooth on. Use only #bluetooth, with no other relays nearby.
- [ ] Arrange phone A and B so they cannot exchange a new unique message directly. With the Stick off, verify that a new control message does **not** arrive. A failed control invalidates the relay trial.
- [ ] Power the Stick and place it where both phones can connect. Confirm two links/eligible signed traffic, then send a new unique message A → B and B → A.
- [ ] Confirm relay counters increase and no duplicate/loop storm occurs. A successful app delivery while phones still have a direct path is not relay proof.
- [ ] Power the Stick off again and repeat with a new nonce to confirm the negative control still holds.

## 6. Endurance and failure recovery

- [ ] Run for 60 minutes with controlled traffic and both phones; record heap trend, drops, latency, resets and lowest battery voltage.
- [ ] Repeat screen-on/screen-off and USB/battery scenarios. Screen-off is not deep sleep; radio remains active.
- [ ] Disconnect both phones and reconnect: no stuck peer/authentication state, endless queue growth or frozen UI.
- [ ] If any test fails, record the screen, phone/app versions, exact steps, time, last serial counter lines and firmware hash. Do not include keys or private conversations.

## Limits that are not failures of this version

Private media/groups, geographic/Nostr channels, sync/history recovery and OTA are not implemented. Private-message sending with a QWERTY/canned-reply composer is implemented; validate it using [the composer checklist](direct-message-composer.md). History and peer pins are in RAM; local identity and brightness persist in unencrypted NVS. Outgoing public sending is currently exercised through InfoBot and public relay; private sending supports confirmed text DMs and protocol delivery/read receipts. Queue/notification acceptance is not end-to-end delivery confirmation; the sender app must actually receive the encrypted delivered/read receipt. Welcome automation intentionally stays disabled.

See [InfoBot usage](infobot.md) for the changed public/private command gates, statuses and automatic-welcome explanation. Test Home → Back → Start and scroll through all 32 Short msgs, including Help/SOS, before confirming a send.

## DM recovery and new public messages

- [ ] With a connected phone, retry DM Send. If Connecting appears, wait for authentication and explicitly retry; record the final reason if it fails.
- [ ] Repeat with at least three discovered identities and verify actual delivery in both directions.
- [ ] Channels → #bluetooth → Write msg sends new public text; per-user Reply remains private.
- [ ] Cancel a public draft: no transmission; return to Read chat/Write msg with the previous focus.

## Manual Welcome msg

- [ ] Verify the method picker shows Short msg, Keyboard and Welcome msg.
- [ ] Select Welcome msg: review the shared introduction and destination; nothing is sent yet.
- [ ] Back restores Welcome msg focus; Cancel exits without transmission.
- [ ] Confirm a private welcome and verify the phone receives it only as a DM.
- [ ] Confirm a public welcome in #bluetooth and verify public reception.
- [ ] With Bot OFF, manual welcome remains available; automatic first-contact introduction stays suppressed.
