#pragma once
#include <cstddef>
#include <cstring>
#include <deque>
#include <memory>
#include <vector>
struct FakeQueue { size_t capacity, itemSize; std::deque<std::vector<unsigned char>> items; };
using QueueHandle_t = FakeQueue*;
inline std::vector<std::unique_ptr<FakeQueue>> fakeQueues;
inline QueueHandle_t xQueueCreate(size_t count, size_t size) {
  fakeQueues.emplace_back(new FakeQueue{count,size,{}});return fakeQueues.back().get();
}
inline int xQueueSend(QueueHandle_t q,const void* value,unsigned) {
  if(q->items.size()==q->capacity)return 0;
  const auto* bytes=static_cast<const unsigned char*>(value);
  q->items.emplace_back(bytes,bytes+q->itemSize);return 1;
}
inline int xQueueReceive(QueueHandle_t q,void* value,unsigned) {
  if(q->items.empty())return 0;
  std::memcpy(value,q->items.front().data(),q->itemSize);q->items.pop_front();return 1;
}
