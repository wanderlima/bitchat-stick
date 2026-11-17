export const repo = 'https://github.com/wanderlima/bitchat-stick';
export const docs = `${repo}/blob/main/docs/`;
export const buildGuide = `${docs}10-build-deploy.md`;
export const description = 'Modular open-source Bluetooth chat firmware for ESP32. Initial support is the M5StickC Plus2 and the optional Mini JoyC HAT.';
export const features = [
  ['01 / Find your people', 'See nearby Bitchat peers and their connection status. Open a private conversation directly from Nearby.'],
  ['02 / Public or private', 'Join the local #bluetooth conversation or send encrypted text DMs through authenticated sessions.'],
  ['03 / Make a quick reply', 'Choose from 32 short messages, type with the on-device QWERTY keyboard, or send a welcome message.'],
  ['04 / A little help, built in', 'InfoBot answers !info, !rules, !help and !about in public chat or authenticated DMs. These are Stick-specific commands.'],
];
export const facts = [
  ['Firmware', 'Modular ESP32 core · one profile per build'],
  ['Board', 'Initial support: M5StickC Plus2'],
  ['Controls', 'Buttons · optional Mini JoyC HAT'],
  ['Display', '135 × 240 pixels'],
  ['Connection', 'Bluetooth Low Energy'],
  ['Install', 'PlatformIO Core + USB'],
  ['Project', 'Open source · by Wander Lima'],
];
export const steps = [
  ['Get the project', 'Clone the repository and follow the setup guide.'],
  ['Build for Plus2', 'Compile the m5stickc-plus2 environment with PlatformIO.'],
  ['Flash over USB', 'Choose your serial port, upload the firmware and run the device checklist. Uploading replaces the installed firmware.'],
];
export const statuses = [
  ['Implemented', 'Peer discovery, public chat, encrypted DMs, InfoBot and JoyC navigation. Network visibility and message sending have been tested on real hardware. The owner has confirmed that it works with Bitchat on iPhone.'],
  ['Needs more device testing', 'Indirect relay, range, background behavior and battery life still need device testing.'],
  ['Current boundaries', 'Text-focused, with a small RAM-only history. No private groups, media, Nostr channels, OTA updates or persistent chat history yet.'],
];
export const faqs = [
  ['What is Bitchat Stick?', 'Bitchat Stick is modular open-source firmware for ESP32. The initial supported device is the M5StickC Plus2, with the Mini JoyC HAT as an optional peripheral. It joins the local Bitchat Bluetooth network with public text chat, private messages and physical controls.'],
  ['Does Bitchat Stick need internet access?', 'Local #bluetooth chat does not require Wi-Fi or cellular internet. Communication depends on compatible nearby devices and available BLE links. Internet-based Nostr channels are not implemented.'],
  ['Which hardware do I need?', 'To install today, use an M5StickC Plus2 and a USB data cable. The Mini JoyC HAT is an optional peripheral of that profile. The code is a shared ESP32 core with a board profile chosen at build time. Other boards are not supported yet, and the Plus2 binary is not a universal ESP32 image.'],
  ['Does it work with Bitchat on Android and iPhone?', 'Yes. The owner has confirmed that Bitchat Stick works with the iPhone app. Android discovery and the Noise handshake also have hardware evidence, and message sending has been confirmed on real hardware. This does not establish range, background behavior or indirect relay performance.'],
  ['Can it relay messages between devices?', 'The firmware supports bounded forwarding of verified public packets using TTL and duplicate detection. Controlled indirect relay tests are still pending. Arbitrary private-message routing is not implemented; the hero animation is conceptual.'],
  ['Are private messages encrypted and saved?', 'DMs use authenticated Noise sessions. The Stick keeps a small RAM-only message history that is lost on restart. Device identity keys are stored in unencrypted NVS, so this is not a claim of complete device security.'],
  ['How do I install the firmware?', 'Build the m5stickc-plus2 target with PlatformIO Core, select the serial port and upload over USB. Uploading replaces the installed firmware. Follow the build guide and device acceptance checklist.'],
  ['What does InfoBot do?', 'When enabled, InfoBot answers !info, !rules, !help and !about in signed public chat or authenticated DMs. These are Stick-specific text commands, not a standard command set built into every Bitchat app.'],
];
export const resources = [
  ['Build and USB flashing guide', buildGuide, 'terminal'],
  ['Device acceptance checklist', `${docs}13-device-checklist.md`, 'documentation'],
  ['Protocol, compatibility and security limits', `${docs}12-wire-contract.md`, 'documentation'],
  ['M5Stack hardware specifications', 'https://docs.m5stack.com/en/core/StickC-Plus2', 'documentation'],
  ['Upstream Bitchat project', 'https://github.com/permissionlesstech/bitchat', 'github'],
] as const;
