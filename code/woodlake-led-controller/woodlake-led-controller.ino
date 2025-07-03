#define SMM_IMPLEMENTATION
#include "smm.h"
#include "messages.h"
#include "pins.h"
#include "PolledTimer.h"

class PollutantButton : public smm::Switch {
  public:
  PollutantButton(int pin) : smm::Switch(pin) {}
  unsigned long pressTime = 0;
  bool pressed;
  unsigned long level = 0;
  void onLow() {
    pressTime = millis();
    pressed = true;
  }
  void onHigh() {
    level += millis() - pressTime;
    pressTime = 0;
    pressed = false;
  }
  unsigned long getLevel() {
    if (pressed) {
      return level + millis() - pressTime;
    } else {
      return level;
    }
  }

};


uint8_t lightsState = 0;
void setLight(int n, int state) {
  if (state) {
    lightsState |= 1<<n;
  } else {
    lightsState &= ~(1<<n);
  }
  digitalWrite(SHIFT_LATCH, 0);
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, lightsState);
  digitalWrite(SHIFT_LATCH, 1);
}


class Button : public smm::Switch {
  public:
  Button(int n, int pin) : n(n), smm::Switch(pin) {}
  int n;
  bool pressed;
  void onLow() {
    Serial.print(pin); Serial.println(" pressed!");
    pressed = true;
  }
  void onHigh() {
    pressed = false;
  }
} button0(0, BUTTON_0)
, button1(1, BUTTON_1)
, button2(2, BUTTON_2)
, button3(3, BUTTON_3)
, button4(4, BUTTON_4)
, button5(5, BUTTON_5)
;




void setup() {
  Serial.begin(115200);
  Serial.println("== boot ==");

  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(SHIFT_LATCH, 1);

  smm::setup();

  setupCan(0x01);

  Serial.println("setup complete.");
  delay(500);
}


void loop() {
  setLight(button0.n, button0.pressed);
  setLight(button1.n, button1.pressed);
  setLight(button2.n, button2.pressed);
  setLight(button3.n, button3.pressed);
  setLight(button4.n, button4.pressed);
  setLight(button5.n, button5.pressed);
  delay(100);
  // static unsigned long t = 100;
  // if (t < millis()) {
  //   t += 100;
  //   Serial.println(phosphorus.getLevel());
  // }
  // phosphorus.interval.update();
  // printInterval.update();
}
