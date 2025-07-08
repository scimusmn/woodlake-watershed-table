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
byte stripPins[] = { 16, 17, 18, 19, 12 };
OctoWS2811 strip(
  STRIP_LEN, displayBuffer, nullptr, 
  WS2811_RGB | WS2811_800kHz, sizeof(stripPins), stripPins
);

unsigned long Time;

class Button : public smm::Switch {
  public:
  Button(int n, int pin, int startAddr, int endAddr, unsigned int color, int ms) 
  : n(n), path(startAddr, endAddr, color), smm::Switch(pin),ms(ms) {}
  PollutantPath path;
  int n;
  int ms;
  bool pressed;
  bool active;
  bool Hold;
  void onLow() {
    Serial.print(n); Serial.println(" pressed");
    path.startAccumulating(ms);
  }
  void onHigh() {
    if(Hold == false){path.stopAccumulating();}
    active = true;
  }
  void drain() {
    path.flush(ms/2);
  }
  void u() {
    path.update(n);
    path.render(strip);
  }
} button0(0, BUTTON_0, POLLUTANT_0A, POLLUTANT_0B, 0xffffff,100)
, button1(1, BUTTON_1, POLLUTANT_1A, POLLUTANT_1B, 0xff0000,100)
, button2(2, BUTTON_2, POLLUTANT_2A, POLLUTANT_2B, 0x00ffff,100)
, button3(3, BUTTON_3, POLLUTANT_3A, POLLUTANT_3B, 0xffff00,100)
, button4(4, BUTTON_4, POLLUTANT_4A, POLLUTANT_4B, 0x0000ff,20)
, button5(5, BUTTON_5, POLLUTANT_4A, POLLUTANT_4A, 0x0000ff,20)
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
  if(digitalRead(BUTTON_4) == LOW || digitalRead(BUTTON_5) == LOW){

    button4.Hold = true;
    button5.Hold = true;
    button4.active = true;
    button5.active = true;
    Time = millis();

    if(button0.active == true){
      button0.drain(); 
      button0.active = false;
    }

    if(button1.active == true){
      button1.drain(); 
      button1.active = false;
    }

    if(button2.active == true){
      button2.drain(); 
      button2.active = false;
    }

    if(button3.active == true){
      button3.drain(); 
      button3.active = false;
    }
  }
  else if(button4.active == true){
    if(millis() > 5000 + Time){
      button4.Hold = false;
      button4.drain();
      button4.active = false;
    }

    if(button0.active == true){
      button0.drain(); 
      button0.active = false;
    }

    if(button1.active == true){
      button1.drain(); 
      button1.active = false;
    }

    if(button2.active == true){
      button2.drain(); 
      button2.active = false;
    }

    if(button3.active == true){
      button3.drain(); 
      button3.active = false;
    }
  }

  button0.u();
  button1.u();
  button2.u();
  button3.u();
  button4.u();
  strip.show();

}
