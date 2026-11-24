#include "Ui.h"
#include "Scanner.h"
#include "Mesh.h"
#include "Transport.h"
#include "Clock.h"
#include "Settings.h"
#include "PeerSelection.h"
#include "NavigationInput.h"
#include "JoycNavigation.h"
#include "Joyc.h"
#include "Feedback.h"
#include "MessageComposer.h"
#include "InfoBot.h"
#include "Version.h"
#include "Board.h"
#include "DrawSurface.h"
#include "DisplayProfile.h"
#include "assets/UiBitmaps.h"
#include <Arduino.h>
#include <cstdio>
#include <cstring>

static_assert(stick::kDisplayProfile.width == 240 && stick::kDisplayProfile.height == 135 && stick::kDisplayProfile.rotation == 1, "Plus2 layout is 240x135 landscape");
static_assert(stick::sameText(stick::kDisplayProfile.layoutId, "240x135-landscape"), "layout id");

namespace stick {
namespace {
constexpr uint16_t bg = 0, fg = 0xEEF7, muted = 0xDEFB;
constexpr uint16_t green = 0x87F0, amber = 0xFFE0, colorRed = 0xF800;
constexpr uint16_t panel = 0x10A3, line = 0x4A07, track = 0x73AE;
constexpr uint32_t screenTimeout = 30000;
constexpr int kRowY = 40, kRowH = 24, kRowStep = 25;
int rowTop(int index) { return kRowY + index * kRowStep; }
enum class Screen {
  Splash, Dashboard, Peers, Channels, Messages, Bot, Settings, Brightness, Battery, About,
  Diagnostics, Sound, Joystick, Reply, ShortMessages, Keyboard, Confirm, DateTime, SetDate, SetTime
};
constexpr Screen settingsPages[] = {
  Screen::Brightness, Screen::Battery, Screen::Diagnostics, Screen::Sound, Screen::Joystick, Screen::About, Screen::DateTime
};
constexpr const char* settingsLabels[] = {"Brightness", "Battery", "Diagnostics", "Sound", "JoyC HAT", "About", "Date & time"};
constexpr const uint8_t* settingsIcons[] = {
  assets::iconBright, assets::iconBattery, assets::iconDiag, assets::iconSound, assets::iconJoy, assets::iconRead, assets::iconClock
};
constexpr size_t settingsCount = sizeof(settingsPages) / sizeof(settingsPages[0]);
Screen screen = Screen::Splash;
uint8_t preferenceFlags = 0;
size_t selection = 0;
PeerSelection peerSelection;
bool detail = false;
MessageComposer composer;
Screen composeOrigin = Screen::Peers, editor = Screen::ShortMessages;
size_t originSelection = 0, editorSelection = 0;
bool originDetail = false, sendFailed = false;
const char* sendFailure = "Not sent";
bool privateDraft = true;
uint32_t queuedMessage = 0;
bool composing() {
  return screen == Screen::Reply || screen == Screen::ShortMessages || screen == Screen::Keyboard || screen == Screen::Confirm;
}
void openComposer(const uint8_t id[8], const char* name) {
  composeOrigin = screen;
  originSelection = selection;
  originDetail = detail;
  composer.open(id, name);
  privateDraft = true;
  queuedMessage = 0;
  sendFailed = false;
  screen = Screen::Reply;
  selection = 0;
}
void openPublicComposer() {
  const uint8_t id[8]{};
  openComposer(id, "#bluetooth");
  privateDraft = false;
}
void closeComposer() {
  composer.clear();
  screen = composeOrigin;
  selection = originSelection;
  detail = originDetail;
  queuedMessage = 0;
}
void confirmDraft() {
  editor = screen;
  editorSelection = selection;
  screen = Screen::Confirm;
  selection = 1;
  sendFailed = false;
}

constexpr uint8_t brightnessLevels[] = {13, 64, 128, 191, 255};
constexpr unsigned brightnessPercent[] = {5, 25, 50, 75, 100};
size_t brightnessIndex = 2;
uint8_t brightness = brightnessLevels[brightnessIndex];
bool sleeping = false, settingsSaved = true, clockSaved = true;
size_t messageOffset = 0, textLine = 0;
MessageScope messageScope = MessageScope::Public;
uint32_t activeMessage = 0;
uint32_t lastInput = 0, lastDraw = 0, lastBattery = 0;
int battery = -1, voltage = 0;
NavigationInput input;
JoycNavigation joycInput;
JoycClickInput joycClick;
JoycSample joycAxes;
bool buffered = false;
CivilTime edited{};
int field = 0;

void textAt(const char* value, int x, int y, uint16_t color, int width, UiFont font) {
  frameSetFont(font);
  char clipped[96]{};
  std::snprintf(clipped, sizeof(clipped), "%s", value ? value : "");
  if (frameTextWidth(clipped) > width) {
    size_t length = std::strlen(clipped);
    while (length && frameTextWidth(clipped) + frameTextWidth("...") > width) clipped[--length] = '\0';
    std::strcat(clipped, "...");
  }
  frameSetColor(color);
  frameDrawText(x, y, clipped);
}
void textRight(const char* value, int y, uint16_t color, int right, UiFont font) {
  frameSetFont(font);
  const int width = frameTextWidth(value);
  textAt(value, right - width, y, color, width, font);
}
void textCenter(const char* value, int y, uint16_t color, int center, UiFont font) {
  frameSetFont(font);
  const int width = frameTextWidth(value);
  textAt(value, center - width / 2, y, color, width, font);
}
int fontCap(UiFont font) {
  switch (font) {
    case UiFont::Vt16: return 10;
    case UiFont::Vt17: return 10;
    case UiFont::Vt18: return 12;
    case UiFont::Vt19: return 12;
    case UiFont::Vt20: return 12;
    case UiFont::Vt21: return 13;
    case UiFont::Vt22: return 13;
    case UiFont::Vt23: return 15;
    case UiFont::Vt31: return 19;
  }
  return 12;
}
int fontRise(UiFont font) { return font == UiFont::Vt31 ? 3 : 2; }
int textTop(int y, int height, UiFont font) {
  const int top = y + (height - fontCap(font)) / 2 - fontRise(font);
  return top < 0 ? 0 : top;
}
void textBox(const char* value, int x, int y, int width, int height, uint16_t color, UiFont font) {
  frameSetFont(font);
  const int textWidth = frameTextWidth(value ? value : "");
  int left = x + (width - textWidth) / 2;
  if (left < x) left = x;
  textAt(value, left, textTop(y, height, font), color, width, font);
}
void bitmap(const uint8_t* bits, int x, int y, int width, int height, uint16_t color) {
  const int stride = (width + 7) / 8;
  for (int row = 0; row < height; ++row) {
    int start = -1;
    for (int col = 0; col <= width; ++col) {
      const bool on = col < width && (bits[row * stride + col / 8] & (0x80 >> (col % 8)));
      if (on && start < 0) start = col;
      if (!on && start >= 0) {
        frameFillRect(x + start, y + row, col - start, 1, color);
        start = -1;
      }
    }
  }
}
void roundedFill(int x, int y, int w, int h, uint16_t color) {
  frameFillRect(x + 2, y, w - 4, h, color);
  frameFillRect(x, y + 2, w, h - 4, color);
  frameFillRect(x + 1, y + 1, 1, 1, color);
  frameFillRect(x + w - 2, y + 1, 1, 1, color);
  frameFillRect(x + 1, y + h - 2, 1, 1, color);
  frameFillRect(x + w - 2, y + h - 2, 1, 1, color);
}
int windowOf(int index, int count) {
  if (count <= 3) return 0;
  int window = index > 2 ? index - 2 : 0;
  if (window > count - 3) window = count - 3;
  return window < 0 ? 0 : window;
}
void scrollbar(int window, int count) {
  if (count <= 3) return;
  const int trackH = kRowStep * 2 + kRowH;
  const int thumb = trackH * 3 / count;
  const int travel = trackH - thumb;
  const int span = count - 3;
  frameFillRect(228, kRowY, 4, trackH, track);
  frameFillRect(228, kRowY + (span ? travel * window / span : 0), 4, thumb < 4 ? 4 : thumb, fg);
}
void row(int y, bool selected, const uint8_t* icon, const char* label, const char* value, int labelX = 0) {
  const uint16_t ink = selected ? bg : fg;
  if (selected) roundedFill(8, y, 216, kRowH, fg);
  const int x = labelX ? labelX : (icon ? 40 : 18);
  if (icon) bitmap(icon, 16, y + (kRowH - 16) / 2, 16, 16, ink);
  textAt(label, x, textTop(y, kRowH, UiFont::Vt20), ink, icon || labelX ? 112 : 160, UiFont::Vt20);
  if (value && value[0]) textRight(value, textTop(y, kRowH, UiFont::Vt20), ink, 214, UiFont::Vt20);
}
void titleBar(const char* title, const char* extra = nullptr, uint16_t extraColor = muted) {
  textAt(title, 8, 20, fg, extra ? 156 : 220, UiFont::Vt23);
  if (extra && extra[0]) textRight(extra, 22, extraColor, 232, UiFont::Vt18);
}
void footer(const char* move, const char* back, const char* action) {
  frameFillRect(8, 116, 224, 1, line);
  if (move && move[0]) textAt(move, 8, 118, muted, 80, UiFont::Vt16);
  if (back && back[0]) textCenter(back, 118, muted, 128, UiFont::Vt16);
  if (action && action[0]) textRight(action, 118, fg, 232, UiFont::Vt16);
}
void statusBar() {
  const bool ready = transportReady();
  const bool fault = scannerFault();
  const uint16_t radioColor = ready ? green : fault ? colorRed : amber;
  frameFillRect(8, 8, 4, 4, radioColor);
  textAt(ready ? "BLE scan" : fault ? "BLE err" : "BLE idle", 18, 2, radioColor, 78, UiFont::Vt17);
  const CivilTime now = clockCivil();
  char clock[8];
  std::snprintf(clock, sizeof(clock), "%02d:%02d", now.hour, now.minute);
  textCenter(clock, 1, muted, 120, UiFont::Vt19);
  if (mesh().unreadCount()) {
    static const uint8_t envelope[] = {0xff, 0xc0, 0xc0, 0xc0, 0xa1, 0x40, 0x92, 0x40, 0x8c, 0x40, 0x80, 0x40, 0xff, 0xc0};
    bitmap(envelope, 78, 6, 10, 7, green);
    frameFillRect(90, 5, 2, 2, green);
  }
  const bool known = battery >= 0 && battery <= 100;
  char percent[8];
  if (known) std::snprintf(percent, sizeof(percent), "%d%%", battery);
  else std::snprintf(percent, sizeof(percent), "--%%");
  const uint16_t batteryColor = known && battery < 20 ? amber : muted;
  textRight(percent, 2, batteryColor, 212, UiFont::Vt17);
  frameFillRect(215, 6, 14, 8, batteryColor);
  frameFillRect(216, 7, 12, 6, bg);
  frameFillRect(229, 8, 2, 4, batteryColor);
  if (known && battery > 0) frameFillRect(217, 8, (10 * battery + 99) / 100, 4, battery < 20 ? amber : green);
  frameFillRect(0, 19, 240, 1, line);
}
void checkbox(int y, bool selected, bool checked, const char* label) {
  row(y, selected, nullptr, label, nullptr, 40);
  const uint16_t ink = selected ? bg : fg;
  const int boxY = y + (kRowH - 16) / 2;
  frameDrawRect(16, boxY, 16, 16, ink);
  if (checked) {
    frameFillRect(19, boxY + 9, 3, 2, ink);
    frameFillRect(21, boxY + 7, 3, 3, ink);
    frameFillRect(24, boxY + 4, 3, 3, ink);
  }
}
void radio(int y, bool selected, bool checked, const char* label) {
  row(y, selected, nullptr, label, nullptr, 40);
  const uint16_t ink = selected ? bg : fg;
  const int boxY = y + (kRowH - 14) / 2;
  frameDrawRect(16, boxY, 14, 14, ink);
  if (checked) frameFillRect(19, boxY + 3, 8, 8, ink);
}
int settingsPageIndex(Screen page) {
  for (size_t i = 0; i < settingsCount; ++i) if (settingsPages[i] == page) return int(i);
  return -1;
}
int daysIn(int year, int month) {
  const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))) return 29;
  return days[month];
}
void adjustClock(int direction) {
  if (screen == Screen::SetDate) {
    if (field == 0) {
      const int limit = daysIn(edited.year, edited.month);
      edited.day += direction;
      if (edited.day < 1) edited.day = limit;
      if (edited.day > limit) edited.day = 1;
    } else if (field == 1) {
      edited.month += direction;
      if (edited.month < 1) edited.month = 12;
      if (edited.month > 12) edited.month = 1;
      const int limit = daysIn(edited.year, edited.month);
      if (edited.day > limit) edited.day = limit;
    } else {
      edited.year += direction;
      if (edited.year < 2000) edited.year = 2099;
      if (edited.year > 2099) edited.year = 2000;
      const int limit = daysIn(edited.year, edited.month);
      if (edited.day > limit) edited.day = limit;
    }
  } else if (field == 0) {
    edited.hour = (edited.hour + direction + 24) % 24;
  } else {
    edited.minute = (edited.minute + direction + 60) % 60;
  }
}
size_t fitLine(const char* text, size_t at, int width, UiFont font) {
  frameSetFont(font);
  const size_t length = std::strlen(text);
  size_t end = at;
  size_t lastBreak = at;
  while (end < length) {
    char probe[96]{};
    const size_t take = end + 1 - at;
    if (take >= sizeof(probe)) break;
    std::memcpy(probe, text + at, take);
    if (frameTextWidth(probe) > width) break;
    if (text[end] == ' ') lastBreak = end + 1;
    ++end;
  }
  if (end < length && lastBreak > at) return lastBreak - at;
  return end > at ? end - at : 1;
}
void wrapped(const char* text, int x, int y, int width, int lines, int step, UiFont font, uint16_t color, bool ellipsis) {
  size_t at = 0;
  const size_t length = std::strlen(text);
  for (int line = 0; line < lines && at < length; ++line) {
    size_t count = fitLine(text, at, width, font);
    char part[96]{};
    const bool more = at + count < length;
    if (ellipsis && line + 1 == lines && more) {
      std::memcpy(part, text + at, count < 90 ? count : 90);
      size_t n = std::strlen(part);
      while (n && frameTextWidth(part) + frameTextWidth("...") > width) part[--n] = '\0';
      std::strcat(part, "...");
    } else {
      std::memcpy(part, text + at, count < sizeof(part) ? count : sizeof(part) - 1);
    }
    textAt(part, x, y + line * step, color, width, font);
    at += count;
  }
}
size_t lineCount(const char* text, int width, UiFont font) {
  size_t at = 0, lines = 0;
  const size_t length = std::strlen(text);
  if (!length) return 0;
  while (at < length && lines < 32) {
    at += fitLine(text, at, width, font);
    ++lines;
  }
  return lines;
}
void syncMessageFocus() {
  if (screen != Screen::Messages) return;
  const size_t count = mesh().messageCount(messageScope);
  if (activeMessage) {
    for (size_t i = 0; i < count; ++i) if (mesh().message(i, messageScope)->sequence == activeMessage) {
      messageOffset = i;
      return;
    }
    activeMessage = 0;
    detail = false;
    textLine = 0;
  }
  if (count && messageOffset >= count) messageOffset = 0;
}
void drawSplash() {
  bitmap(assets::bolt, 18, 24, 25, 68, green);
  textAt("Bitchat Stick", 52, 38, fg, 186, UiFont::Vt31);
  textAt("Local mesh chat", 52, 66, muted, 186, UiFont::Vt21);
  footer("", "", "A Start");
}
void drawPeers(const PeerTable& peers, uint32_t now) {
  if (mesh().peerCount()) {
    selection %= mesh().peerCount();
    const auto* peer = mesh().peer(selection);
    char counter[16];
    std::snprintf(counter, sizeof(counter), "%u / %u", unsigned(selection + 1), unsigned(mesh().peerCount()));
    titleBar("Nearby", counter);
    textAt(peer->name[0] ? peer->name : "Unnamed", 8, 48, fg, 224, UiFont::Vt31);
    if (detail) {
      char id[12];
      std::snprintf(id, sizeof(id), "%02x%02x%02x%02x", peer->id[0], peer->id[1], peer->id[2], peer->id[3]);
      textAt(id, 8, 82, fg, 220, UiFont::Vt18);
      std::snprintf(id, sizeof(id), "%02x%02x%02x%02x", peer->id[4], peer->id[5], peer->id[6], peer->id[7]);
      textAt(id, 8, 98, muted, 220, UiFont::Vt18);
    } else {
      const bool authed = peer->authenticated;
      if (authed) frameFillRect(8, 83, 3, 16, green);
      textAt(authed ? "Noise authenticated" : "Signed only", authed ? 16 : 8, 81, authed ? green : amber, 210, UiFont::Vt18);
      char seen[24];
      std::snprintf(seen, sizeof(seen), "Seen %lus ago", (unsigned long)((now - peer->seen) / 1000));
      textAt(seen, 16, 99, muted, 200, UiFont::Vt16);
    }
    footer("B/C Move", "Hold B Back", "A Reply");
    return;
  }
  if (peerSelection.sync(peers)) detail = false;
  const auto n = peers.candidates();
  if (!n) {
    titleBar("Nearby");
    textAt("No Bitchat peers", 8, 53, fg, 224, UiFont::Vt23);
    textAt(scannerFault() ? "Scan error" : "Waiting for BLE devices", 8, 82, scannerFault() ? amber : muted, 224, UiFont::Vt19);
    if (scannerFault()) textAt("Check BLE", 8, 100, muted, 224, UiFont::Vt16);
    footer("B/C Move", "Hold B Back", "");
    return;
  }
  char counter[24];
  std::snprintf(counter, sizeof(counter), "Device %u / %u", unsigned(peerSelection.index() + 1), unsigned(n));
  titleBar("Nearby", counter);
  const auto& obs = peers.candidateAt(peerSelection.index())->observation;
  if (detail) {
    textAt(obs.name[0] ? obs.name : "Unnamed", 8, 46, fg, 224, UiFont::Vt20);
    char half[9]{};
    std::memcpy(half, obs.address, 8);
    textAt(half, 8, 68, fg, 220, UiFont::Vt18);
    textAt(obs.address + 9, 8, 84, fg, 220, UiFont::Vt18);
    char meta[24];
    std::snprintf(meta, sizeof(meta), "%d dBm", obs.rssi);
    textAt(meta, 8, 100, muted, 100, UiFont::Vt16);
  } else {
    textAt("Bitchat?", 8, 48, fg, 224, UiFont::Vt31);
    textAt("Not verified", 8, 81, amber, 220, UiFont::Vt18);
    textAt("BLE advertisement", 8, 99, muted, 220, UiFont::Vt16);
  }
  footer("B/C Move", "Hold B Back", "A Details");
}
void drawMessages(uint32_t) {
  const auto count = mesh().messageCount(messageScope);
  const char* heading = messageScope == MessageScope::Direct ? "Direct DMs" : "#bluetooth";
  if (!count) {
    titleBar(heading);
    textAt("No messages", 8, 53, fg, 224, UiFont::Vt23);
    textAt(messageScope == MessageScope::Direct ? "Direct DMs" : "Public chat", 8, 82, muted, 224, UiFont::Vt19);
    footer("B/C Move", "Hold B Back", "");
    return;
  }
  if (messageOffset >= count) messageOffset = count - 1;
  const auto* message = mesh().message(messageOffset, messageScope);
  char counter[16];
  std::snprintf(counter, sizeof(counter), "%u / %u", unsigned(messageOffset + 1), unsigned(count));
  titleBar(heading, counter);
  textAt(message->name, 8, 44, green, 180, UiFont::Vt18);
  const size_t lines = lineCount(message->text, 210, UiFont::Vt19);
  if (textLine >= lines) textLine = lines ? lines - 1 : 0;
  size_t at = 0;
  for (size_t line = 0; line < textLine; ++line) at += fitLine(message->text, at, 210, UiFont::Vt19);
  char rest[193];
  std::snprintf(rest, sizeof(rest), "%s", message->text + at);
  wrapped(rest, 8, 61, 210, 3, 17, UiFont::Vt19, fg, false);
  if (lines > 3) scrollbar(int(textLine), int(lines));
  if (message->truncated) textRight("Truncated", 98, amber, 220, UiFont::Vt16);
  footer(detail ? "B/C Scroll" : "B/C Move", "Hold B Back", detail ? "A Reply" : "A Read");
}
void drawKeyboard() {
  // Keys and ABC/SP/DEL/OK sit together above the footer. The draft takes the room above them.
  constexpr int textY = 3;
  constexpr int textStep = 14;
  constexpr int textLines = 3;
  constexpr int keyY = 52;
  constexpr int keyStep = 15;
  constexpr int actionY = 98;
  frameDrawRect(8, 1, 224, keyY - 3, line);
  const size_t length = composer.length();
  char counter[12];
  std::snprintf(counter, sizeof(counter), "%u/192", unsigned(length));
  textRight(counter, 3, length == 192 ? amber : muted, 220, UiFont::Vt16);
  const int width = 160;
  const size_t lines = lineCount(composer.draft, width, UiFont::Vt19);
  size_t at = 0;
  size_t skip = lines > textLines ? lines - textLines : 0;
  for (size_t line = 0; line < skip; ++line) at += fitLine(composer.draft, at, width, UiFont::Vt19);
  char rest[193];
  std::snprintf(rest, sizeof(rest), "%s", composer.draft + at);
  wrapped(rest, 12, textY, width, textLines, textStep, UiFont::Vt19, fg, false);
  frameSetFont(UiFont::Vt19);
  int caret = 12;
  if (rest[0]) {
    const size_t shown = lineCount(rest, width, UiFont::Vt19);
    const int row = shown ? int(shown) - 1 : 0;
    size_t rowAt = 0;
    for (int line = 0; line < row; ++line) rowAt += fitLine(rest, rowAt, width, UiFont::Vt19);
    char last[96]{};
    std::snprintf(last, sizeof(last), "%s", rest + rowAt);
    caret = 12 + frameTextWidth(last);
    if (caret > 200) caret = 200;
    frameFillRect(caret, textY + row * textStep, 1, 12, fg);
  } else frameFillRect(12, textY, 1, 12, fg);
  for (size_t key = 0; key < 30; ++key) {
    const int x = 8 + int((key % 10) * 204) / 9;
    const int y = keyY + int(key / 10) * keyStep;
    const bool on = selection == key;
    if (on) frameFillRect(x, y, 20, 14, fg);
    else frameFillRect(x, y, 20, 14, panel);
    char character[] = {composer.key(key), 0};
    textBox(character, x, y, 20, 14, on ? bg : fg, UiFont::Vt18);
  }
  const char* controls[] = {composer.mode == 0 ? "ABC" : composer.mode == 1 ? "123" : "abc", "SP", "DEL", "OK"};
  const int widths[] = {54, 55, 54, 55};
  int x = 8;
  for (size_t key = 0; key < 4; ++key) {
    const bool on = selection == 30 + key;
    const bool enabled = key != 3 || composer.ready();
    if (on && enabled) frameFillRect(x, actionY, widths[key], 16, fg);
    else frameFillRect(x, actionY, widths[key], 16, panel);
    textBox(controls[key], x, actionY, widths[key], 16, on && enabled ? bg : enabled ? fg : muted, UiFont::Vt18);
    x += widths[key] + 2;
  }
  footer("B/C Move", "Hold B Back", "A Select");
}
void drawConfirm() {
  const ChatMessage* queued = nullptr;
  if (queuedMessage) {
    for (size_t i = 0; i < mesh().messageCount(); ++i) if (mesh().message(i)->sequence == queuedMessage) queued = mesh().message(i);
  }
  const char* state = "Queued";
  if (queued && queued->delivery == DeliveryState::Read) state = "Read";
  else if (queued && queued->delivery == DeliveryState::Delivered) state = "Delivered";
  const char* heading = queuedMessage ? "Queued" : sendFailed ? "Not sent" : "Confirm";
  const char* extra = sendFailed ? sendFailure : queuedMessage ? (privateDraft ? state : "Public chat") : privateDraft ? "Private DM" : "Public chat";
  if (queuedMessage && privateDraft && state && std::strcmp(state, "Queued") != 0) extra = state;
  titleBar(heading, extra, sendFailed ? amber : queuedMessage ? green : muted);
  char who[40];
  std::snprintf(who, sizeof(who), privateDraft ? "To %s / DM" : "#bluetooth", composer.recipientName);
  textAt(who, 8, 44, muted, 224, UiFont::Vt18);
  wrapped(composer.draft, 8, 60, 224, 2, 15, UiFont::Vt17, fg, true);
  if (queuedMessage) {
    roundedFill(8, 88, 224, 26, fg);
    textBox("Done", 8, 88, 224, 26, bg, UiFont::Vt20);
    footer("", "Hold B Back", "A Done");
  } else {
    const bool cancel = selection == 1;
    if (cancel) roundedFill(8, 88, 108, 26, fg);
    else frameDrawRect(8, 88, 108, 26, line);
    textBox("Cancel", 8, 88, 108, 26, cancel ? bg : fg, UiFont::Vt20);
    if (!cancel) roundedFill(124, 88, 108, 26, fg);
    else frameDrawRect(124, 88, 108, 26, line);
    textBox("Send", 124, 88, 108, 26, cancel ? fg : bg, UiFont::Vt20);
    footer("B/C Move", "Hold B Back", cancel ? "A Cancel" : "A Send");
  }
}
void draw(const PeerTable& peers, uint32_t now) {
  syncMessageFocus();
  if (screen == Screen::Messages) {
    const auto* message = mesh().message(messageOffset, messageScope);
    if (message) {
      activeMessage = message->sequence;
      mesh().markRead(activeMessage);
    }
  }
  frameStart();
  frameFill(bg);
  if (screen == Screen::Splash) {
    drawSplash();
    frameFinish();
    return;
  }
  if (screen != Screen::Keyboard) statusBar();
  char value[48];
  switch (screen) {
    case Screen::Dashboard: {
      const char* labels[] = {"Nearby", "Channels", "InfoBot", "Settings"};
      const uint8_t* icons[] = {assets::iconPeer, assets::iconChannels, assets::iconBot, assets::iconSettings};
      char nearby[8], channel[16], bot[8];
      const unsigned near = mesh().peerCount() ? unsigned(mesh().peerCount()) : unsigned(peers.candidates());
      std::snprintf(nearby, sizeof(nearby), "%u", near);
      std::snprintf(channel, sizeof(channel), "%u new", unsigned(mesh().unreadCount()));
      std::snprintf(bot, sizeof(bot), "%s", mesh().botEnabled() ? "ON" : "OFF");
      const char* values[] = {nearby, channel, bot, ""};
      char counter[16];
      std::snprintf(counter, sizeof(counter), "%u / %u", unsigned(selection + 1), 4u);
      titleBar("Home", counter);
      const int window = windowOf(int(selection), 4);
      for (int i = 0; i < 3; ++i) row(rowTop(i), window + i == int(selection), icons[window + i], labels[window + i], values[window + i]);
      scrollbar(window, 4);
      footer("B/C Move", "Hold B Back", "A Open");
      break;
    }
    case Screen::Peers: drawPeers(peers, now); break;
    case Screen::Channels:
      if (detail) {
        titleBar("#bluetooth");
        const char* actions[] = {"Read chat", "Write msg"};
        const uint8_t* icons[] = {assets::iconRead, assets::iconWrite};
        for (int i = 0; i < 2; ++i) row(rowTop(i), int(selection) == i, icons[i], actions[i], nullptr);
        textAt("Public chat over BLE", 8, 96, muted, 220, UiFont::Vt18);
        footer("B/C Move", "Hold B Back", "A Open");
      } else {
        titleBar("Channels");
        char pub[16], dm[16];
        std::snprintf(pub, sizeof(pub), "%u new", unsigned(mesh().unreadCount(MessageScope::Public)));
        std::snprintf(dm, sizeof(dm), "%u new", unsigned(mesh().unreadCount(MessageScope::Direct)));
        row(rowTop(0), selection == 0, assets::iconHash, "#bluetooth", pub);
        row(rowTop(1), selection == 1, assets::iconDm, "Direct DMs", dm);
        textAt(selection == 0 ? "Public chat over BLE" : "Private text", 8, 96, muted, 220, UiFont::Vt18);
        footer("B/C Move", "Hold B Back", "A Open");
      }
      break;
    case Screen::Messages: drawMessages(now); break;
    case Screen::Reply: {
      titleBar(privateDraft ? "Reply" : "Write", privateDraft ? composer.recipientName : "#bluetooth");
      const char* options[] = {"Short msg", "Keyboard", "Welcome msg"};
      const uint8_t* icons[] = {assets::iconShort, assets::iconKeyboard, assets::iconWelcome};
      for (int i = 0; i < 3; ++i) row(rowTop(i), int(selection) == i, icons[i], options[i], nullptr);
      footer("B/C Move", "Hold B Back", "A Open");
      break;
    }
    case Screen::ShortMessages: {
      char counter[16];
      std::snprintf(counter, sizeof(counter), "%u / %u", unsigned(selection + 1), unsigned(MessageComposer::presetCount));
      titleBar("Short messages", counter);
      const int window = windowOf(int(selection), int(MessageComposer::presetCount));
      for (int i = 0; i < 3 && window + i < int(MessageComposer::presetCount); ++i)
        row(rowTop(i), window + i == int(selection), assets::iconShort, MessageComposer::presets[window + i], nullptr);
      scrollbar(window, int(MessageComposer::presetCount));
      footer("B/C Move", "Hold B Back", "A Select");
      break;
    }
    case Screen::Keyboard: drawKeyboard(); break;
    case Screen::Confirm: drawConfirm(); break;
    case Screen::Bot: {
      char counter[16];
      const size_t count = kBotCommandCount + 1;
      std::snprintf(counter, sizeof(counter), "%u / %u", unsigned(selection + 1), unsigned(count));
      titleBar("InfoBot", detail ? mesh().botStatus() : counter, mesh().botEnabled() ? green : amber);
      if (detail && selection < kBotCommandCount) {
        textAt(kBotCommands[selection], 8, 48, green, 220, UiFont::Vt20);
        textAt("Public + DM", 8, 70, fg, 220, UiFont::Vt18);
        const char* lines[][3] = {{"Stick info", "BLE relay", "Private text"},
                                   {"Respect all", "No spam", "Public chat"},
                                   {"Send command", "in chat / DM", "Wait 3s"},
                                   {"Open-source", "Wander Lima", "Project link"}};
        for (int i = 0; i < 3; ++i) textAt(lines[selection][i], 8, 80 + i * 11, muted, 220, UiFont::Vt16);
        footer("B/C Move", "Hold B Back", "");
      } else {
        const int window = windowOf(int(selection), int(count));
        const uint8_t* icons[] = {assets::iconRead, assets::iconRead, assets::iconRead, assets::iconRead, assets::iconBot};
        const char* toggle = mesh().botEnabled() ? "Bot: ON" : "Bot: OFF";
        for (int i = 0; i < 3 && window + i < int(count); ++i) {
          const bool last = window + i == int(kBotCommandCount);
          row(rowTop(i), window + i == int(selection), icons[window + i], last ? toggle : kBotCommands[window + i], nullptr);
        }
        scrollbar(window, int(count));
        footer("B/C Move", "Hold B Back", selection == kBotCommandCount ? "A Toggle" : "A Open");
      }
      break;
    }
    case Screen::Settings: {
      char counter[16];
      std::snprintf(counter, sizeof(counter), "%u / %u", unsigned(selection + 1), unsigned(settingsCount));
      titleBar("Settings", settingsSaved ? counter : "Not saved", settingsSaved ? muted : amber);
      const int window = windowOf(int(selection), int(settingsCount));
      for (int i = 0; i < 3 && window + i < int(settingsCount); ++i)
        row(rowTop(i), window + i == int(selection), settingsIcons[window + i], settingsLabels[window + i], nullptr);
      scrollbar(window, int(settingsCount));
      footer("B/C Move", "Hold B Back", "A Open");
      break;
    }
    case Screen::Sound: {
      titleBar("Sound", settingsSaved ? nullptr : "Not saved", amber);
      const char* options[] = {"Interact", "DM alert", "Channel"};
      const uint8_t flags[] = {SoundUi, SoundDm, SoundMesh};
      for (int i = 0; i < 3; ++i) checkbox(rowTop(i), int(selection) == i, preferenceFlags & flags[i], options[i]);
      footer("B/C Move", "Hold B Back", "A Toggle");
      break;
    }
    case Screen::Joystick:
      titleBar("JoyC HAT", settingsSaved ? nullptr : "Not saved", amber);
      checkbox(rowTop(0), selection == 0, preferenceFlags & JoycClick, "Click");
      checkbox(rowTop(1), selection == 1, preferenceFlags & JoycLed, "LED pulse");
      textAt(joycAxes.valid ? "Hat ready" : "No HAT", 8, 98, joycAxes.valid ? green : amber, 120, UiFont::Vt18);
      if (preferenceFlags & JoycClick)
        textRight(!joycAxes.buttonValid ? "Click error" : joycAxes.pressed ? "Pressed" : "Released", 98, muted, 232, UiFont::Vt18);
      else textRight("Click off", 98, muted, 232, UiFont::Vt18);
      footer("B/C Move", "Hold B Back", "A Toggle");
      break;
    case Screen::Brightness:
      titleBar("Brightness");
      {
        const int window = windowOf(int(selection), 5);
        for (int i = 0; i < 3; ++i) {
          std::snprintf(value, sizeof(value), "%u%%", brightnessPercent[window + i]);
          radio(rowTop(i), window + i == int(selection), window + i == int(brightnessIndex), value);
        }
        scrollbar(window, 5);
      }
      footer("B/C Move", "Hold B Back", "A Apply");
      break;
    case Screen::Battery: {
      titleBar("Battery");
      const bool known = battery >= 0 && battery <= 100;
      if (known) std::snprintf(value, sizeof(value), "%d%%", battery);
      else std::snprintf(value, sizeof(value), "--%%");
      textAt(value, 8, 46, fg, 120, UiFont::Vt31);
      if (voltage > 0) std::snprintf(value, sizeof(value), "%d mV", voltage);
      else std::snprintf(value, sizeof(value), "Unknown");
      textRight(value, 66, muted, 232, UiFont::Vt19);
      textAt(known ? "Estimated charge" : "Estimated charge", 8, 84, muted, 220, UiFont::Vt18);
      textAt(known ? "Approximate only" : "No reading available", 8, 100, muted, 220, UiFont::Vt16);
      footer("", "Hold B Back", "");
      break;
    }
    case Screen::About:
      titleBar("About");
      textAt("Bitchat Stick", 8, 46, fg, 130, UiFont::Vt22);
      textAt("Wander Lima", 8, 68, muted, 130, UiFont::Vt19);
      textAt("Project website", 8, 90, green, 130, UiFont::Vt18);
      frameFillRect(148, 26, 84, 84, 0xFFFF);
      bitmap(assets::repositoryQr, 148, 26, 84, 84, bg);
      footer("", "Hold B Back", "");
      break;
    case Screen::Diagnostics:
      titleBar("Diagnostics");
      std::snprintf(value, sizeof(value), "BLE %lu", (unsigned long)scannerReports());
      textAt(value, 8, 46, fg, 220, UiFont::Vt18);
      std::snprintf(value, sizeof(value), "Drop %lu", (unsigned long)scannerDrops());
      textAt(value, 8, 62, fg, 220, UiFont::Vt18);
      std::snprintf(value, sizeof(value), "RAM %u", unsigned(boardFreeHeap()));
      textAt(value, 8, 78, fg, 220, UiFont::Vt18);
      std::snprintf(value, sizeof(value), "Up %lus", (unsigned long)(now / 1000));
      textAt(value, 120, 78, muted, 110, UiFont::Vt18);
      std::snprintf(value, sizeof(value), "OK %lu Err%lu", (unsigned long)mesh().stats().accepted, (unsigned long)mesh().stats().rejected);
      textAt(value, 8, 96, green, 220, UiFont::Vt16);
      if (joycAxes.valid) {
        std::snprintf(value, sizeof(value), "J:%d,%d", int(joycAxes.x), int(joycAxes.y));
        textRight(value, 96, green, 232, UiFont::Vt16);
      } else textRight(clockRtc() ? "v" STICK_VERSION " RTC" : "Build clock", 96, clockRtc() ? muted : amber, 232, UiFont::Vt16);
      footer("", "Hold B Back", "");
      break;
    case Screen::DateTime: {
      titleBar("Date & time");
      const CivilTime current = clockCivil();
      char date[16], hm[8];
      std::snprintf(date, sizeof(date), "%02d/%02d/%04d", current.day, current.month, current.year);
      std::snprintf(hm, sizeof(hm), "%02d:%02d", current.hour, current.minute);
      row(rowTop(0), selection == 0, assets::iconClock, "Date", date);
      row(rowTop(1), selection == 1, assets::iconClock, "Time", hm);
      textAt(clockSaved ? "Local device clock / 24h" : "Not saved", 8, 98, clockSaved ? muted : amber, 220, UiFont::Vt16);
      footer("B/C Move", "Hold B Back", "A Edit");
      break;
    }
    case Screen::SetDate:
    case Screen::SetTime: {
      char counter[8];
      const int total = screen == Screen::SetDate ? 3 : 2;
      std::snprintf(counter, sizeof(counter), "%d / %d", field + 1, total);
      titleBar(screen == Screen::SetDate ? "Set date" : "Set time", counter);
      auto box = [&](int x, int w, bool on, const char* caption, const char* number) {
        textAt(caption, x, 45, muted, w, UiFont::Vt18);
        if (on) frameFillRect(x, 65, w, 33, fg);
        else frameFillRect(x, 65, w, 33, panel);
        textBox(number, x, 65, w, 33, on ? bg : fg, UiFont::Vt31);
      };
      char a[8], b[8], c[8];
      if (screen == Screen::SetDate) {
        std::snprintf(a, sizeof(a), "%02d", edited.day);
        std::snprintf(b, sizeof(b), "%02d", edited.month);
        std::snprintf(c, sizeof(c), "%04d", edited.year);
        box(8, 48, field == 0, "DD", a);
        box(64, 48, field == 1, "MM", b);
        box(120, 112, field == 2, "YYYY", c);
        textAt(clockSaved ? "Day / Month / Year" : "Not saved", 8, 101, clockSaved ? muted : amber, 220, UiFont::Vt16);
      } else {
        std::snprintf(a, sizeof(a), "%02d", edited.hour);
        std::snprintf(b, sizeof(b), "%02d", edited.minute);
        box(8, 100, field == 0, "HH", a);
        box(132, 100, field == 1, "MM", b);
        textBox(":", 112, 65, 16, 33, fg, UiFont::Vt31);
        textAt(clockSaved ? "24-hour format" : "Not saved", 8, 101, clockSaved ? muted : amber, 220, UiFont::Vt16);
      }
      const bool last = field + 1 == total;
      footer("B/C Change", "Hold B Back", last ? "A Save" : "A Next");
      break;
    }
    default: break;
  }
  frameFinish();
}
void moveKeyboard(int dx, int dy) {
  constexpr int columns = 10;
  constexpr int actionOfColumn[] = {30, 30, 30, 31, 31, 32, 32, 32, 33, 33};
  constexpr int columnOfAction[] = {1, 3, 6, 8};
  const int index = int(selection);
  if (dx) {
    if (index < 30) {
      const int column = index % columns + dx;
      if (column >= 0 && column < columns) selection = size_t(index - index % columns + column);
    } else {
      const int next = index + dx;
      if (next >= 30 && next < 34) selection = size_t(next);
    }
    return;
  }
  if (!dy || index < 0) return;
  if (index < 30) {
    const int row = index / columns + dy;
    const int column = index % columns;
    if (row >= 0 && row < 3) selection = size_t(row * columns + column);
    else if (row == 3) selection = size_t(actionOfColumn[column]);
  } else if (dy < 0 && index < 34) {
    selection = size_t(20 + columnOfAction[index - 30]);
  }
}
void move(int direction, const PeerTable& peers) {
  if (screen == Screen::SetDate || screen == Screen::SetTime) {
    adjustClock(direction);
    return;
  }
  if (screen == Screen::Channels && detail) {
    selection = (selection + 2 + direction) % 2;
    return;
  }
  if (composing()) {
    const size_t count = screen == Screen::Reply ? 3 : screen == Screen::ShortMessages ? MessageComposer::presetCount
                         : screen == Screen::Keyboard ? MessageComposer::keyCount : queuedMessage ? 1 : 2;
    selection = (selection + count + direction) % count;
    return;
  }
  if (screen == Screen::Messages) {
    if (detail) {
      const auto* message = mesh().message(messageOffset, messageScope);
      const size_t lines = message ? lineCount(message->text, 210, UiFont::Vt19) : 0;
      if (direction < 0 && textLine) --textLine;
      else if (direction > 0 && textLine + 3 < lines) ++textLine;
    } else if (mesh().messageCount(messageScope)) {
      messageOffset = (messageOffset + mesh().messageCount(messageScope) + direction) % mesh().messageCount(messageScope);
      textLine = 0;
      activeMessage = 0;
    }
    return;
  }
  if (screen == Screen::Peers) {
    if (mesh().peerCount()) selection = (selection + mesh().peerCount() + direction) % mesh().peerCount();
    else peerSelection.move(direction, peers);
    detail = false;
    return;
  }
  const size_t count = screen == Screen::Dashboard ? 4 : screen == Screen::Settings ? settingsCount
                       : screen == Screen::Sound ? 3 : screen == Screen::Joystick ? 2
                       : screen == Screen::Channels ? 2 : screen == Screen::Bot ? kBotCommandCount + 1
                       : screen == Screen::Brightness ? 5 : screen == Screen::DateTime ? 2 : 0;
  if (count) selection = (selection + count + direction) % count;
  detail = false;
}
} // namespace
void beginUi() {
  preferenceFlags = loadPreferenceFlags();
  brightnessIndex = loadBrightnessIndex();
  brightness = brightnessLevels[brightnessIndex];
  buffered = surfaceBegin();
  displaySetBrightness(brightness);
  frameSetFont(UiFont::Vt18);
  lastInput = millis();
  draw(PeerTable{}, lastInput);
}
void updateUi(const PeerTable& peers, uint32_t now) {
  syncMessageFocus();
  const BoardButtons keys = boardButtons();
  const bool aDown = keys.a;
  const bool bDown = keys.b;
  const bool cDown = keys.power;
  const auto buttons = input.update(aDown, bDown, cDown, now, sleeping);
  const bool buttonsBusy = aDown || bDown || cDown || buttons != NavigationAction::None;
  joycAxes = pollJoyc(now, preferenceFlags & JoycClick);
  const auto click = joycClick.update(joycAxes.pressed, joycAxes.valid && joycAxes.buttonValid,
                                     preferenceFlags & JoycClick, now, sleeping, buttonsBusy);
  const bool keyboard = screen == Screen::Keyboard;
  const auto joystick = joycInput.update(joycAxes.x, joycAxes.y, joycAxes.valid, now, sleeping,
                                        buttonsBusy || click != NavigationAction::None, keyboard);
  auto action = joystick;
  if (click != NavigationAction::None) action = click;
  if (buttons != NavigationAction::None) action = buttons;
  if (aDown || bDown || cDown || action != NavigationAction::None) lastInput = now;
  if (action == NavigationAction::Wake) {
    sleeping = false;
    displaySetBrightness(brightness);
    lastDraw = 0;
  } else if (keyboard && (action == NavigationAction::Left || action == NavigationAction::Right ||
                          action == NavigationAction::Up || action == NavigationAction::Down)) {
    const size_t before = selection;
    moveKeyboard(action == NavigationAction::Left ? -1 : action == NavigationAction::Right ? 1 : 0,
                 action == NavigationAction::Up ? -1 : action == NavigationAction::Down ? 1 : 0);
    if (selection == before) action = NavigationAction::None;
  } else if (action == NavigationAction::Up) move(-1, peers);
  else if (action == NavigationAction::Down) move(1, peers);
  else if (action == NavigationAction::Back) {
    if (composing()) {
      if (screen == Screen::Reply || (screen == Screen::Confirm && queuedMessage)) closeComposer();
      else if (screen == Screen::Confirm) {
        screen = editor;
        selection = editorSelection;
        sendFailed = false;
      } else {
        selection = screen == Screen::Keyboard ? 1 : 0;
        screen = Screen::Reply;
      }
    } else if (screen == Screen::SetDate || screen == Screen::SetTime) {
      selection = screen == Screen::SetDate ? 0 : 1;
      screen = Screen::DateTime;
      clockSaved = true;
    } else if (screen == Screen::Channels && detail) {
      detail = false;
      selection = 0;
    } else if (detail && (screen == Screen::Peers || screen == Screen::Bot || screen == Screen::Messages)) detail = false;
    else if (settingsPageIndex(screen) >= 0) {
      selection = size_t(settingsPageIndex(screen));
      screen = Screen::Settings;
    } else if (screen != Screen::Splash) {
      const bool directInbox = screen == Screen::Messages && messageScope == MessageScope::Direct;
      screen = screen == Screen::Dashboard ? Screen::Splash : screen == Screen::Messages ? Screen::Channels : Screen::Dashboard;
      selection = directInbox ? 1 : 0;
      if (screen == Screen::Channels) detail = !directInbox;
    }
  } else if (action == NavigationAction::Select) {
    if (screen == Screen::Reply) {
      if (selection == 2) {
        std::snprintf(composer.draft, sizeof(composer.draft), "%s", kBotIntroduction);
        confirmDraft();
      } else {
        screen = selection == 0 ? Screen::ShortMessages : Screen::Keyboard;
        selection = 0;
      }
    } else if (screen == Screen::ShortMessages) {
      composer.preset(selection);
      confirmDraft();
    } else if (screen == Screen::Keyboard) {
      if (composer.selectKey(selection)) confirmDraft();
    } else if (screen == Screen::Confirm) {
      if (queuedMessage || selection == 1) closeComposer();
      else if (privateDraft ? mesh().sendDirect(composer.recipient, composer.draft) : mesh().sendText(composer.draft)) {
        queuedMessage = mesh().message(0)->sequence;
        selection = 0;
        sendFailed = false;
      } else {
        sendFailed = true;
        sendFailure = mesh().sendStatus();
        if (!privateDraft && transportLinks() && std::strcmp(sendFailure, "No phone") == 0) sendFailure = "Busy";
      }
    } else if (screen == Screen::SetDate || screen == Screen::SetTime) {
      const int total = screen == Screen::SetDate ? 3 : 2;
      if (field + 1 < total) ++field;
      else if (clockSetCivil(edited)) {
        clockSaved = true;
        selection = screen == Screen::SetDate ? 0 : 1;
        screen = Screen::DateTime;
      } else clockSaved = false;
    } else if (screen == Screen::DateTime) {
      edited = clockCivil();
      field = 0;
      clockSaved = true;
      screen = selection == 0 ? Screen::SetDate : Screen::SetTime;
    } else if (screen == Screen::Peers && mesh().peerCount()) {
      const auto* peer = mesh().peer(selection % mesh().peerCount());
      openComposer(peer->id, peer->name);
    } else if (screen == Screen::Messages && detail) {
      const auto* message = mesh().message(messageOffset, messageScope);
      if (message) {
        bool known = false;
        for (auto id : message->senderId) known |= id != 0;
        if (known) openComposer(message->senderId, message->name);
      }
    } else if (screen == Screen::Splash) screen = Screen::Dashboard;
    else if (screen == Screen::Dashboard) {
      const Screen destinations[] = {Screen::Peers, Screen::Channels, Screen::Bot, Screen::Settings};
      screen = destinations[selection];
      selection = 0;
    } else if (screen == Screen::Channels) {
      if (!detail && selection == 0) {
        detail = true;
        selection = 0;
      } else if (detail && selection == 1) openPublicComposer();
      else {
        messageScope = detail ? MessageScope::Public : MessageScope::Direct;
        screen = Screen::Messages;
        messageOffset = textLine = 0;
        activeMessage = 0;
        detail = false;
      }
    } else if (screen == Screen::Bot && selection == kBotCommandCount) mesh().setBotEnabled(!mesh().botEnabled());
    else if (screen == Screen::Peers || screen == Screen::Bot || screen == Screen::Messages) {
      detail = !detail;
      textLine = 0;
    } else if (screen == Screen::Settings) {
      screen = settingsPages[selection];
      selection = screen == Screen::Brightness ? brightnessIndex : 0;
    } else if (screen == Screen::Sound || screen == Screen::Joystick) {
      const uint8_t flags[] = {SoundUi, SoundDm, SoundMesh, JoycClick, JoycLed};
      const size_t index = screen == Screen::Sound ? selection : selection + 3;
      preferenceFlags ^= flags[index];
      settingsSaved = savePreferenceFlags(preferenceFlags);
    } else if (screen == Screen::Brightness) {
      settingsSaved = saveBrightnessIndex(uint8_t(selection));
      brightnessIndex = selection;
      brightness = brightnessLevels[brightnessIndex];
      displaySetBrightness(brightness);
      screen = Screen::Settings;
      selection = 0;
    }
  }
  const auto& feedbackStats = mesh().stats();
  updateFeedback(now, preferenceFlags, action, feedbackStats.receivedDm, feedbackStats.receivedMesh);
  if (!sleeping && uint32_t(now - lastInput) >= screenTimeout) {
    sleeping = true;
    displaySetBrightness(0);
  }
  if (lastBattery == 0 || uint32_t(now - lastBattery) >= 5000) {
    boardBattery(voltage, battery);
    lastBattery = now;
  }
  if (!sleeping && (action != NavigationAction::None || lastDraw == 0 || uint32_t(now - lastDraw) >= 1000)) {
    draw(peers, now);
    lastDraw = now;
  }
}
} // namespace stick
