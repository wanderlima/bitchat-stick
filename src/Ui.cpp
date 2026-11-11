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
enum class Screen { Splash, Dashboard, Peers, Channels, Messages, Bot, Settings, Brightness, Battery, About, Diagnostics, Sound, Joystick, Reply, ShortMessages, Keyboard, Confirm };
constexpr Screen settingsPages[] = {Screen::Brightness, Screen::Battery, Screen::Diagnostics, Screen::Sound, Screen::Joystick, Screen::About};
constexpr const char* settingsLabels[] = {"Brightness", "Battery", "Diagnostics", "Sound", "JoyC HAT", "About"};
constexpr size_t settingsCount = sizeof(settingsPages) / sizeof(settingsPages[0]);
Screen screen = Screen::Splash;
uint8_t preferenceFlags = 0;
size_t selection = 0;
PeerSelection peerSelection;
bool detail=false;
MessageComposer composer;
Screen composeOrigin=Screen::Peers, editor=Screen::ShortMessages;
size_t originSelection=0, editorSelection=0, reviewPage=0;
bool originDetail=false, sendFailed=false;
const char* sendFailure="Not sent";
bool privateDraft=true;
uint32_t queuedMessage=0;
bool composing() {return screen==Screen::Reply||screen==Screen::ShortMessages||screen==Screen::Keyboard||screen==Screen::Confirm;}
size_t reviewPages() {return (composer.length()+47)/48;}
void openComposer(const uint8_t id[8],const char* name) {
  composeOrigin=screen;originSelection=selection;originDetail=detail;
  composer.open(id,name);privateDraft=true;queuedMessage=0;sendFailed=false;reviewPage=0;
  screen=Screen::Reply;selection=0;
}
void openPublicComposer(){const uint8_t id[8]{};openComposer(id,"#bluetooth");privateDraft=false;}
void closeComposer() {
  composer.clear();screen=composeOrigin;selection=originSelection;detail=originDetail;queuedMessage=0;
}
void confirmDraft() {
  editor=screen;editorSelection=selection;screen=Screen::Confirm;
  reviewPage=0;selection=reviewPages()+1;sendFailed=false; // Cancel focused initially.
}

