#pragma once

#include "Arduino.h"
#include "Packet.hpp"
#include "../ParsingInformation.hpp"
#include "../Callbacks.hpp"

namespace AsyncMqttClientInternals {
class PubRelPacket : public Packet {
 public:
  explicit PubRelPacket(ParsingInformation* parsingInformation, OnPubRelInternalCallback callback);
  ~PubRelPacket();

  void parseVariableHeader(uint8_t* data, size_t len, size_t* currentBytePosition);
  void parsePayload(uint8_t* data, size_t len, size_t* currentBytePosition);

 private:
  ParsingInformation* _parsingInformation;
  OnPubRelInternalCallback _callback;

  uint8_t _bytePosition;
  uint8_t _packetIdMsb;
  uint16_t _packetId;
};
}  // namespace AsyncMqttClientInternals
