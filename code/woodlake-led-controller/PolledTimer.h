#pragma once

class PolledTimer {
  protected:
  unsigned long endTime = 0;
  void (*fn)(void *) = nullptr;
  void *data = nullptr;
  public:
  void start(void (*fn)(void *), unsigned long t);
  void start(void (*fn)(void *), void *data, unsigned long t);
  void clear();
  void update();
};


class PolledInterval : public PolledTimer {
  protected:
  unsigned long delta;
  public:
  void start(void (*fn)(void *), unsigned long t);
  void start(void (*fn)(void *), void *data, unsigned long t);
  void update();
};


#ifdef SMM_IMPLEMENTATION

void PolledTimer::start(void (*fn)(void*), unsigned long t) {
  endTime = millis() + t;
  this->fn = fn;
  data = nullptr;
}


void PolledTimer::start(void (*fn)(void *), void *data, unsigned long t) {
  endTime = millis() + t;
  this->fn = fn;
  data = data;
}


void PolledTimer::clear() {
  endTime = 0;
  fn = nullptr;
  data = nullptr;
}


void PolledTimer::update() {
  if (fn == nullptr) {
    return;
  }
  if (millis() > endTime) {
    fn(data);
    clear();
  }
}

void PolledInterval::start(void (*fn)(void *), unsigned long t) {
  delta = t;
  PolledTimer::start(fn, t);
}


void PolledInterval::start(void (*fn)(void *), void *data, unsigned long t) {
  delta = t;
  PolledTimer::start(fn, data, t);
}

void PolledInterval::update() {
  if (fn == nullptr) {
    return;
  }
  if (millis() > endTime) {
    fn(data);
    endTime = millis() + delta;
  }
}

#endif
