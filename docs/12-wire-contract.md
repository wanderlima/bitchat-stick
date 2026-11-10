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
- Service discovery is not authenticated association. The production Stick still only scans; no GATT participant or relay has been enabled by this framing work.

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

The current limits intentionally exclude large media streams. The assembler is not yet connected to GATT or Messages; a complete frame still needs cryptographic/session validation before delivery. Tests cover ordering, conflicts, resource limits, insufficient output capacity/retry, expiry/wrap and inner-sender mismatch.

## Security boundary

A parsed frame is **not an authenticated packet**. The codec only exposes signature bytes; it never validates them or creates a verified peer. Synthetic signature fixtures are intentionally not cryptographically valid.

The pinned Swift public-message handler requires a valid sender signature. Signing also has its own canonicalization: TTL zero, signature omitted, mutable RSR removed, serializer compression/padding policy applied. Raw unpadded encoding is therefore not a drop-in signing preimage. Identity-key binding, signing/verification, Noise XX, replay/freshness checks, compression and radio-integrated fragment handling must be implemented/tested before any untrusted frame can feed real Messages or InfoBot.

No new public/private message, welcome, forwarding, or channel claim is enabled by the codec. Traffic stays disabled until the relevant gates in [the two-phone plan](06-testing.md) pass.

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
