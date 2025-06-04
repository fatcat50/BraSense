#pragma once

namespace AsyncMqttClientInternals {
class Packet {
 public:
  virtual ~Packet() {}

  virtual void parseVariableHeader(uint8_t* data, size_t len, size_t* currentBytePosition) = 0;
  virtual void parsePayload(uint8_t* data, size_t len, size_t* currentBytePosition) = 0;
};
}  // namespace AsyncMqttClientInternals
