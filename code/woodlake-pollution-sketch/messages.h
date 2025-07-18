#pragma once
#include <FlexCAN_T4.h>


void setupCan(uint8_t id);
uint8_t getNodeId();


class GenericCanMsg {
  protected:
  uint8_t id;
  GenericCanMsg *next;
  public:
  GenericCanMsg(uint8_t id);
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


enum CanMsgType {
  WATER_LEVEL,
  POLLUTANT_OUTPUT,
};


struct WaterLevelMsg : public GenericCanMsg {
  uint8_t level;

  WaterLevelMsg();
  const char *getName();
  void * getBuffer();
  void setBuffer(const void *buf);
  size_t getBufferSize();
  void rx();
};


struct PollutantOutputMsg : public GenericCanMsg {
  uint8_t pollutantId;
  uint8_t amt;
  uint8_t buf[2];

  PollutantOutputMsg();
  PollutantOutputMsg(int pollutantId, int amt);
  const char *getName();
  void *getBuffer();
  void setBuffer(const void *buf);
  size_t getBufferSize();
  void rx();
};
