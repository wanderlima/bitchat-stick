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
  uint16_t link=0xffff;
};
enum class MessageScope {All, Public, Direct};
enum class DeliveryState { Queued, Delivered, Read };
struct ChatMessage {DeliveryState delivery=DeliveryState::Queued;uint32_t sequence=0;bool unread=false,readReceiptSent=false;uint8_t senderId[8]{},messageId[64]{},messageIdLength=0; char name[25]{}, text[193]{}; bool outgoing=false, truncated=false, privateMessage=false; };
struct MeshStats { uint32_t accepted=0,rejected=0,relayed=0,sent=0,dropped=0,botReplies=0,receivedDm=0,receivedMesh=0,botIntroductions=0; };
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
  // Queues only through a live authenticated private session. Never broadcasts plaintext.
  bool sendDirect(const uint8_t recipient[8],const char* text);
  bool botEnabled() const {return botEnabled_;}
  void setBotEnabled(bool enabled) {botEnabled_=enabled;botStatus_=enabled?"Waiting":"Bot OFF";}
  const char* botStatus() const {return botEnabled_?botStatus_:"Bot OFF";}
  size_t peerCount() const;
  size_t authenticatedCount() const;
  const MeshPeer* peer(size_t ordinal) const;
  size_t messageCount(MessageScope scope=MessageScope::All) const;
  const ChatMessage* message(size_t newestOffset,MessageScope scope=MessageScope::All) const;
  size_t unreadCount(MessageScope scope=MessageScope::All) const;
  void markRead(uint32_t sequence);
  const MeshStats& stats() const {return stats_;}
  const char* lastReject() const {return lastReject_;}
  const char* sendStatus() const {return sendStatus_;}
  bool ready() const {return identity_ && identity_->ready() && epoch_!=0;}
 private:
  struct Session {NoiseSession noise;uint8_t id[8]{};uint16_t link=kAnyLink;uint32_t started=0;bool used=false,initiator=false;};
  bool emit(wire::FrameView& frame,uint16_t target=kAnyLink,uint16_t except=kAnyLink,bool sign=true);
  void process(wire::FrameView& frame,uint16_t link);
  MeshPeer* find(const uint8_t* id);
  Session* session(const uint8_t* id,uint16_t link);
  bool duplicate(const wire::FrameView& frame);
  bool rememberDigest(const uint8_t digest[32]);
  ChatMessage* store(const char* name,wire::Bytes text,bool outgoing,bool privateMessage=false);
  bool sendReadReceipt(ChatMessage& message);
  void startNoise(MeshPeer& peer,uint16_t link);
  void handleNoise(const wire::FrameView& frame,uint16_t link);
  void bot(MeshPeer& peer,wire::Bytes text,uint64_t timestamp,bool direct=false);
  void introduce(MeshPeer& peer,uint64_t timestamp,bool command);
  Identity* identity_=nullptr;Identity::Entropy entropy_=nullptr;Send send_=nullptr;
  uint32_t now_=0,lastAnnounce_=0,lastBot_=0,lastReceiptAttempt_=0;uint64_t epoch_=0,lastStamp_=0;bool replied_=false,botEnabled_=true;
  std::array<MeshPeer,12> peers_{};std::array<Session,12> sessions_{};
  std::array<ChatMessage,12> messages_{};size_t messageCount_=0,messageNext_=0;uint32_t messageSequence_=0;
  std::array<std::array<uint8_t,16>,64> dedup_{};size_t dedupCount_=0,dedupNext_=0;
  wire::FragmentAssembler assembler_;
  uint8_t assembled_[4096]{},scratch_[4096]{},out_[4096]{},plain_[2048]{};
  // No eviction: reconnect/peer expiry must not repeat an introduction this boot.
  std::array<std::array<uint8_t,8>,32> introduced_{};size_t introducedCount_=0;
  const char* sendStatus_="Ready";
  const char* lastReject_="none";
  const char* botStatus_="Waiting";
  MeshStats stats_{};
};
Mesh& mesh();
}
