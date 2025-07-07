#pragma once
#include <OctoWS2811.h>
#include "PolledTimer.h"

class PollutantPath {
  private:
  bool flowing = false;
  int length();
  int addr(int n);
  unsigned long color;
  int start, end;
  int amt = 0;
  int offset = 0;
  PolledInterval interval;
  public:
  PollutantPath(int start, int end, unsigned long color);
  void startAccumulating(unsigned long ms);
  void stopAccumulating();
  void flush(unsigned long ms);
  void update();
  void render(OctoWS2811 &strip);
  void showPixel(OctoWS2811 &strip, int n, unsigned long color);
  void advance();
  static void advanceCb(void *ptr);
  void accumulate();
  static void accumulateCb(void *ptr);
};


#ifdef SMM_IMPLEMENTATION
void PollutantPath::advance() {
  offset += 1;
}
void PollutantPath::advanceCb(void *ptr) {
  PollutantPath *self = (PollutantPath*) ptr;
  self->advance();
}
void PollutantPath::accumulate() {
  amt += 1;
}
void PollutantPath::accumulateCb(void *ptr) {
  PollutantPath *self = (PollutantPath*) ptr;
  self->accumulate();
}
int PollutantPath::length() {
  if (start < end) {
    return end - start;
  } else {
    return start - end;
  }
}
int PollutantPath::addr(int n) {
  if (start < end) {
    return start + n;
  } else {
    return start - n - 1;
  }
}
PollutantPath::PollutantPath(int start, int end, unsigned long color)
: start(start), end(end), color(color) {}

void PollutantPath::startAccumulating(unsigned long ms) {
  if (!flowing) {
    interval.start(PollutantPath::accumulateCb, this, ms);
  }
}
void PollutantPath::stopAccumulating() {
  if (!flowing) {
    interval.clear();
  }
}
void PollutantPath::update() {
  interval.update();
  if (offset > length()) {
    flowing = false;
    offset = 0;
    amt = 0;
    interval.clear();
  }
}
void PollutantPath::flush(unsigned long ms) {
  flowing = true;
  interval.start(PollutantPath::advanceCb, this, ms);
}
void PollutantPath::render(OctoWS2811 &strip) {
  for (int i=0; i<amt; i++) {
    showPixel(strip, i+offset, color);
  }
}

void PollutantPath::showPixel(OctoWS2811 &strip, int n, unsigned long color) {
  if (n < 0 || n >= length()) {
    return;
  } else {
    strip.setPixel(addr(n), color);
  }
}
#endif
