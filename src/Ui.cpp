#include "Ui.h"
#include "Scanner.h"
#include "NavigationInput.h"
#include <M5Unified.h>
#include <cstdio>
#include <cstring>

namespace stick {
namespace {
constexpr uint16_t bg = TFT_BLACK, fg = TFT_WHITE, muted = 0xDEFB;
constexpr uint16_t green = 0x87F0, amber = TFT_YELLOW;
constexpr uint32_t screenTimeout = 30000;
enum class Screen { Splash, Dashboard, Peers, Channels, Messages, Bot, Settings, Brightness, Battery, About, Diagnostics };
Screen screen = Screen::Splash;
size_t selection = 0, peerIndex = 0;
constexpr uint8_t brightnessLevels[] = {13, 64, 128, 191, 255};
constexpr unsigned brightnessPercent[] = {5, 25, 50, 75, 100};
size_t brightnessIndex = 2;
uint8_t brightness = brightnessLevels[brightnessIndex];
bool sleeping = false, detail = false;
uint32_t lastInput = 0, lastDraw = 0, lastBattery = 0;
int battery = -1, voltage = 0;
NavigationInput input;
const lgfx::IFont* body = &fonts::FreeSansBold9pt7b;
const lgfx::IFont* compact = &fonts::Font2;

void label(const char* value, int y, uint16_t color = fg, int x = 12,
           int width = 111, const lgfx::IFont* font = nullptr) {
  auto& d = M5.Display;
  d.setFont(font ? font : body);
  d.setTextSize(1);
  char clipped[48]{};
  std::snprintf(clipped, sizeof(clipped), "%s", value);
  // All labels/radio names here are ASCII. Clip to the measured font width.
  const int available = width < 129 - x ? width : 129 - x;
  if (d.textWidth(clipped) > available) {
    size_t length = std::strlen(clipped);
    while (length && (length > sizeof(clipped) - 4 ||
                     d.textWidth(clipped) + d.textWidth("...") > available))
      clipped[--length] = '\0';
    std::strcat(clipped, "...");
  }
  d.setTextColor(color); // transparent background preserves inverse selection.
  d.setCursor(x, y);
  d.print(clipped);
}
void centered(const char* text, int y, uint16_t color = fg,
              const lgfx::IFont* font = nullptr) {
  auto& d = M5.Display;
  d.setFont(font ? font : body);
  d.setTextSize(1);
  d.setTextColor(color);
  d.setCursor((135 - d.textWidth(text)) / 2, y);
  d.print(text);
}
void item(const char* text, int y, bool selected) {
  if (selected) M5.Display.fillRect(4, y - 1, 127, 22, fg);
  label(text, y, selected ? bg : fg, 8, 119);
}
void panel(int top, int height, bool peer = false) {
  // Group related information without shrinking the text.
  M5.Display.fillRect(4, top, 127, height, peer ? 0x0964 : 0x10A3);
  M5.Display.drawRect(4, top, 127, height, peer ? green : muted);
  if (peer) M5.Display.fillRect(4, top, 3, height, green);
}
void drawSplash() {
  auto& d = M5.Display;
  d.drawRect(43, 13, 49, 49, fg);
  centered("LOGO", 29, muted, compact);
  centered("BITCHAT", 74, green, &fonts::FreeSansBold12pt7b);
  centered("STICK", 101, fg, &fonts::FreeSansBold12pt7b);
  centered("A: confirm", 139, fg, compact);
  centered("B: up", 158, fg, compact);
  centered("C: down", 177, fg, compact);
  centered("Hold B: back", 196, muted, compact);
  d.fillRect(4, 215, 127, 24, fg);
  centered("A: START", 217, bg);
}
void draw(const PeerTable& peers, uint32_t now) {
  auto& d = M5.Display;
  d.startWrite();
  d.fillScreen(bg);
  if (screen == Screen::Splash) { drawSplash(); d.endWrite(); return; }
  const uint16_t radioColor = scannerFault() ? TFT_RED : scannerActive() ? green : amber;
  d.fillRect(6, 7, 5, 5, radioColor);
  label(scannerFault() ? "BLE err" : scannerActive() ? "BLE on" : "BLE idle", 2,
        fg, 16, 59, compact);
  char value[48];
  const bool batteryKnown = battery >= 0 && battery <= 100;
  if (batteryKnown) std::snprintf(value, sizeof(value), "%d%%", battery);
  else std::snprintf(value, sizeof(value), "--%%");
  const uint16_t batteryColor = batteryKnown && battery < 20 ? amber : fg;
  d.setFont(compact);
  label(value, 2, batteryColor, 115 - d.textWidth(value), d.textWidth(value), compact);
  d.drawRect(119, 5, 11, 9, batteryColor);
  d.fillRect(130, 8, 2, 3, batteryColor);
  if (batteryKnown && battery > 0) d.fillRect(121, 7, (7 * battery + 99) / 100, 5, batteryColor);
  const char* title = "Home";
  switch (screen) {
    case Screen::Peers: title = "Nearby"; break;
    case Screen::Channels: title = "Channels"; break;
    case Screen::Messages: title = "Messages"; break;
    case Screen::Bot: title = "InfoBot"; break;
    case Screen::Settings: title = "Settings"; break;
    case Screen::Brightness: title = "Brightness"; break;
    case Screen::Battery: title = "Battery"; break;
    case Screen::About: title = "About"; break;
    case Screen::Diagnostics: title = "Diagnostics"; break;
    default: break;
  }
  label(title, 26);
  d.drawFastHLine(4, 48, 127, fg);
  switch (screen) {
    case Screen::Dashboard: {
      panel(51, 69);
      std::snprintf(value, sizeof(value), "BLE: %u", unsigned(peers.count())); label(value, 53);
      std::snprintf(value, sizeof(value), "Cand. %u", unsigned(peers.candidates())); label(value, 76, green);
      label("Relay: OFF", 100, amber, 12, 111, compact);
      panel(126, 109);
      const char* menu[] = {"Nearby", "Channels", "InfoBot", "Settings"};
      for (size_t i = 0; i < 4; ++i) item(menu[i], 130 + i * 26, i == selection);
      break;
    }
    case Screen::Peers: {
      const auto n = peers.count();
      if (!n) { label("No BLE seen", 65); label("yet", 88); label("Scanning...", 131, muted); }
      else {
        peerIndex %= n;
        const auto& obs = peers.at(peerIndex)->observation;
        panel(76, 159, true);
        std::snprintf(value, sizeof(value), "%u / %u", unsigned(peerIndex + 1), unsigned(n)); label(value, 52);
        if (detail) {
          // Full MAC in two readable lines instead of shrinking the font.
          char half[9]{};
          std::memcpy(half, obs.address, 8); label(half, 86, fg);
          label(obs.address + 9, 114, fg);
          std::snprintf(value, sizeof(value), "Addr type %u", unsigned(obs.addressType)); label(value, 150);
        } else {
          label(obs.name[0] ? obs.name : "Unnamed", 82, fg);
          std::snprintf(value, sizeof(value), "%d dBm", obs.rssi); label(value, 110);
          label(obs.network == Network::Other ? "Other BLE" : "Candidate", 138, green);
          if (obs.network != Network::Other)
            label(obs.network == Network::Mainnet ? "Mainnet?" : "Testnet?", 164, amber);
        }
        std::snprintf(value, sizeof(value), "%lus ago", (unsigned long)((now - obs.seenAt) / 1000)); label(value, 188, muted);
        label("Unverified", 215, amber, 12, 111, compact);
      }
      break;
    }
    case Screen::Channels:
      panel(56, 96, true);
      label("#bluetooth", 63, fg);
      label("Local BLE", 93);
      label("No RX/TX", 123, amber);
      panel(164, 71);
      label("Nostr not", 175, muted);
      label("supported", 205, muted);
      break;
    case Screen::Messages:
      panel(58, 177);
      label("No RX/TX", 65, amber);
      label("Messages", 97); label("not available", 120); label("yet", 143);
      label("Scanner only", 207, muted, 12, 111, compact);
      break;
    case Screen::Bot: {
      panel(52, 31);
      label("No RX/TX", 56, amber);
      const char* commands[] = {"!info", "!rules", "!help"};
      if (detail) {
        panel(92, 143, true);
        label(commands[selection], 100, green);
        label("No sending.", 133);
        label("Bot is", 166, muted); label("planned.", 194, muted);
      } else {
        panel(92, 88);
        for (size_t i = 0; i < 3; ++i) item(commands[i], 96 + i * 28, i == selection);
        panel(190, 45);
        label("Welcome bot", 194, muted, 12, 111, compact);
        label("OFF", 215, amber, 12, 111, compact);
      }
      break;
    }
    case Screen::Settings: {
      panel(55, 123);
      const char* options[] = {"Brightness", "Battery", "About", "Diagnostics"};
      for (size_t i = 0; i < 4; ++i) item(options[i], 61 + i * 28, i == selection);
      panel(190, 45);
      std::snprintf(value, sizeof(value), "Light: %u%%", brightnessPercent[brightnessIndex]);
      label(value, 202, fg);
      break;
    }
    case Screen::Brightness:
      panel(55, 151);
      for (size_t i = 0; i < 5; ++i) {
        std::snprintf(value, sizeof(value), "%u%%%s", brightnessPercent[i], i == brightnessIndex ? " *" : "");
        item(value, 62 + i * 28, i == selection);
      }
      label("* Applied", 216, muted, 12, 111, compact);
      break;
    case Screen::Battery:
      panel(55, 180);
      if (voltage > 0) std::snprintf(value, sizeof(value), "%d mV", voltage);
      else std::snprintf(value, sizeof(value), "Unavailable");
      label(value, 66);
      if (batteryKnown) std::snprintf(value, sizeof(value), "~%d%%", battery);
      else std::snprintf(value, sizeof(value), "--%%");
      label(value, 102, green);
      label("Estimate only", 145, muted, 12, 111, compact);
      label("No runtime", 178, muted, 12, 111, compact);
      label("measurement", 199, muted, 12, 111, compact);
      break;
    case Screen::About:
      panel(55, 118, true);
      label("Wander Lima", 61, fg, 10, 117);
      label("Open-source", 87, fg, 12, 111, compact);
      label("Bitchat-inspired", 108, muted, 12, 111, compact);
      label("github.com/", 130, green, 12, 111, compact);
      label("wanderlima", 150, green, 12, 111, compact);
      d.drawRect(46, 184, 44, 44, fg);
      centered("QR", 189, fg, compact);
      centered("SOON", 208, muted, compact);
      break;
    case Screen::Diagnostics:
      panel(55, 180);
      std::snprintf(value, sizeof(value), "BLE %lu", (unsigned long)scannerReports()); label(value, 66, fg, 12, 111, compact);
      std::snprintf(value, sizeof(value), "Drop %lu", (unsigned long)scannerDrops()); label(value, 98, fg, 12, 111, compact);
      std::snprintf(value, sizeof(value), "RAM %u", unsigned(ESP.getFreeHeap())); label(value, 130, fg, 12, 111, compact);
      std::snprintf(value, sizeof(value), "Up %lus", (unsigned long)(now / 1000)); label(value, 162, fg, 12, 111, compact);
      label("v0.1.0 Scanner", 205, green, 12, 111, compact);
      break;
    default: break;
  }
  d.endWrite();
}
void move(int direction, const PeerTable& peers) {
  if (screen == Screen::Peers) {
    if (peers.count()) peerIndex = (peerIndex + peers.count() + direction) % peers.count();
    detail = false;
    return;
  }
  const size_t count = screen == Screen::Dashboard || screen == Screen::Settings ? 4 :
                       screen == Screen::Bot ? 3 : screen == Screen::Brightness ? 5 : 0;
  if (count) selection = (selection + count + direction) % count;
  detail = false;
}
} // namespace
void beginUi() {
  M5.Display.setRotation(0);
  M5.Display.setBrightness(brightness);
  M5.Display.setFont(body);
  M5.Display.setTextSize(1);
  M5.Display.setTextWrap(false);
  lastInput = millis();
  draw(PeerTable{}, lastInput);
}
void updateUi(const PeerTable& peers, uint32_t now) {
  const bool aDown = M5.BtnA.isPressed();
  const bool bDown = M5.BtnB.isPressed();
  const bool cDown = M5.BtnPWR.isPressed();
  const auto action = input.update(aDown, bDown, cDown, now, sleeping);
  if (aDown || bDown || cDown || action != NavigationAction::None) lastInput = now;
  if (action == NavigationAction::Wake) {
    sleeping = false;
    M5.Display.setBrightness(brightness);
    lastDraw = 0;
  } else if (action == NavigationAction::Up) move(-1, peers);
  else if (action == NavigationAction::Down) move(1, peers);
  else if (action == NavigationAction::Back) {
    if (detail && (screen == Screen::Peers || screen == Screen::Bot)) detail = false;
    else if (screen == Screen::Brightness || screen == Screen::Battery || screen == Screen::About || screen == Screen::Diagnostics) {
      selection = screen == Screen::Brightness ? 0 : screen == Screen::Battery ? 1 : screen == Screen::About ? 2 : 3;
      screen = Screen::Settings;
    } else if (screen != Screen::Splash) {
      screen = screen == Screen::Dashboard ? Screen::Settings :
               screen == Screen::Messages ? Screen::Channels : Screen::Dashboard;
      selection = 0;
    }
  } else if (action == NavigationAction::Select) {
    if (screen == Screen::Splash) screen = Screen::Dashboard;
    else if (screen == Screen::Dashboard) {
      const Screen destinations[] = {Screen::Peers, Screen::Channels, Screen::Bot, Screen::Settings};
      screen = destinations[selection]; selection = 0;
    } else if (screen == Screen::Channels) screen = Screen::Messages;
    else if (screen == Screen::Peers || screen == Screen::Bot) detail = !detail;
    else if (screen == Screen::Settings) {
      const Screen destinations[] = {Screen::Brightness, Screen::Battery, Screen::About, Screen::Diagnostics};
      screen = destinations[selection];
      selection = screen == Screen::Brightness ? brightnessIndex : 0;
    } else if (screen == Screen::Brightness) {
      brightnessIndex = selection;
      brightness = brightnessLevels[brightnessIndex];
      M5.Display.setBrightness(brightness);
      screen = Screen::Settings;
      selection = 0;
    }
  }
  if (!sleeping && uint32_t(now - lastInput) >= screenTimeout) {
    sleeping = true;
    M5.Display.setBrightness(0);
  }
  if (lastBattery == 0 || uint32_t(now - lastBattery) >= 5000) {
    voltage = M5.Power.getBatteryVoltage();
    battery = voltage > 0 ? M5.Power.getBatteryLevel() : -1;
    lastBattery = now;
  }
  if (!sleeping && (action != NavigationAction::None || lastDraw == 0 || uint32_t(now - lastDraw) >= 1000)) {
    draw(peers, now);
    lastDraw = now;
  }
}
} // namespace stick
