#include "../src/Ui.cpp"
#include "SurfaceProbe.h"
#include <cassert>
#include <fstream>
#include <Preferences.h>
#include "PacketSecurity.h"
namespace stick {
JoycSample fakeJoyc;
JoycSample pollJoyc(uint32_t, bool) { return fakeJoyc; }
bool setJoycLed(uint8_t) { return true; }
bool transportReady(){return true;}
size_t transportLinks(){return 1;}
uint32_t transportDrops(){return 0;}
const char* transportStatus(){return "Connected";}
bool clockRtc(){return true;}
CivilTime clockCivil() { return {2026, 11, 18, 14, 32}; }
bool clockSetCivil(CivilTime time) { return civilTimeValid(time); }
bool scanning = true, fault = false;
bool scannerActive() { return scanning; }
bool scannerFault() { return fault; }
uint32_t scannerDrops() { return 2; }
uint32_t scannerReports() { return 123; }
}
int main(int argc, char** argv) {
  assert(argc == 2);
  using namespace stick;
  PeerTable peers;
  beginUi();
  battery = 72;
  voltage = 4000;
  auto save = [&](const char* name) {
    draw(peers, 3000);
    assert(surfaceProbe().boundsOk);
    std::ofstream out(std::string(argv[1]) + "/" + name + ".ops");
    for (const auto& op : surfaceProbe().ops) out << op << '\n';
  };
  auto saveEmpty = [&](const char* name) {
    draw(PeerTable{}, 3000);
    assert(surfaceProbe().boundsOk);
    std::ofstream out(std::string(argv[1]) + "/" + name + ".ops");
    for (const auto& op : surfaceProbe().ops) out << op << '\n';
  };

  screen = Screen::Splash;
  save("start");
  save("07-splash");

  screen = Screen::Peers;
  detail = false;
  saveEmpty("nearby-empty");
  fault = true;
  saveEmpty("nearby-scan-error");
  fault = false;
  scanning = false;
  screen = Screen::Dashboard;
  selection = 0;
  save("home-ble-idle");
  scanning = true;
  fault = true;
  save("home-ble-err");
  fault = false;

  Observation value{};
  std::strcpy(value.address, "AA:BB:CC:DD:EE:FF");
  std::strcpy(value.name, "Test phone");
  value.rssi = -64;
  value.network = Network::Mainnet;
  value.seenAt = 1000;
  peers.observe(value);
  screen = Screen::Peers;
  detail = false;
  peerSelection = PeerSelection{};
  save("nearby-advertisement");
  detail = true;
  save("nearby-details");

  screen = Screen::Dashboard;
  selection = 0;
  save("home");
  save("home-nearby");
  save("01-dashboard");
  selection = 1;
  save("home-channels");
  selection = 2;
  save("home-infobot");
  selection = 3;
  save("home-scrolled");
  save("home-settings");

  screen = Screen::Channels;
  detail = false;
  selection = 0;
  save("channels");
  save("03-channels");
  detail = true;
  selection = 1;
  save("bluetooth-menu");
  save("18-bluetooth-menu");
  screen = Screen::Messages;
  messageScope = MessageScope::Public;
  messageOffset = 0;
  detail = false;
  save("public-empty");
  messageScope = MessageScope::Direct;
  save("dm-empty");

  Identity identity;
  auto entropy = [](void* out, size_t n) {
    for (size_t i = 0; i < n; ++i) static_cast<uint8_t*>(out)[i] = uint8_t(i + 17);
  };
  assert(identity.begin(entropy));
  mesh().begin(identity, entropy, [](wire::Bytes, uint16_t, uint16_t) { return true; });
  mesh().tick(3000, 1800000000000ULL);
  assert(mesh().sendText("Hello from the Stick"));
  fake_preferences::identityExists = false;
  fake_preferences::identityLength = 0;
  Identity other;
  auto otherEntropy = [](void* out, size_t n) {
    for (size_t i = 0; i < n; ++i) static_cast<uint8_t*>(out)[i] = uint8_t(i + 59);
  };
  assert(other.begin(otherEntropy));
  uint8_t announcement[90] = {1, 10, 'T', 'e', 's', 't', ' ', 'p', 'h', 'o', 'n', 'e', 2, 32};
  std::memcpy(announcement + 14, other.dhPublic(), 32);
  announcement[46] = 3;
  announcement[47] = 32;
  std::memcpy(announcement + 48, other.signingPublic(), 32);
  auto incoming = [&](uint8_t type, wire::Bytes payload) {
    uint8_t encoded[4096], scratch[4096], sig[64];
    size_t n = 0;
    wire::FrameView frame;
    frame.type = type;
    frame.ttl = 7;
    frame.timestamp = 1800000000000ULL;
    frame.sender = {other.id(), 8};
    frame.payload = payload;
    assert(wire::signPacket(frame, other.signingSecret(), scratch, sizeof(scratch), sig));
    frame.signature = {sig, 64};
    assert(wire::encode(frame, encoded, sizeof(encoded), n) == wire::Error::None);
    mesh().receive({encoded, n}, 1);
  };
  incoming(1, {announcement, 80});
  const char* text = "Hey, are you nearby?";
  incoming(2, {reinterpret_cast<const uint8_t*>(text), std::strlen(text)});
  uint8_t longText[192];
  std::memset(longText, 'x', sizeof(longText));
  longText[0] = 'A';
  incoming(2, {longText, sizeof(longText)});

  screen = Screen::Dashboard;
  selection = 0;
  save("home-unread");
  screen = Screen::Peers;
  detail = false;
  selection = 0;
  save("nearby");
  save("02-nearby-peers");
  save("nearby-signed");
  detail = true;
  save("nearby-signed-detail");

  screen = Screen::Channels;
  detail = false;
  selection = 0;
  save("channels-unread");
  screen = Screen::Messages;
  messageScope = MessageScope::Public;
  messageOffset = 0;
  detail = false;
  activeMessage = 0;
  messageOffset = 1;
  save("received");
  messageOffset = 0;
  save("04-channel-messages");
  detail = true;
  textLine = 0;
  save("message-reader");
  messageOffset = 1;
  textLine = 1;
  detail = true;
  save("text-scroll");
  messageScope = MessageScope::Direct;
  save("dm-empty-after");

  screen = Screen::Bot;
  selection = 0;
  detail = false;
  save("infobot");
  save("05-infobot");
  selection = 4;
  save("infobot-scrolled");
  mesh().setBotEnabled(true);
  save("infobot-enabled");
  for (size_t i = 0; i < 4; ++i) {
    selection = i;
    detail = true;
    char name[16];
    std::snprintf(name, sizeof(name), "infobot-cmd-%u", unsigned(i));
    save(name);
  }
  detail = false;

  screen = Screen::Settings;
  selection = 0;
  settingsSaved = true;
  save("settings");
  save("06-settings");
  save("settings-top");
  selection = 3;
  save("settings-mid");
  selection = 6;
  save("settings-date");
  settingsSaved = false;
  save("settings-not-saved");
  settingsSaved = true;

  screen = Screen::Brightness;
  brightnessIndex = 2;
  selection = 2;
  save("brightness");
  save("08-brightness");
  selection = 0;
  save("brightness-5");
  selection = 4;
  save("brightness-100");
  selection = 3;
  save("brightness-focus");

  screen = Screen::Battery;
  battery = 72;
  voltage = 4000;
  save("battery");
  save("10-battery");
  battery = -1;
  voltage = 0;
  save("battery-unknown");
  battery = 72;
  voltage = 4000;

  screen = Screen::Diagnostics;
  joycAxes = {};
  save("diagnostics");
  save("11-diagnostics");
  joycAxes.valid = true;
  joycAxes.x = 0;
  joycAxes.y = 12;
  save("diagnostics-joy");

  screen = Screen::Sound;
  selection = 0;
  preferenceFlags = 0;
  save("sound");
  save("12-sound");
  screen = Screen::Joystick;
  joycAxes = {};
  save("joyc-no-hat");
  save("13-joyc-hat");
  joycAxes.valid = true;
  joycAxes.buttonValid = false;
  preferenceFlags = JoycClick;
  save("joyc-click-error");
  joycAxes.buttonValid = true;
  joycAxes.pressed = false;
  preferenceFlags = JoycClick | JoycLed;
  save("joyc-ready");

  screen = Screen::About;
  save("about");
  save("09-about");

  screen = Screen::DateTime;
  selection = 0;
  clockSaved = true;
  save("date-time");
  clockSaved = false;
  save("date-time-not-saved");
  clockSaved = true;
  edited = clockCivil();
  field = 0;
  screen = Screen::SetDate;
  save("set-date");
  field = 2;
  save("set-date-year");
  clockSaved = false;
  save("set-date-not-saved");
  clockSaved = true;
  field = 0;
  screen = Screen::SetTime;
  save("set-time");

  composer.open(other.id(), "Test phone");
  for (size_t method = 0; method < 3; ++method) {
    screen = Screen::Reply;
    selection = method;
    privateDraft = true;
    char name[24];
    std::snprintf(name, sizeof(name), "reply-method-%u", unsigned(method));
    save(name);
  }
  screen = Screen::Reply;
  selection = 0;
  save("14-reply");
  openPublicComposer();
  save("write-public");
  save("19-public-compose");
  composer.open(other.id(), "Test phone");
  privateDraft = true;
  screen = Screen::ShortMessages;
  selection = 0;
  save("15-short-messages");

  for (size_t index = 0; index < MessageComposer::presetCount; ++index) {
    composer.open(other.id(), "Test phone");
    composer.preset(index);
    queuedMessage = 0;
    sendFailed = false;
    privateDraft = true;
    screen = Screen::ShortMessages;
    selection = index;
    char name[24];
    std::snprintf(name, sizeof(name), "short-%u", unsigned(index));
    save(name);
    screen = Screen::Confirm;
    selection = 1;
    std::snprintf(name, sizeof(name), "review-%u-cancel", unsigned(index));
    save(name);
    selection = 0;
    std::snprintf(name, sizeof(name), "review-%u-send", unsigned(index));
    save(name);
    queuedMessage = mesh().message(0) ? mesh().message(0)->sequence : 1;
    selection = 0;
    std::snprintf(name, sizeof(name), "queued-%u", unsigned(index));
    save(name);
    queuedMessage = 0;
  }

  composer.open(other.id(), "Test phone");
  std::snprintf(composer.draft, sizeof(composer.draft), "Hi! You there?");
  composer.mode = 0;
  screen = Screen::Keyboard;
  selection = 0;
  save("keyboard");
  save("16-keyboard");
  save("keyboard-sample");
  save("writing");
  composer.mode = 1;
  save("keyboard-upper");
  composer.mode = 2;
  save("keyboard-symbols");
  composer.mode = 0;
  composer.draft[0] = 0;
  selection = 33;
  save("keyboard-empty");
  std::memset(composer.draft, 'q', 192);
  composer.draft[192] = 0;
  selection = 16;
  save("keyboard-limit");

  std::snprintf(composer.draft, sizeof(composer.draft), "Hi! You there?");
  screen = Screen::Confirm;
  selection = 1;
  queuedMessage = 0;
  sendFailed = false;
  privateDraft = true;
  save("confirm");
  save("17-confirm");
  selection = 0;
  save("confirm-send");
  privateDraft = false;
  selection = 1;
  save("confirm-public");
  std::snprintf(composer.draft, sizeof(composer.draft), "%s", kBotIntroduction);
  privateDraft = true;
  save("confirm-welcome");
  std::memset(composer.draft, 'M', 180);
  composer.draft[180] = 0;
  save("confirm-long");
  const char* failures[] = {"Connecting", "Peer gone", "No phone", "Not ready", "Bad text"};
  const char* failureNames[] = {"not-sent-connecting", "not-sent-peer-gone", "not-sent-link", "not-sent-ready", "not-sent-text"};
  sendFailed = true;
  for (int i = 0; i < 5; ++i) {
    sendFailure = failures[i];
    save(failureNames[i]);
  }
  sendFailed = false;
  privateDraft = false;
  std::snprintf(composer.draft, sizeof(composer.draft), "Hi! You there?");
  queuedMessage = 0;
  for (size_t i = 0; i < mesh().messageCount(); ++i) {
    if (mesh().message(i)->outgoing) {
      queuedMessage = mesh().message(i)->sequence;
      break;
    }
  }
  save("queued");
  save("20-public-confirm");
  save("queued-public");
  privateDraft = true;
  save("queued-dm");
}
