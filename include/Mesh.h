#pragma once
#include "Identity.h"
#include "NoiseSession.h"
#include "FragmentAssembler.h"
#include <array>
namespace stick {
constexpr uint16_t kAnyLink=0xffff;
struct MeshPeer {
  bool used=false, authenticated=false;
  uint8_t id[8]{}, noiseKey[32]{}, signingKey[32]{};
  char name[25]{};
  uint32_t seen=0, lastBot=0;
  bool botReplied=false;
};
struct ChatMessage { char name[25]{}, text[193]{}; bool outgoing=false, truncated=false; };
struct MeshStats { uint32_t accepted=0,rejected=0,relayed=0,sent=0,dropped=0,botReplies=0; };
// Single-owner loop model. Callbacks must enqueue before calling receive().
class Mesh {
 public:
  using Send=bool (*)(wire::Bytes,uint16_t target,uint16_t excluded);
  void begin(Identity& identity,Identity::Entropy entropy,Send send);
  void tick(uint32_t now,uint64_t epochMs);
  void receive(wire::Bytes data,uint16_t link);
  void disconnected(uint16_t link);
  bool announce(uint16_t target=kAnyLink);
  bool sendText(const char* text);
  size_t peerCount() const;
  size_t authenticatedCount() const;
  const MeshPeer* peer(size_t ordinal) const;
  size_t messageCount() const {return messageCount_;}
  const ChatMessage* message(size_t newestOffset) const;
  const MeshStats& stats() const {return stats_;}
  bool ready() const {return identity_ && identity_->ready() && epoch_!=0;}
 private:
  struct Session {NoiseSession noise;uint8_t id[8]{};uint16_t link=kAnyLink;uint32_t started=0;bool used=false,initiator=false;};
  bool emit(wire::FrameView& frame,uint16_t target=kAnyLink,uint16_t except=kAnyLink,bool sign=true);
  void process(wire::FrameView& frame,uint16_t link);
  MeshPeer* find(const uint8_t* id);
  Session* session(const uint8_t* id,uint16_t link);
  bool duplicate(const wire::FrameView& frame);
  void store(const char* name,wire::Bytes text,bool outgoing);
  void startNoise(MeshPeer& peer,uint16_t link);
  void handleNoise(const wire::FrameView& frame,uint16_t link);
  void bot(MeshPeer& peer,wire::Bytes text);
  Identity* identity_=nullptr;Identity::Entropy entropy_=nullptr;Send send_=nullptr;
  uint32_t now_=0,lastAnnounce_=0,lastBot_=0;uint64_t epoch_=0,lastStamp_=0;bool replied_=false;
  std::array<MeshPeer,12> peers_{};std::array<Session,2> sessions_{};
  std::array<ChatMessage,12> messages_{};size_t messageCount_=0,messageNext_=0;
  std::array<std::array<uint8_t,16>,64> dedup_{};size_t dedupCount_=0,dedupNext_=0;
  wire::FragmentAssembler assembler_;
  uint8_t assembled_[4096]{},scratch_[4096]{},out_[4096]{},plain_[2048]{};
  MeshStats stats_{};
};
Mesh& mesh();
}
