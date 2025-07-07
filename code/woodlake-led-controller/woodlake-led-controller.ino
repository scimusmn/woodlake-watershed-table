#define SMM_IMPLEMENTATION
#include <OctoWS2811.h>
#include "smm.h"
#include "messages.h"
#include "pins.h"
#include "PolledTimer.h"
#include "pollutant.h"
#include "strips.h"

#define STRIP_LEN 200
DMAMEM int displayBuffer[STRIP_LEN*6];
byte stripPins[] = { 8, 9, 10, 11, 12 };
OctoWS2811 strip(
  STRIP_LEN, displayBuffer, nullptr, 
  WS2811_RGB | WS2811_800kHz, sizeof(stripPins), stripPins
);


class Button : public smm::Switch {
  public:
  Button(int n, int pin, int startAddr, int endAddr, unsigned int color) 
  : n(n), path(startAddr, endAddr, color), smm::Switch(pin) {}
  PollutantPath path;
  int n;
  bool pressed;
  void onLow() {
    Serial.print(n); Serial.println(" pressed");
    path.startAccumulating(100);
  }
  void onHigh() {
    path.stopAccumulating();
  }
  void u() {
    path.update();
    path.render(strip);
  }
} button0(0, BUTTON_0, POLLUTANT_0A, POLLUTANT_0B, 0xff0000)
, button1(1, BUTTON_1, POLLUTANT_0A, POLLUTANT_0B, 0xff0000)
, button2(2, BUTTON_2, POLLUTANT_0A, POLLUTANT_0B, 0xff0000)
, button3(3, BUTTON_3, POLLUTANT_0A, POLLUTANT_0B, 0xff0000)
, button4(4, BUTTON_4, POLLUTANT_0A, POLLUTANT_0B, 0xff0000)
, button5(5, BUTTON_5, POLLUTANT_0A, POLLUTANT_0B, 0xff0000)
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
  strip.begin();
  memset(displayBuffer, 0, sizeof(displayBuffer));
  strip.show();

  Serial.println("setup complete.");
  delay(500);
}


void loop() {
  memset(displayBuffer, 0, sizeof(displayBuffer));

  button0.u();

  strip.show();
}
