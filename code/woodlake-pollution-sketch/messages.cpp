#include "messages.h"
#include "state.h"
#include <FlexCAN_T4.h>
static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> MsgCan;


static uint8_t selfId = 0xff;
uint8_t getNodeId() {
  return selfId;
}


GenericCanMsg::GenericCanMsg(uint8_t id)
: id(id), next(nullptr)
{}

void GenericCanMsg::add(GenericCanMsg *msg) {
  if (next == nullptr) {
    msg->verify();
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
PollutantOutputMsg pollutantOutput;


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

  Serial.println(sizeof(WaterLevelMsg));

  waterLevel.verify();
  waterLevel.add(&pollutantOutput);
}


WaterLevelMsg::WaterLevelMsg() : GenericCanMsg(WATER_LEVEL) {}
const char *WaterLevelMsg::getName() { return "WaterLevelMsg"; };
void * WaterLevelMsg::getBuffer() { return &level; }
void WaterLevelMsg::setBuffer(const void *buf) { level = *(uint8_t*)buf; }
size_t WaterLevelMsg::getBufferSize() { return sizeof(level); }
void WaterLevelMsg::rx() {
  Serial.print("set water level to "); Serial.println(level);
}





PollutantOutputMsg::PollutantOutputMsg() 
: pollutantId(0), amt(0), GenericCanMsg(POLLUTANT_OUTPUT) {}
PollutantOutputMsg::PollutantOutputMsg(int pollutantId, int amt)
: pollutantId(pollutantId), amt(amt), GenericCanMsg(POLLUTANT_OUTPUT) {}
const char * PollutantOutputMsg::getName() { return "PollutantOutputMsg"; }

void * PollutantOutputMsg::getBuffer() {
  buf[0] = pollutantId;
  buf[1] = amt;
  return buf;
}

void PollutantOutputMsg::setBuffer(const void *buf) {
  pollutantId = ((uint8_t*) buf)[0];
  amt = ((uint8_t*) buf)[1];
}

size_t PollutantOutputMsg::getBufferSize() { return sizeof(buf); }

void PollutantOutputMsg::rx() {
  bool setLevel = true;
  Serial.print(pollutantId); Serial.println(" received!");
  switch (pollutantId) {
    case 0:
    pollutant0.startFlow(amt);
    break;

    case 1:
    pollutant1.startFlow(amt);
    break;

    case 2:
    pollutant2.startFlow(amt);
    break;

    case 3:
    pollutant3.startFlow(amt);
    break;

    default:
    setLevel = false;
    break;
  }

  if (setLevel) {
    targetLevel = LAKE_HIGH;
    levelTimer.start([](void*) { targetLevel = LAKE_LOW; }, 5000);
  }
}
