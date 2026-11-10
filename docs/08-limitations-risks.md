# Limitations and risks

| Risk/limitation | Effect | Mitigation / advancement condition |
|---|---|---|
| Mobile protocol changes | Discovery succeeds while messaging fails | Pin builds/SHAs; compare codecs; maintain a tested matrix |
| C3/C6/S3 code on classic ESP32 | Build, BLE, or power failures | Minimal Plus2 proof before broad porting |
| Two phones still directly connected | False relay demonstration | Negative controls, unique nonces, path logs |
| Nostr confused with mesh | False offline-channel promise | Show transport; keep Wi-Fi outside MVP |
| Presence confused with joining | Repeated welcomes/spam | Block until a reliable event exists |
| Private ciphertext | Bot cannot legitimately read it | Opaque relay; explicit participation/keys |
| BLE load/interference | Loss, latency, unstable connections | Limits, backoff, foreground/background testing |
| Fragment/memory exhaustion | Crash or denial of service | Bounded pools, limits, expiry, minimum-heap metrics |
| Replay/spoofing/duplicate names | Wrong replies and misleading counts | Verify signatures/binding; per-identity limits |
| Logs/history | Privacy exposure and tracking | Minimal logs, short retention, synthetic data |
| Small battery/display load | Short runtime | Measure profiles; distinguish screen-off from sleep |
| Wrong HOLD/power handling | Unexpected reset or shutdown | Validate Plus2-specific abstractions on USB/battery |
| Third-party licenses | Incorrect redistribution | Review licenses/dependencies at the imported SHA |
| Text length/Unicode | Clipping or broken text | Paged details, bounded rendering, physical readability checks |

BLE does not guarantee delivery, fixed range, anonymity, or availability. The [official whitepaper](https://github.com/permissionlesstech/bitchat/blob/main/WHITEPAPER.md) discusses observable metadata and persistent identity. Do not call the device invisible or untraceable. Passive scanning without advertising is not a general invisibility guarantee.

The Plus2 has no built-in LoRa radio. Projects with LoRa backhaul require additional hardware and scope. Relay may extend useful connectivity through intermediate links; it does not amplify the phones' radio power.

No local measurements establish range, sustained BLE stability, runtime heap, battery life, or Bitchat messaging compatibility. Native tests/builds and the owner's initial UI use are narrower evidence. Keep protocol proof ahead of relay/bot claims.
