#include "messages.h"
#include <FlexCAN_T4.h>
static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> MsgCan;


static uint8_t selfId = 0xff;
uint8_t getNodeId() {
  return selfId;
}


uint8_t GenericCanMsg::nextId = 0;


GenericCanMsg::GenericCanMsg()
: next(nullptr)
{
  id = GenericCanMsg::nextId;
  GenericCanMsg::nextId += 1;
}

void GenericCanMsg::add(GenericCanMsg *msg) {
  if (next == nullptr) {
    next->verify();
    next = msg;
  } else {
    next->add(msg);
  }
}

void GenericCanMsg::handle(const CAN_message_t *msg) {
  uint8_t type = msg->id >> 4;
  if (type == this->id) {
    setBuffer(msg->buf);
    rx();
  } else if (next != nullptr) {
    next->handle(msg);
  }
}

void GenericCanMsg::tx() {
  CAN_message_t msg;
  msg.id = (this->id << 4) | selfId;
  memcpy(msg.buf, getBuffer(), getBufferSize());
  msg.len = getBufferSize();
  MsgCan.write(msg);
}

void GenericCanMsg::verify() {
  if (sizeof(CAN_message_t::buf) < getBufferSize()) {
    Serial.print(getName()); Serial.println(" will not fit into the CAN buffer!");
  }
}


WaterLevelMsg waterLevel;


void processMessage(const CAN_message_t &msg) {
  waterLevel.handle(&msg);
}


void setupCan(uint8_t id) {
  selfId = id;
  MsgCan.begin();
  MsgCan.setBaudRate(1000000);
  MsgCan.setMaxMB(16);
  MsgCan.enableFIFO();
  MsgCan.enableFIFOInterrupt();
  MsgCan.onReceive(processMessage);

  waterLevel.verify();
}
