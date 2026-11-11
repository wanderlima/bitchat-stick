# Pinned wire contract and interoperability evidence

Date: **2026-11-10**. This document records source/host evidence, not a completed phone-to-Stick messaging test.

## Reference versions

- Swift source: [permissionlesstech/bitchat, 04995f68e470a18d1fc99f86fa326c2a87bab4f2](https://github.com/permissionlesstech/bitchat/tree/04995f68e470a18d1fc99f86fa326c2a87bab4f2).
- Android source: [permissionlesstech/bitchat-android, 14ab73c4afe57deb9a1e7987ac9814b307c2f6ab](https://github.com/permissionlesstech/bitchat-android/tree/14ab73c4afe57deb9a1e7987ac9814b307c2f6ab).
- Connected Android: Samsung SM-A366E, Android 16, `com.bitchat.droid` **2.2.0 / versionCode 42**, installed through Google Play. Queried through ADB.
- Paired iPhone: iPhone 15 Pro, `chat.bitchat` **1.8.0 / build 1**. Queried through CoreDevice. iOS system version has not been recorded.
- These installed app versions do not by themselves prove correspondence to the source SHAs. Exact release-to-source mapping and over-the-air behavior remain unverified.

## BLE contract observed in Swift source

- Mainnet service: `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C`.
- Testnet service: `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5A`.
- Characteristic: `A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D`.
- Service discovery is not authenticated association. The Stick now advertises mainnet GATT and processes validated packets; a service match alone still does not authorize chat or bot actions.

## Implemented framing subset

`include/WireCodec.h` and `src/WireCodec.cpp` implement non-owning, allocation-free views of complete frames:

1. Version/type/TTL: one byte each.
2. Timestamp: eight bytes, big-endian.
3. Flags at byte 11.
4. Payload length: two bytes for v1, four for v2, big-endian; payload only, excluding route/signature.
5. Sender ID: eight bytes.
6. Optional recipient: eight bytes.
7. Optional v2 route: one-byte hop count followed by eight bytes per hop.
8. Payload, followed by an optional 64-byte signature.

Supported flags: recipient `0x01`, signature `0x02`, route `0x08`, RSR `0x10`. Compression `0x04` is parsed with a bounded declared output size; raw DEFLATE is expanded only into a caller-provided buffer. The embedded subset caps the complete input at **4096 bytes** and payload at **2048 bytes**, below upstream media/text limits. Oversized input is explicitly rejected, never silently truncated.

The decoder validates all reads before accessing bytes and publishes views only after success. It accepts exact frames or PKCS#7-style tails of 1–255 equal-valued bytes. Unlike the permissive upstream decoder, arbitrary trailing bytes, unknown flag bits, a v1 route flag and zero-hop route fields are rejected. The encoder outputs unpadded frames and preserves exact compressed wire preambles/bytes when present and checks capacity/field sizes before writing. Input views must remain alive; encoder source and destination buffers must not overlap.

## Bounded fragment reassembly

`FragmentAssembler` implements the upstream 13-byte fragment prefix: 8-byte stream ID, 2-byte big-endian index, 2-byte big-endian count and original type. It holds at most **two assemblies**, **32 fragments per assembly**, **1024 bytes per fragment chunk** and **4096 reassembled bytes**. Storage is fixed; no allocation or unbounded map is used.

Out-of-order fragments are accepted. Exact duplicates do not consume space or extend the 30-second lifetime; conflicting duplicate data or metadata invalidates the assembly. Full capacity rejects new streams. Expiry is wrap-safe. Completion validates the recovered wire frame and requires its sender/type to match the assembly envelope. This is consistency checking, not cryptographic sender authentication. Nested fragments are rejected; recovered compressed frames still pass declared-size and bounded expansion checks.

The current limits intentionally exclude large media streams. The assembler is connected to GATT; recovered frames still pass cryptographic/session validation before delivery. Tests cover ordering, conflicts, resource limits, insufficient output capacity/retry, expiry/wrap and inner-sender mismatch.

## Security boundary

A parsed frame is **not an authenticated packet**. The codec only exposes signature bytes; it never validates them or creates a verified peer. The framing-only fixtures use opaque synthetic signature bytes; the separate signed-packet fixtures contain valid independently generated signatures.

The pinned Swift public-message handler requires a valid sender signature. Signing also has its own canonicalization: TTL zero, signature omitted, mutable RSR removed, serializer compression/padding policy applied. Raw unpadded encoding is therefore not a drop-in signing preimage. Identity-key binding, signing/verification, Noise XX, replay/freshness checks, compression and radio-integrated fragment handling must be implemented/tested before any untrusted frame can feed real Messages or InfoBot.

Framing alone is not authentication. The integrated mesh enables only the validated source subset described below; radio acceptance is tracked separately in [the two-phone plan](06-testing.md).

## Independent serializer fixtures

`test/fixtures/bitchat-wire-vectors.txt` contains five frames generated by executing the **actual Swift BitFoundation serializer** at the pinned SHA:

- Uncompressed public v1.
- v1 with an opaque synthetic signature.
- Directed/signed v1.
- Directed/signed v2 with two route hops.
- Signed v1 with serializer padding.

The C++ test decodes each frame, verifies fields, re-encodes the unpadded prefix byte-for-byte, rejects every truncated prefix, checks invalid flags/lengths/padding and runs 10,000 deterministic hostile frames under AddressSanitizer/UndefinedBehaviorSanitizer. This is independent serializer evidence for the stated subset, not Android-radio or cryptographic certification.

Run all eleven host suites from the root:

```sh
sh tools/test-native.sh
```

The fixture generator source is `tools/generate-protocol-fixtures.swift`. To regenerate, create a temporary Swift executable package that depends on the pinned checkout's `localPackages/BitFoundation` product, copy the generator into its executable target and invoke it with an output JSON path. Keep `localPackages/BitLogger` from the same checkout available. Convert each JSON `name`/`hex` pair to a line in the fixture file, retaining the source SHA and synthetic-signature warning. No third-party implementation is copied into the firmware, and no remote dependency is needed for ordinary host tests/builds.

## Remaining release gates

- Confirm release/source mapping and the wire behavior of Android 2.2.0 and iOS 1.8.0.
- Add the missing codec/compression/reassembly/security layers with independently generated vectors.
- Implement bounded GATT lifecycle and recovery; keep radio callbacks separate from UI/model mutation.
- Prove authenticated phone↔Stick messages before channel/bot participation.
- Prove the indirect route, TTL/deduplication and negative controls before enabling relay.
- Establish reliable authenticated entry events before enabling welcomes; a scan result never qualifies.

## Cryptographic primitives

Monocypher **4.0.2**, pinned commit `0d85f98c9d9b0227e42cf795cb527dff372b40a4`, provides Ed25519, X25519 and IETF ChaCha20-Poly1305. It is an external PlatformIO dependency; its dual BSD-2-Clause/CC0 notice is reproduced in `monocypher-license.md`. SHA-256 uses the existing ESP32 mbedTLS implementation; host tests use the macOS system implementation. No custom elliptic-curve or cipher algorithm is introduced.

Host tests cover RFC 4231 HMAC, RFC 8032 Ed25519, RFC 7748 X25519, all-zero shared-secret rejection, and AEAD output compared independently with Apple CryptoKit. Each Noise cipher operation initializes a fresh IETF AEAD context with the protocol nonce; Monocypher's optional streaming rekey state is not retained across packets. These primitives alone do not enable or authenticate live messages.

## Noise XX session layer

`NoiseSession` implements the exact `Noise_XX_25519_ChaChaPoly_SHA256` empty-payload handshake used by the pinned apps. It binds the remote static key to the claimed eight-byte peer ID via SHA-256 before exposing an established session. Synthetic deterministic fixtures were generated by executing the Android client's actual Southern Storm Java implementation, not by the C++ session: all three handshake messages (32/96/64 bytes), handshake hash and both first transport ciphertexts match byte-for-byte.

Transport uses the Bitchat four-byte big-endian nonce prefix, with the Noise nonce encoded little-endian inside the IETF cipher nonce. A 1024-message replay window accepts authenticated out-of-order delivery; invalid tags do not consume a nonce. Sessions refuse sends/receives at the 10,000-message bound so the integration must rekey. Handshake/transport secrets are wiped on reset, destruction or failure. Connection lifecycle, one-hour rekey and timeout scheduling are integration responsibilities, not reasons to accept an unbound key.

Run `tools/GenerateNoiseVectors.java` against the pinned Android `noise/southernstorm` sources to regenerate `test/fixtures/noise-java-vectors.txt`. Fixture private keys are deterministic synthetic test data only. Host tests cover identity mismatch, ciphertext changes, replay, out-of-order delivery and successful retry after a failed tag. Live app interoperability remains a separate device check.

## Packet signatures and compressed payloads

`PacketSecurity` constructs the actual upstream signing preimage: TTL zero, signature absent, RSR removed, exact wire payload/route, then the serializer's bucket/padding policy. Compressed wire bytes are retained, avoiding compressor-dependent re-encoding. Three additional fixtures were generated by the official Swift serializer and signed by Apple CryptoKit: short, compressed and routed messages. The C++ verifier accepts them, their canonical bytes match exactly, and CryptoKit independently accepts signatures generated locally. CryptoKit signatures can be randomized; equality of signature bytes is deliberately not used as a verification test.

Raw DEFLATE expansion uses the existing pinned M5GFX miniz implementation, with at most 2048 output bytes and exact declared-length checking. Invalid or oversized streams never publish partial plaintext. `verifyPacket` proves a signature only against the supplied key; the integration must still validate peer-ID/key binding, pinned keys, freshness and replay before delivering messages.

## Live mesh model

`Mesh` owns twelve signed peer records, two initiator/responder Noise sessions, twelve 192-byte display messages, a 64-entry deduplication cache, and bounded fragment storage. Announcements bind the sender ID to SHA-256 of the advertised Noise key and verify the advertised Ed25519 key. This is self-consistency, not out-of-band verification. Keys are pinned in RAM until peer expiry; peer trust is not persisted.

Only a successful identity-bound Noise handshake followed by authenticated peer state (`0x21`, version 1) authorizes InfoBot commands. Public commands must also be fresh within 30 seconds, with at most five seconds of future skew. Replies have a 30-second per-peer and 3-second global cooldown. `!info`, `!rules`, `!help`, `!regras`, and `!ajuda` are supported. No advertisement or reconnect produces a welcome message.

The public BLE channel accepts UTF-8 signed text, rejects malformed/control input, and substitutes unsupported display glyphs. Display history is RAM-only and long messages are explicitly truncated. Signed announcements/public text are relayed with reduced TTL after verification and deduplication. Private media, private groups, gossip synchronization, Nostr, and opaque multi-hop Noise forwarding are outside this implementation. Noise initiates automatically after signed discovery and also handles incoming direct-link handshakes; it is not an arbitrary routed session manager. Crossed initiations keep the smaller peer ID as initiator. Reconnect/timeout clears authenticated state; sessions expire after one hour.

Local identity uses a versioned NVS record and never rotates automatically when storage is corrupt. NVS is not encrypted: physical flash extraction can disclose keys. No hardware secure-storage guarantee is made.

## GATT transport and clock

The Stick advertises the official mainnet service and exposes the official read/write/write-without-response/notify characteristic. It acts as a peripheral; mobile apps initiate connections. Two subscribed links, twelve inbound 512-byte events, and six outgoing 4096-byte frames bound memory use. Callbacks only enqueue; parsing, crypto, drawing and sending run in the application loop. Notifications are paced at most once every 20 ms and are fragmented for each negotiated MTU. MTU values too small for the fragment envelope produce an explicit error/drop, never a truncated frame. Queue acceptance is not delivery acknowledgement.

UTC is read from the Plus2 RTC. The build timestamp repairs an unset/older RTC and initializes system time; the RTC is preserved when newer. A failed RTC falls back to build time plus uptime and is visible in Diagnostics. A long delay between build and first use with no working RTC can make packets stale: rebuild/upload to repair the time floor. No phone timestamp is trusted as a clock source. Do not distribute an old binary as a reliable clock-setting mechanism.

## Authenticated direct-message reader

Noise payload type `0x01` is decoded as the official private-text TLV packet: one-byte type/length, required message ID (`0x00`) and content (`0x01`). The decoder requires authenticated peer state, nonempty valid UTF-8, unique known fields, a message ID of at most 64 bytes and exact lengths. IDs are deduplicated together with sender identity. Received text enters the same bounded RAM history with a DM marker; long content is explicitly truncated for display. Decrypted working buffers are wiped on every exit.

A successful store sends an encrypted `0x03` delivered receipt carrying the original message ID. Retransmissions can be acknowledged without duplicating history. Receipt means received/stored, not read. No DM is sent to the public relay or InfoBot. Private media/groups, courier envelopes and DM composition remain unsupported. Native tests cover private decoding, receipts, re-encrypted duplicates, malformed TLVs and isolation of `!info` inside a DM.

## Inbox, unread state and read receipts

Each retained message has a local sequence ID and unread flag; public/private filters share the twelve-entry bound. Outgoing messages start read. The unread envelope/counts derive from retained incoming entries, so overwriting history also removes its old unread entry. UI focus follows the sequence ID rather than a shifting ring offset. Opening the wrong inbox or remaining on Home does not mark a DM read.

Private entries retain at most 64 bytes of message ID plus the sender ID. Once displayed, a pending read receipt uses Noise payload `0x02` followed by the exact original message-ID bytes, matching the official payload factory. At most one live-session encryption/enqueue attempt is made per second; unavailable sessions leave the receipt pending. Queue failures retry, and successfully queued receipts are not repeatedly emitted on redraw. Tests verify the exact decrypted receipt, no receipt before opening, per-message counts, queue retry, filtering, focus stability and eviction. No public read receipt is generated.

## Outgoing private text

`sendDirect` requires a live session and authenticated peer state for the copied eight-byte recipient ID. It validates 1–192 bytes of nonblank UTF-8 text, generates a random 128-bit lowercase-hex message ID, encodes Noise type 0x01 with TLV 0x00 (ID) and 0x01 (content), and emits an addressed 0x11 packet only to that session link. The built-in editor emits ASCII; the transport API validates UTF-8. No public fallback is permitted. Only successful queue acceptance adds an outgoing history entry. Failed enqueue consumes a transport nonce but creates no history entry; the replay window permits gaps.

An outgoing entry starts Queued. Authenticated encrypted type 0x03 (delivered) and 0x02 (read) payloads update only an outgoing DM with the same message ID and peer ID. Read cannot regress to Delivered. Evicted/unknown IDs do not change other entries. Outgoing messages never schedule a read receipt. The sender performs no offline storage, automatic resend or end-to-end delivery guarantee. Android/iOS reception is still a device acceptance check, even though host tests decrypt the exact outgoing frame.

Upstream encoding reference: [NoiseEncrypted.kt](https://github.com/permissionlesstech/bitchat-android/blob/main/app/src/main/java/com/bitchat/android/model/NoiseEncrypted.kt).
