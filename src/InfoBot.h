#pragma once
#include "WireCodec.h"
#include <cstring>
namespace stick {
constexpr size_t kBotCommandCount=4;
inline constexpr const char* kBotCommands[]={"!info","!rules","!help","!about"};
// Commands may have surrounding whitespace and ASCII case differences.
inline const char* infoBotReply(wire::Bytes input) {
  auto space=[](uint8_t c){return c==' '||c=='\t'||c=='\r'||c=='\n';};
  while(input.size&&space(input.data[0])){++input.data;--input.size;}
  while(input.size&&space(input.data[input.size-1]))--input.size;
  auto is=[&](const char* word){if(input.size!=std::strlen(word))return false;for(size_t i=0;i<input.size;++i){uint8_t c=input.data[i];if(c>='A'&&c<='Z')c+=32;if(c!=uint8_t(word[i]))return false;}return true;};
  if(is("!info"))return "InfoBot: Bitchat Stick | M5StickC Plus2 | #bluetooth local relay and private text DMs.";
  if(is("!rules")||is("!regras"))return "InfoBot: Respect others. No spam. #bluetooth is public; use DM for private text. Automatic welcomes are disabled.";
  if(is("!help")||is("!ajuda"))return "InfoBot: !info !rules !help !about. Use #bluetooth or DM Bitchat Stick. Wait 3s between commands. Aliases: !regras !ajuda.";
  if(is("!about"))return "Bitchat Stick: open-source Bitchat for your pocket, by Wander Lima. Project and source: https://github.com/wanderlima/bitchat-stick";
  return nullptr;
}
}