constexpr uint8_t brightnessLevels[] = {13, 64, 128, 191, 255};
constexpr unsigned brightnessPercent[] = {5, 25, 50, 75, 100};
size_t brightnessIndex = 2;
uint8_t brightness = brightnessLevels[brightnessIndex];
bool sleeping = false, settingsSaved = true;
size_t messageOffset = 0, textLine = 0;
MessageScope messageScope = MessageScope::Public;
uint32_t activeMessage = 0;
uint32_t lastInput = 0, lastDraw = 0, lastBattery = 0;
int battery = -1, voltage = 0;
NavigationInput input;
JoycNavigation joycInput;
JoycClickInput joycClick;
JoycSample joycAxes;
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
void checkbox(const char* text, int top, bool selected, bool checked) {
  if (selected) surface().fillRect(5, top, 125, 30, fg);
  const uint16_t ink = selected ? bg : fg;
  surface().drawRect(13, top + 9, 12, 12, ink);
  if (checked) surface().fillRect(16, top + 12, 6, 6, ink);
  label(text, top + 7, ink, 30, 92);
}
int settingsPageIndex(Screen page) {
  for (size_t i = 0; i < settingsCount; ++i) if (settingsPages[i] == page) return int(i);
  return -1;
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
void syncMessageFocus() {
  if(screen!=Screen::Messages)return;
  const size_t count=mesh().messageCount(messageScope);
  if(activeMessage){for(size_t i=0;i<count;++i)if(mesh().message(i,messageScope)->sequence==activeMessage){messageOffset=i;return;}
    activeMessage=0;detail=false;textLine=0;
  }
  if(messageOffset>=count)messageOffset=0;
}
void draw(const PeerTable& peers, uint32_t now) {
  syncMessageFocus();
  if(screen==Screen::Messages){const auto* message=mesh().message(messageOffset,messageScope);if(message){activeMessage=message->sequence;mesh().markRead(activeMessage);}}
  auto& d = surface();
  d.startWrite();
  d.fillScreen(bg);
  const uint16_t radioColor = transportReady() ? green : scannerFault() ? TFT_RED : amber;
  d.fillRect(6, 7, 5, 5, radioColor);
  label(transportReady() ? "BLE" : scannerFault() ? "ERR" : "IDLE", 2,
        fg, 16, 59, compact);
  if(mesh().unreadCount()) {
    static const uint8_t envelope[]={0xff,0xc0,0xc0,0xc0,0xa1,0x40,0x92,0x40,0x8c,0x40,0x80,0x40,0xff,0xc0};
    bitmap(envelope,56,6,10,7,green);
    d.fillRect(68,5,2,2,green);
  }
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
  if (screen == Screen::Splash) { drawSplash(); d.endWrite(); if(buffered) canvas.pushSprite(0,0); return; }
  const char* title = "Home";
  switch (screen) {
    case Screen::Peers: title = "Nearby"; break;
    case Screen::Channels: title = detail ? "#bluetooth" : "Inbox"; break;
    case Screen::Messages: title = "Messages"; break;
    case Screen::Bot: title = "InfoBot"; break;
    case Screen::Settings: title = "Settings"; break;
    case Screen::Brightness: title = "Brightness"; break;
    case Screen::Battery: title = "Battery"; break;
    case Screen::About: title = "About"; break;
    case Screen::Diagnostics: title = "Diagnostics"; break;
    case Screen::Sound: title = "Sound"; break;
    case Screen::Joystick: title = "JoyC HAT"; break;
    case Screen::Reply: title = privateDraft?"Reply":"Write"; break;
    case Screen::ShortMessages: title = "Short msgs"; break;
    case Screen::Keyboard: title = "Keyboard"; break;
    case Screen::Confirm: title = queuedMessage ? "Queued" : sendFailed ? "Not sent" : "Confirm"; break;
    default: break;
  }
  label(title, 26, fg, 4, 127, &fonts::FreeMonoBold9pt7b);
  switch (screen) {
    case Screen::Dashboard: {
      panel(54, 69);
      std::snprintf(value, sizeof(value), "Nearby: %u", unsigned(peers.candidates())); label(value, 57);
      std::snprintf(value, sizeof(value), "P:%u New:%u", unsigned(mesh().peerCount()), unsigned(mesh().unreadCount())); label(value, 79, green);
      label(transportReady() && mesh().ready() ? "Relay: ON" : "Relay: OFF", 101, transportReady() && mesh().ready() ? green : amber);
      const char* options[] = {"Nearby", "Inbox", "InfoBot", "Settings"};
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
        label("Send DM",199,green,15,111,compact);
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
      if(detail){
        panel(54,50);label("Public chat",61,fg,13,109,compact);
        std::snprintf(value,sizeof(value),"New: %u",unsigned(mesh().unreadCount(MessageScope::Public)));label(value,83,green,13,109,compact);
        const char* actions[]={"Read chat","Write msg"};menu(actions,2,117,32);break;
      }
      panel(54,76);item("#bluetooth",60,selection==0,26);
      label("Public chat",94,fg,13,109,compact);
      std::snprintf(value,sizeof(value),"New: %u",unsigned(mesh().unreadCount(MessageScope::Public)));label(value,112,green,13,109,compact);
      panel(140,76);item("Direct DMs",146,selection==1,26);
      label("Private text",180,fg,13,109,compact);
      std::snprintf(value,sizeof(value),"New: %u",unsigned(mesh().unreadCount(MessageScope::Direct)));label(value,198,green,13,109,compact);
      label(transportStatus(),220,transportReady()?green:amber,7,123,compact);
      break;
    case Screen::Messages: {
      const auto count = mesh().messageCount(messageScope);
      if (!count) {
        panel(54, 117); label("No messages", 64); divider(92);
        label(messageScope==MessageScope::Direct?"Direct DMs":"#bluetooth",103,green,13,109,compact);
        label(transportStatus(),127,muted,13,109,compact);
        label("Waiting...",149,muted,13,109,compact);
      } else {
        if(messageOffset>=count) messageOffset=count-1;
        const auto* message=mesh().message(messageOffset,messageScope);
        std::snprintf(value,sizeof(value),"%u/%u %s",unsigned(messageOffset+1),unsigned(count),message->outgoing?"You":message->privateMessage?"DM":"BLE");label(value,55,muted,4,127,compact);
        panel(77, 159);label(message->name,84,green,13,109,compact);divider(106);
        const size_t length=std::strlen(message->text), lines=(length+11)/12;
        if(textLine>=lines) textLine=lines?lines-1:0;
        for(size_t line=0;line<5;++line){const size_t start=(textLine+line)*12;if(start>=length)break;char part[13]{};std::memcpy(part,message->text+start,length-start>12?12:length-start);label(part,115+int(line)*20,fg,13,109,compact);}
        if(detail)label("Reply",218,green,13,109,compact);
        else if(message->truncated)label("Truncated",218,amber,13,109,compact);
      }
      break;
    }
    case Screen::Reply: {
      panel(54,68,true);label(composer.recipientName,60,fg,13,109,compact);
      std::snprintf(value,sizeof(value),"%02x%02x%02x%02x",composer.recipient[0],composer.recipient[1],composer.recipient[2],composer.recipient[3]);
      label(privateDraft?value:"Public chat",84,muted,13,109,compact);label(privateDraft?"Private DM":"Everyone",103,green,13,109,compact);
      const char* options[]={"Short msg", "Keyboard"};menu(options,2,137,32);
      break;
    }
    case Screen::ShortMessages: {
      label(composer.recipientName,55,green,4,127,compact);
      std::snprintf(value,sizeof(value),"%u/%u",unsigned(selection+1),unsigned(MessageComposer::presetCount));label(value,76,muted,4,127,compact);
      const size_t first=selection<5?0:selection-4;
      const int thumb=123*5/int(MessageComposer::presetCount);
      surface().fillRect(132,101,2,123,headerBorder);
      surface().fillRect(132,101+int(first)*(123-thumb)/int(MessageComposer::presetCount-5),2,thumb,fg);
      panel(96,133);
      for(size_t row=0;row<5 && first+row<MessageComposer::presetCount;++row) {
        const int top=101+int(row)*25;
        item(MessageComposer::presets[first+row],top,selection==first+row,24);
        if(row<4 && first+row+1<MessageComposer::presetCount)divider(top+24,5,125);
      }
      break;
    }
    case Screen::Keyboard: {
      panel(54,70);label(composer.recipientName,57,green,9,117,compact);
      const size_t length=composer.length(),start=length>36?length-36:0;
      for(size_t row=0;row<3;++row){const size_t at=start+row*12;if(at>=length)break;char part[13]{};std::memcpy(part,composer.draft+at,length-at>12?12:length-at);label(part,76+int(row)*15,fg,9,117,compact);}
      std::snprintf(value,sizeof(value),"%u/192",unsigned(length));label(value,126,length==192?amber:muted,7,121,compact);
      for(size_t key=0;key<30;++key) {
        const int x=7+int(key%10)*12,y=146+int(key/10)*19;
        d.drawRect(x,y,12,19,headerBorder);if(selection==key)d.fillRect(x,y,12,19,fg);
        char character[]={composer.key(key),0};label(character,y+2,selection==key?bg:fg,x+2,10,compact);
      }
      const char* controls[]={composer.mode==0?"ABC":composer.mode==1?"123":"abc","SP","DEL","OK"};
      for(size_t key=0;key<4;++key){const int x=5+int(key)*31;d.drawRect(x,210,31,25,fg);if(selection==30+key)d.fillRect(x,210,31,25,fg);label(controls[key],215,selection==30+key?bg:fg,x+3,27,compact);}
      break;
    }
    case Screen::Confirm: {
      panel(54,44,true);label(composer.recipientName,58,fg,13,109,compact);
      std::snprintf(value,sizeof(value),"%02x%02x%02x%02x",composer.recipient[0],composer.recipient[1],composer.recipient[2],composer.recipient[3]);label(privateDraft?value:"Public chat",79,muted,13,109,compact);
      if(selection<reviewPages()&&!queuedMessage)reviewPage=selection;
      panel(104,80);if(selection<reviewPages()&&!queuedMessage)d.drawRect(4,104,127,80,green);
      for(size_t row=0;row<4;++row){const size_t at=reviewPage*48+row*12,length=composer.length();if(at>=length)break;char part[13]{};std::memcpy(part,composer.draft+at,length-at>12?12:length-at);label(part,109+int(row)*18,fg,13,109,compact);}
      if(queuedMessage) {
        const ChatMessage* queued=nullptr;
        for(size_t i=0;i<mesh().messageCount();++i)if(mesh().message(i)->sequence==queuedMessage)queued=mesh().message(i);
        const char* state=queued&&queued->delivery==DeliveryState::Read?"Read":queued&&queued->delivery==DeliveryState::Delivered?"Delivered":"Queued";
        label(state,185,green,13,109,compact);item("Done",213,true,22);
      }else{
        if(sendFailed)label(sendFailure,185,amber,13,109,compact);
        else {std::snprintf(value,sizeof(value),"Text %u/%u",unsigned(reviewPage+1),unsigned(reviewPages()));label(value,185,muted,13,109,compact);}
        item("Send",201,selection==reviewPages(),18);item("Cancel",220,selection==reviewPages()+1,18);
      }
      break;
    }
    case Screen::Bot: {
      panel(54,31);
      label(mesh().botStatus(),61,mesh().botEnabled()?green:amber,13,109,compact);
      if(detail) {
        panel(93,139);label(kBotCommands[selection],102,green);divider(129);
        label("Public + DM",140,fg,13,109,compact);
        const char* lines[][3]={{"Stick info","BLE relay","Private text"},
          {"Respect all","No spam","Public chat"},
          {"Send command","in chat / DM","Wait 3s"},
          {"Open-source","Wander Lima","Project link"}};
        for(int i=0;i<3;++i)label(lines[selection][i],166+i*20,fg,13,109,compact);
      }else{
        panel(93,139);
        for(size_t i=0;i<kBotCommandCount;++i)item(kBotCommands[i],98+int(i)*26,selection==i,22);
        item(mesh().botEnabled()?"Bot: ON":"Bot: OFF",202,selection==kBotCommandCount,22);
      }
      break;
    }
    case Screen::Settings:
      menu(settingsLabels, settingsCount, 54, 24);
      if (!settingsSaved) label("Not saved", 219, amber, 13,109,compact);
      break;
    case Screen::Sound: {
      const char* options[] = {"Interact", "DM alert", "Channel"};
      const uint8_t flags[] = {SoundUi, SoundDm, SoundMesh};
      panel(54,105);
      for (size_t i=0; i<3; ++i) {
        checkbox(options[i],59+int(i)*32,selection==i,preferenceFlags & flags[i]);
        if (i<2) divider(90+int(i)*32,5,125);
      }
      if (!settingsSaved) label("Not saved",219,amber,13,109,compact);
      break;
    }
    case Screen::Joystick:
      panel(54,73);
      checkbox("Click",59,selection==0,preferenceFlags & JoycClick);
      divider(90,5,125);
      checkbox("LED pulse",91,selection==1,preferenceFlags & JoycLed);
      panel(140,68);
      label(joycAxes.valid ? "Hat ready" : "No HAT",150,joycAxes.valid?green:amber);
      if (preferenceFlags & JoycClick) label(!joycAxes.buttonValid ? "Click error" : joycAxes.pressed ? "Pressed" : "Released",180,muted,13,109,compact);
      else label("Click off",180,muted,13,109,compact);
      if (!settingsSaved) label("Not saved",219,amber,13,109,compact);
      break;
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
      if (joycAxes.valid) {
        std::snprintf(value, sizeof(value), "J:%d,%d", int(joycAxes.x), int(joycAxes.y));
        label(value, 216, green, 13,109,compact);
      } else label(clockRtc() ? "RTC / v0.2" : "Build clock", 216, clockRtc() ? muted : amber, 13,109,compact);
      break;
    default: break;
  }
  d.endWrite();
  if(buffered) canvas.pushSprite(0,0);
}
void move(int direction, const PeerTable& peers) {
  if(screen==Screen::Channels&&detail){selection=(selection+2+direction)%2;return;}
  if(composing()) {
    const size_t count=screen==Screen::Reply?2:screen==Screen::ShortMessages?MessageComposer::presetCount:screen==Screen::Keyboard?MessageComposer::keyCount:queuedMessage?1:reviewPages()+2;
    selection=(selection+count+direction)%count;return;
  }
  if (screen == Screen::Messages) {
    if(detail) { const auto* message=mesh().message(messageOffset,messageScope);const size_t lines=message?(std::strlen(message->text)+11)/12:0;
      if(direction<0 && textLine) --textLine; else if(direction>0 && textLine+5<lines) ++textLine;
    } else if(mesh().messageCount(messageScope)){messageOffset=(messageOffset+mesh().messageCount(messageScope)+direction)%mesh().messageCount(messageScope);textLine=0;activeMessage=0;}
    return;
  }
  if (screen == Screen::Peers) {
    if(mesh().peerCount()) selection=(selection+mesh().peerCount()+direction)%mesh().peerCount();
    else peerSelection.move(direction, peers);
    detail = false;
    return;
  }
  const size_t count = screen == Screen::Dashboard ? 4 : screen == Screen::Settings ? settingsCount :
                       screen == Screen::Sound ? 3 : screen == Screen::Joystick ? 2 :
                       screen == Screen::Channels ? 2 : screen == Screen::Bot ? kBotCommandCount+1 : screen == Screen::Brightness ? 5 : 0;
  if (count) selection = (selection + count + direction) % count;
  detail = false;
}
} // namespace
void beginUi() {
  preferenceFlags = loadPreferenceFlags();
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
  syncMessageFocus();
  const bool aDown = M5.BtnA.isPressed();
  const bool bDown = M5.BtnB.isPressed();
  const bool cDown = M5.BtnPWR.isPressed();
  const auto buttons = input.update(aDown, bDown, cDown, now, sleeping);
  const bool buttonsBusy = aDown || bDown || cDown || buttons != NavigationAction::None;
  joycAxes = pollJoyc(now, preferenceFlags & JoycClick);
  const auto click = joycClick.update(joycAxes.pressed, joycAxes.valid && joycAxes.buttonValid,
                                     preferenceFlags & JoycClick, now, sleeping, buttonsBusy);
  const auto joystick = joycInput.update(joycAxes.x, joycAxes.y, joycAxes.valid, now, sleeping,
                                        buttonsBusy || click != NavigationAction::None);
  auto action = joystick;
  if (click != NavigationAction::None) action = click;
  if (buttons != NavigationAction::None) action = buttons;
  if (aDown || bDown || cDown || action != NavigationAction::None) lastInput = now;
  if (action == NavigationAction::Wake) {
    sleeping = false;
    M5.Display.setBrightness(brightness);
    lastDraw = 0;
  } else if (action == NavigationAction::Up) move(-1, peers);
  else if (action == NavigationAction::Down) move(1, peers);
  else if (action == NavigationAction::Back) {
    if(composing()) {
      if(screen==Screen::Reply || (screen==Screen::Confirm&&queuedMessage))closeComposer();
      else if(screen==Screen::Confirm){screen=editor;selection=editorSelection;sendFailed=false;}
      else {selection=screen==Screen::Keyboard?1:0;screen=Screen::Reply;}
    }
    else if(screen==Screen::Channels&&detail){detail=false;selection=0;}
    else if (detail && (screen == Screen::Peers || screen == Screen::Bot || screen == Screen::Messages)) detail = false;
    else if (settingsPageIndex(screen) >= 0) {
      selection = size_t(settingsPageIndex(screen));
      screen = Screen::Settings;
    } else if (screen != Screen::Splash) {
      const bool directInbox=screen==Screen::Messages && messageScope==MessageScope::Direct;
      screen = screen == Screen::Dashboard ? Screen::Splash :
               screen == Screen::Messages ? Screen::Channels : Screen::Dashboard;
      selection = directInbox ? 1 : 0;
      if(screen==Screen::Channels)detail=!directInbox;
    }
  } else if (action == NavigationAction::Select) {
    if(screen==Screen::Reply){screen=selection==0?Screen::ShortMessages:Screen::Keyboard;selection=0;}
    else if(screen==Screen::ShortMessages){composer.preset(selection);confirmDraft();}
    else if(screen==Screen::Keyboard){if(composer.selectKey(selection))confirmDraft();}
    else if(screen==Screen::Confirm){
      if(queuedMessage)closeComposer();
      else if(selection==reviewPages()+1)closeComposer();
      else if(selection==reviewPages()) {
        if(privateDraft?mesh().sendDirect(composer.recipient,composer.draft):mesh().sendText(composer.draft)){queuedMessage=mesh().message(0)->sequence;selection=0;sendFailed=false;}
        else {sendFailed=true;sendFailure=privateDraft?mesh().sendStatus():"Link / queue";}
      }
    }
    else if(screen==Screen::Peers&&mesh().peerCount()) {
      const auto* peer=mesh().peer(selection%mesh().peerCount());openComposer(peer->id,peer->name);
    }
    else if(screen==Screen::Messages&&detail) {
      const auto* message=mesh().message(messageOffset,messageScope);
      if(message){bool known=false;for(auto id:message->senderId)known|=id!=0;if(known)openComposer(message->senderId,message->name);}
    }
    else if (screen == Screen::Splash) screen = Screen::Dashboard;
    else if (screen == Screen::Dashboard) {
      const Screen destinations[] = {Screen::Peers, Screen::Channels, Screen::Bot, Screen::Settings};
      screen = destinations[selection]; selection = 0;
    } else if(screen==Screen::Channels){
      if(!detail&&selection==0){detail=true;selection=0;}
      else if(detail&&selection==1)openPublicComposer();
      else {messageScope=detail?MessageScope::Public:MessageScope::Direct;screen=Screen::Messages;messageOffset=textLine=0;activeMessage=0;detail=false;}
    }
    else if (screen == Screen::Bot && selection==kBotCommandCount) mesh().setBotEnabled(!mesh().botEnabled());
    else if (screen == Screen::Peers || screen == Screen::Bot || screen == Screen::Messages) { detail = !detail; textLine=0; }
    else if (screen == Screen::Settings) {
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
      M5.Display.setBrightness(brightness);
      screen = Screen::Settings;
      selection = 0;
    }
  }
  const auto& feedbackStats = mesh().stats();
  updateFeedback(now, preferenceFlags, action, feedbackStats.receivedDm, feedbackStats.receivedMesh);
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
