# BLE and command interface

Bitchat Stick does not expose an HTTP or WebSocket API. The external interfaces are the Bitchat BLE GATT service and the InfoBot text commands carried inside validated packets. Field-level framing, fixtures, and pinned upstream SHAs are specified in [the wire contract](wire-contract.md). This page records the contract the current firmware advertises and enforces.

## GATT

Defined in `src/Transport.cpp` and matched by `src/Scanner.cpp`:

| Item | Value |
| --- | --- |
| Mainnet service | `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C` |
| Testnet service, scan filter only | `F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5A` |
| Characteristic | `A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D` |
| Characteristic properties | Read, Write, Write without response, Notify |
| Advertised name | `Bitchat Stick` |
| Requested MTU | 512 |
| Subscribed links | 2. Extra connections are disconnected |
| Inbound event | At most 512 bytes, copied out of the NimBLE callback |
| Outbound frame | At most 4096 bytes, six queued frames |

The phone is the BLE central. The Stick is the peripheral. A discovered service UUID does not authenticate the peer and does not authorize InfoBot.

Notifications are paced at 20 ms. Frames larger than the peer MTU are split with the fragment type the transport already uses. Queue acceptance means the frame was copied into the local outbound queue, not that a phone displayed it.

## Frame subset

`include/WireCodec.h` views a complete frame as version, type, TTL, an 8-byte timestamp, flags, payload length, an 8-byte sender, an optional 8-byte recipient, an optional v2 route, the payload, and an optional 64-byte signature. Supported flag bits are recipient `0x01`, signature `0x02`, compression `0x04`, route `0x08`, and RSR `0x10`. Unknown flag bits, a v1 route flag, and arbitrary trailing bytes are rejected. Compression expands raw DEFLATE only into a caller-provided buffer after a bounded declared size check.

The codec does not verify signatures. `src/PacketSecurity.cpp` does that before the mesh treats a frame as authentic. Signing canonicalization is not the raw unpadded frame; the wire-contract document records that distinction.

Fragment reassembly accepts the 13-byte prefix used by the pinned upstream format and holds two assemblies of at most 32 fragments. Nested fragments are rejected. Reassembly is not authentication.

## InfoBot commands

Text commands are matched by `src/InfoBot.h` after trimming whitespace and folding ASCII case. A command is answered only when the mesh has already accepted it as a recent signed public message or as an authenticated DM. The reply uses the same audience as the command: public in, public out; private in, private out.

| Command | Alias |
| --- | --- |
| `!info` |  |
| `!rules` | `!regras` |
| `!help` | `!ajuda` |
| `!about` |  |

Anything else produces no reply. Per-peer cooldown is three seconds. Global cooldown is one second. There is no HTTP admin interface for these commands.

## Identity

`Identity::begin` loads or creates the Stick's Noise and signing keys in NVS. Peer key pins are RAM-only and expire. They are not a persistent trust-on-first-use store and they are not a check that a human owns that key.

## Related Documentation

- [Architecture](architecture.md)
- [Features](features.md)
- [Wire contract and fixtures](wire-contract.md)
- [InfoBot troubleshooting](infobot.md)
- [Compatibility investigation](compatibility.md)
