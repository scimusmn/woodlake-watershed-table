#pragma once
#include <FlexCAN_T4.h>


void setupCan(uint8_t id);
uint8_t getNodeId();


class GenericCanMsg {
  protected:
  static uint8_t nextId;
  uint8_t id;
  GenericCanMsg *next;
  public:
  GenericCanMsg();
  void add(GenericCanMsg *msg);
  void handle(const CAN_message_t *msg);
  void tx();
  void verify();

  virtual const char *getName() = 0;
  virtual void * getBuffer() = 0;
  virtual void setBuffer(const void *buf) = 0;
  virtual size_t getBufferSize() = 0;
  virtual void rx() = 0;
};


class WaterLevelMsg : public GenericCanMsg {
  public:
  uint8_t level;
  const char *getName() { return "WaterLevelMsg"; };
  void * getBuffer() { return &level; }
  void setBuffer(const void *buf) { level = *(uint8_t*)buf; }
  size_t getBufferSize() { return sizeof(level); }
  void rx() {
    Serial.print("set water level to "); Serial.println(level);
  }
};
