#include "Scanner.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <atomic>
#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace stick {
namespace {
// Official BLEService.swift at 04995f68e470a18d1fc99f86fa326c2a87bab4f2.
// A matching advertisement is only a candidate, never an authenticated peer.
const NimBLEUUID mainnet("F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C");
const NimBLEUUID testnet("F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5A");
QueueHandle_t queue = nullptr;
NimBLEScan* scan = nullptr;
std::atomic<uint32_t> dropped{0};
std::atomic<uint32_t> reports{0};
uint32_t lastAttempt = 0;
bool fault = true;
class Callbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* device) override {
    ++reports; // All advertisements remain visible in Diagnostics.
    const auto network = device->isAdvertisingService(mainnet) ? Network::Mainnet :
                         device->isAdvertisingService(testnet) ? Network::Testnet : Network::Other;
    if (network == Network::Other) return; // Keep unrelated BLE traffic out of the queue/table.
    Observation value{};
    value.network = network;
    const auto address = device->getAddress();
    std::snprintf(value.address, sizeof(value.address), "%s", address.toString().c_str());
    value.addressType = address.getType();
    const auto name = device->getName();
    // Fixed buffer; strip control bytes and unsupported glyphs from radio input.
    size_t n = 0;
    for (const unsigned char c : name) {
      if (n == sizeof(value.name) - 1) break;
      value.name[n++] = (c >= 32 && c <= 126) ? char(c) : '?';
    }
    value.rssi = device->getRSSI();
    value.seenAt = millis();
    if (xQueueSend(queue, &value, 0) != pdTRUE) ++dropped;
  }
} callbacks;
} // namespace
bool beginScanner() {
  queue = xQueueCreate(32, sizeof(Observation));
  if (!queue || !NimBLEDevice::init("")) return false;
  scan = NimBLEDevice::getScan();
  if (!scan) return false;
  scan->setScanCallbacks(&callbacks, true);
  scan->setDuplicateFilter(false);
  scan->setMaxResults(0); // callback-only: no growing library result history.
  scan->setActiveScan(false); // passive candidate discovery alongside the GATT peripheral.
  scan->setInterval(160);
  scan->setWindow(80);
  lastAttempt = millis();
  fault = !scan->start(5000);
  return !fault;
}
void pollScanner(PeerTable& peers, uint32_t now) {
  Observation value{};
  // Bound work per frame even under an advertising flood.
  for (unsigned i = 0; queue && i < 32 && xQueueReceive(queue, &value, 0) == pdTRUE; ++i)
    peers.observe(value);
  peers.expire(now);
  if (scan && !scan->isScanning() && uint32_t(now - lastAttempt) >= 7000) {
    lastAttempt = now;
    fault = !scan->start(5000);
  }
}
bool scannerActive() { return scan && scan->isScanning(); }
bool scannerFault() { return fault; }
uint32_t scannerDrops() { return dropped.load(); }
uint32_t scannerReports() { return reports.load(); }
} // namespace stick
