#include "ConnAckPacket.hpp"

using AsyncMqttClientInternals::ConnAckPacket;

ConnAckPacket::ConnAckPacket(ParsingInformation* parsingInformation, OnConnAckInternalCallback callback)
: _parsingInformation(parsingInformation)
, _callback(callback)
, _bytePosition(0)
, _sessionPresent(false)
, _connectReturnCode(0) {
}

ConnAckPacket::~ConnAckPacket() {
}

void ConnAckPacket::parseVariableHeader(uint8_t* data, size_t len, size_t* currentBytePosition) {
  uint8_t currentByte = data[(*currentBytePosition)++];
  if (_bytePosition++ == 0) {
    _sessionPresent = (currentByte << 7) >> 7;
  } else {
    _connectReturnCode = currentByte;
    _parsingInformation->bufferState = BufferState::NONE;
    _callback(_sessionPresent, _connectReturnCode);
  }
}

void ConnAckPacket::parsePayload(uint8_t* data, size_t len, size_t* currentBytePosition) {
  (void)data;
  (void)currentBytePosition;
}
