#include "Ui.h"
#include "Scanner.h"
#include "Mesh.h"
#include "Transport.h"
#include "Clock.h"
#include "Settings.h"
#include "PeerSelection.h"
#include "NavigationInput.h"
#include <M5Unified.h>
#include "assets/UiBitmaps.h"
#include "assets/UiFonts.h"
#include <cstdio>
#include <cstring>

namespace stick {
namespace {
// #EBDCBA warm cream, quantized to RGB565 for the display.
constexpr uint16_t bg = TFT_BLACK, fg = 0xEEF7, muted = 0xDEFB;
constexpr uint16_t green = 0x87F0, amber = TFT_YELLOW;
constexpr uint16_t headerBorder = 0x41C6;
constexpr uint32_t screenTimeout = 30000;
enum class Screen { Splash, Dashboard, Peers, Channels, Messages, Bot, Settings, Brightness, Battery, About, Diagnostics };
Screen screen = Screen::Splash;
size_t selection = 0;
PeerSelection peerSelection;
constexpr uint8_t brightnessLevels[] = {13, 64, 128, 191, 255};
constexpr unsigned brightnessPercent[] = {5, 25, 50, 75, 100};
size_t brightnessIndex = 2;
uint8_t brightness = brightnessLevels[brightnessIndex];
bool sleeping = false, detail = false, settingsSaved = true;
size_t messageOffset = 0, textLine = 0;
uint32_t lastInput = 0, lastDraw = 0, lastBattery = 0;
int battery = -1, voltage = 0;
NavigationInput input;
M5Canvas canvas(&M5.Display);
bool buffered = false;
lgfx::LGFXBase& surface() {
  return buffered ? static_cast<lgfx::LGFXBase&>(canvas) : static_cast<lgfx::LGFXBase&>(M5.Display);
}
const lgfx::IFont* body = &assets::FreeMonoBody16;
const lgfx::IFont* compact = &assets::FreeMonoCompact14;

void label(const char* value, int y, uint16_t color = fg, int x = 12,
           int width = 111, const lgfx::IFont* font = nullptr) {
  auto& d = surface();
  d.setFont(font ? font : body);
  d.setTextSize(1);
  char clipped[48]{};
  std::snprintf(clipped, sizeof(clipped), "%s", value);
  // All labels/radio names here are ASCII. Clip to the measured font width.
  const int available = width < 131 - x ? width : 131 - x;
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
void divider(int y, int x = 13, int width = 109) {
  surface().drawFastHLine(x, y, width, fg);
}
void bitmap(const uint8_t* bits, int x, int y, int width, int height, uint16_t color) {
  const int stride = (width + 7) / 8;
  for (int row = 0; row < height; ++row) {
    int start = -1;
    for (int col = 0; col <= width; ++col) {
      const bool on = col < width && (bits[row * stride + col / 8] & (0x80 >> (col % 8)));
      if (on && start < 0) start = col;
      if (!on && start >= 0) {
        surface().fillRect(x + start, y + row, col - start, 1, color);
        start = -1;
      }
    }
  }
}
void item(const char* text, int top, bool selected, int height = 24, bool radio = false, bool checked = false) {
  if (selected) surface().fillRect(5, top, 125, height, fg);
  else if (radio) surface().fillRect(5, top, 125, height, bg);
  if (radio) bitmap(checked ? assets::radioChecked : assets::radioUnchecked,
                    13, top + (height - 10) / 2, 10, 10, selected ? bg : fg);
  label(text, top + (height - 16) / 2, selected ? bg : fg, radio ? 29 : 13, radio ? 93 : 111);
}
void panel(int top, int height, bool peer = false) {
  surface().fillRect(4, top, 127, height, peer ? 0x0964 : 0x10A3);
  surface().drawRect(4, top, 127, height, peer ? green : fg);
  if (peer) surface().fillRect(4, top, 3, height, green);
}
void menu(const char* const* labels, size_t count, int top, int rowHeight) {
  panel(top, int(count) * rowHeight + int(count) - 1 + 10);
  for (size_t i = 0; i < count; ++i) {
    const int y = top + 5 + int(i) * (rowHeight + 1);
    item(labels[i], y, i == selection, rowHeight);
    if (i + 1 < count) divider(y + rowHeight, 5, 125);
  }
}
void drawSplash() {
  label("BITCHAT", 36, green, 10, 121, &fonts::FreeMonoBold12pt7b);
  label("STICK", 63, fg, 10, 121, &fonts::FreeMonoBold12pt7b);
  label("A: confirm", 115, fg, 10, 121, compact);
  label("B: up", 135, fg, 10, 121, compact);
  label("C: down", 155, fg, 10, 121, compact);
  label("Hold B: back", 175, muted, 10, 121, compact);
  item("A: START", 210, true);
}
void draw(const PeerTable& peers, uint32_t now) {
  auto& d = surface();
  d.startWrite();
  d.fillScreen(bg);
  if (screen == Screen::Splash) { drawSplash(); d.endWrite(); if(buffered) canvas.pushSprite(0,0); return; }
  const uint16_t radioColor = transportReady() ? green : scannerFault() ? TFT_RED : amber;
  d.fillRect(6, 7, 5, 5, radioColor);
  label(transportReady() ? "BLE" : scannerFault() ? "ERR" : "IDLE", 2,
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
  d.drawRect(0, 0, 135, 20, headerBorder);
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
  label(title, 26, fg, 4, 127, &fonts::FreeMonoBold9pt7b);
  switch (screen) {
    case Screen::Dashboard: {
      panel(54, 69);
      std::snprintf(value, sizeof(value), "Nearby: %u", unsigned(peers.candidates())); label(value, 57);
      std::snprintf(value, sizeof(value), "P:%u Msg:%u", unsigned(mesh().peerCount()), unsigned(mesh().messageCount())); label(value, 79, green);
      label(transportReady() && mesh().ready() ? "Relay: ON" : "Relay: OFF", 101, transportReady() && mesh().ready() ? green : amber);
      const char* options[] = {"Nearby", "Channels", "InfoBot", "Settings"};
      menu(options, 4, 129, 24);
      break;
    }
    case Screen::Peers: {
      if (mesh().peerCount()) {
        selection %= mesh().peerCount();
        const auto* peer = mesh().peer(selection);
        std::snprintf(value, sizeof(value), "Peer %u/%u", unsigned(selection+1), unsigned(mesh().peerCount()));
        label(value, 56, fg, 4, 127);
        panel(80, 147, true);
        label(peer->name[0] ? peer->name : "Unnamed", 89, fg, 15, 111);
        if (detail) {
          std::snprintf(value, sizeof(value), "%02x%02x%02x%02x",peer->id[0],peer->id[1],peer->id[2],peer->id[3]); label(value, 115, fg, 15, 111);
          std::snprintf(value, sizeof(value), "%02x%02x%02x%02x",peer->id[4],peer->id[5],peer->id[6],peer->id[7]); label(value, 141, fg, 15, 111);
        } else {
          label("Signed peer", 115, fg, 15, 111, compact);
          std::snprintf(value, sizeof(value), "Seen %lus", (unsigned long)((now-peer->seen)/1000));label(value, 141, muted, 15, 111, compact);
        }
        divider(162,15,111);
        label(peer->authenticated ? "Noise auth" : "Signed only", 173, peer->authenticated ? green : amber,15,111,compact);
        label("BLE local",199,muted,15,111,compact);
        break;
      }
      if (peerSelection.sync(peers)) detail = false;
      const auto peerIndex = peerSelection.index();
      const auto n = peers.candidates();
      if (!n) {
        panel(54, 110);
        label("No Bitchat", 63);
        divider(92);
        label(scannerFault() ? "Scan error" : "Waiting for", 103, muted, 13, 109, compact);
        label(scannerFault() ? "Check BLE" : "BLE adverts", 125, muted, 13, 109, compact);
      } else {
        const auto& obs = peers.candidateAt(peerIndex)->observation;
        std::snprintf(value, sizeof(value), "Device %u/%u", unsigned(peerIndex + 1), unsigned(n));
        label(value, 56, fg, 4, 127);
        panel(80, 147, true);
        if (detail) {
          char half[9]{};
          std::memcpy(half, obs.address, 8); label(half, 89);
          label(obs.address + 9, 111);
          std::snprintf(value, sizeof(value), "Type %u", unsigned(obs.addressType)); label(value, 137, muted, 13, 109, compact);
        } else {
          label(obs.name[0] ? obs.name : "Unnamed", 89, fg, 15, 111);
          std::snprintf(value, sizeof(value), "%d dBm", obs.rssi); label(value, 115, fg, 15, 111);
          std::snprintf(value, sizeof(value), "Seen %lus ago", (unsigned long)((now - obs.seenAt) / 1000));
          label(value, 141, muted, 15, 111, compact);
        }
        divider(162, 15, 111);
        label(obs.network == Network::Other ? "Other BLE" : "Bitchat?", 173,
              obs.network == Network::Other ? fg : amber, 15, 111);
        label("Not verified", 199, amber, 15, 111, compact);
      }
      break;
    }
    case Screen::Channels:
      panel(54, 93);
      item("#bluetooth", 60, true, 28);
      divider(94);
      label("Public BLE", 105, fg, 13, 109, compact);
      label(transportStatus(), 126, transportReady() ? green : amber, 13, 109, compact);
      panel(158, 78);
      label("Inbox + DMs", 167);
      label("No private", 194, muted, 13,109,compact);
      label("groups yet", 214, muted, 13,109,compact);
      break;
    case Screen::Messages: {
      const auto count = mesh().messageCount();
      if (!count) {
        panel(54, 117); label("No messages", 64); divider(92);
        label("#bluetooth",103,green,13,109,compact);
        label(transportStatus(),127,muted,13,109,compact);
        label("Waiting...",149,muted,13,109,compact);
      } else {
        if(messageOffset>=count) messageOffset=count-1;
        const auto* message=mesh().message(messageOffset);
        std::snprintf(value,sizeof(value),"%u/%u %s",unsigned(messageOffset+1),unsigned(count),message->outgoing?"You":message->privateMessage?"DM":"BLE");label(value,55,muted,4,127,compact);
        panel(77, 159);label(message->name,84,green,13,109,compact);divider(106);
        const size_t length=std::strlen(message->text), lines=(length+11)/12;
        if(textLine>=lines) textLine=lines?lines-1:0;
        for(size_t line=0;line<5;++line){const size_t start=(textLine+line)*12;if(start>=length)break;char part[13]{};std::memcpy(part,message->text+start,length-start>12?12:length-start);label(part,115+int(line)*20,fg,13,109,compact);}
        if(detail)label("Scroll text",218,green,13,109,compact);
        else if(message->truncated)label("Truncated",218,amber,13,109,compact);
      }
      break;
    }
    case Screen::Bot: {
      panel(54, 31);
      label(!mesh().botEnabled() ? "Bot OFF" : mesh().authenticatedCount() ? "Bot ready" : "Needs auth", 61, mesh().botEnabled() && mesh().authenticatedCount() ? green : amber);
      const char* commands[] = {"!info", "!rules", "!help"};
      if (detail) {
        panel(93, 139);
        label(commands[selection], 102, green);
        divider(129);
        label("Public reply", 140, green, 13, 109, compact);
        const char* lines[][3] = {{"Stick info", "BLE mesh", "Open-source"},
                                 {"Respect all", "No spam", "No welcomes"},
                                 {"!info !rules", "!help", "30s cooldown"}};
        for (int i = 0; i < 3; ++i) label(lines[selection][i], 166 + i * 20, fg, 13, 109, compact);
      } else {
        panel(93, 114);
        for (size_t i = 0; i < 3; ++i) item(commands[i], 98 + i * 26, i == selection, 22);
        item(mesh().botEnabled() ? "Bot: ON" : "Bot: OFF",176,selection==3,22);
        label("Welcome: OFF", 218, amber, 7, 123, compact);
      }
      break;
    }
    case Screen::Settings: {
      const char* options[] = {"Brightness", "Battery", "About", "Diagnostics"};
      menu(options, 4, 54, 30);
      if (!settingsSaved) {
        panel(195, 40);
        label("Not saved", 200, amber, 13, 109, compact);
        label("RAM only", 218, muted, 13, 109, compact);
      }
      break;
    }
    case Screen::Brightness:
      panel(54, 164);
      for (size_t i = 0; i < 5; ++i) {
        std::snprintf(value, sizeof(value), "%u%%", brightnessPercent[i]);
        const int top = 59 + int(i) * 31;
        item(value, top, i == selection, 30, true, i == brightnessIndex);
        if (i < 4) divider(top + 30, 5, 125);
      }
      break;
    case Screen::Battery:
      panel(54, 144);
      if (voltage > 0) std::snprintf(value, sizeof(value), "%d mV", voltage);
      else std::snprintf(value, sizeof(value), "Unknown");
      label(value, 65);
      if (batteryKnown) std::snprintf(value, sizeof(value), "~%d%%", battery);
      else std::snprintf(value, sizeof(value), "--%%");
      label(value, 95, green, 13, 109, &fonts::FreeMonoBold12pt7b);
      divider(126);
      label("Estimate", 137, muted, 13, 109, compact);
      label("No runtime", 163, muted, 13, 109, compact);
      label("test yet", 181, muted, 13, 109, compact);
      break;
    case Screen::About:
      surface().fillRect(4, 54, 127, 74, 0x0964);
      surface().fillRect(4, 54, 3, 74, green);
      label("Wander Lima", 60, fg, 13, 112);
      label("Open-source", 86, muted, 13, 112, compact);
      label("Bitchat app", 108, muted, 13, 112, compact);
      d.fillRect(20, 136, 95, 95, fg);
      bitmap(assets::repositoryQr, 20, 136, 95, 95, bg);
      break;
    case Screen::Diagnostics:
      panel(54, 158);
      std::snprintf(value, sizeof(value), "BLE %lu", (unsigned long)scannerReports()); label(value, 65, fg, 13, 109, compact);
      std::snprintf(value, sizeof(value), "Drop %lu", (unsigned long)scannerDrops()); label(value, 91, fg, 13, 109, compact);
      divider(115);
      std::snprintf(value, sizeof(value), "RAM %u", unsigned(ESP.getFreeHeap())); label(value, 126, fg, 13, 109, compact);
      std::snprintf(value, sizeof(value), "Up %lus", (unsigned long)(now / 1000)); label(value, 152, fg, 13, 109, compact);
      divider(176);
      std::snprintf(value, sizeof(value), "OK %lu Err%lu", (unsigned long)mesh().stats().accepted, (unsigned long)mesh().stats().rejected); label(value, 187, green, 13, 109, compact);
      label(clockRtc() ? "RTC / v0.2" : "Build clock", 216, clockRtc() ? muted : amber, 13,109,compact);
      break;
    default: break;
  }
  d.endWrite();
  if(buffered) canvas.pushSprite(0,0);
}
void move(int direction, const PeerTable& peers) {
  if (screen == Screen::Messages) {
    if(detail) { const auto* message=mesh().message(messageOffset);const size_t lines=message?(std::strlen(message->text)+11)/12:0;
      if(direction<0 && textLine) --textLine; else if(direction>0 && textLine+5<lines) ++textLine;
    } else if(mesh().messageCount()){messageOffset=(messageOffset+mesh().messageCount()+direction)%mesh().messageCount();textLine=0;}
    return;
  }
  if (screen == Screen::Peers) {
    if(mesh().peerCount()) selection=(selection+mesh().peerCount()+direction)%mesh().peerCount();
    else peerSelection.move(direction, peers);
    detail = false;
    return;
  }
  const size_t count = screen == Screen::Dashboard || screen == Screen::Settings ? 4 :
                       screen == Screen::Bot ? 4 : screen == Screen::Brightness ? 5 : 0;
  if (count) selection = (selection + count + direction) % count;
  detail = false;
}
} // namespace
void beginUi() {
  brightnessIndex = loadBrightnessIndex();
  brightness = brightnessLevels[brightnessIndex];
  M5.Display.setRotation(0);
  M5.Display.setBrightness(brightness);
  canvas.setColorDepth(16);
  buffered = canvas.createSprite(135,240) != nullptr;
  surface().setFont(body);
  surface().setTextSize(1);
  surface().setTextWrap(false);
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
    if (detail && (screen == Screen::Peers || screen == Screen::Bot || screen == Screen::Messages)) detail = false;
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
    } else if (screen == Screen::Channels) { screen = Screen::Messages; messageOffset=textLine=0; detail=false; }
    else if (screen == Screen::Bot && selection==3) mesh().setBotEnabled(!mesh().botEnabled());
    else if (screen == Screen::Peers || screen == Screen::Bot || screen == Screen::Messages) { detail = !detail; textLine=0; }
    else if (screen == Screen::Settings) {
      const Screen destinations[] = {Screen::Brightness, Screen::Battery, Screen::About, Screen::Diagnostics};
      screen = destinations[selection];
      selection = screen == Screen::Brightness ? brightnessIndex : 0;
    } else if (screen == Screen::Brightness) {
      settingsSaved = saveBrightnessIndex(uint8_t(selection));
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
