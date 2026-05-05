#define SMM_IMPLEMENTATION
#include <OctoWS2811.h>
#include <Wire.h>
#include "smm.h"
#include "pins.h"
#include "pollutant.h"
#include "rain.h"
#include "comms.h"
#include "led-map.h"


#define STRIP_LEN 834
byte stripPins[] = { 9, 8, 7, 4, 3, 2 };
DMAMEM int displayBuffer[STRIP_LEN*6];
int drawBuffer[STRIP_LEN*6];
OctoWS2811 strip(
  STRIP_LEN, displayBuffer, drawBuffer,
  WS2811_RGB | WS2811_800kHz, sizeof(stripPins), stripPins
);


class Button : public smm::Switch {
    public:
    bool pressed;
    int pin;
    Button(int pin) : smm::Switch(pin), pin(pin), pressed(false) {}
    void onLow() {
      // press
      pressed = true;
      Serial.print("press "); Serial.println(pin);
    }
    void onHigh() {
      // release
      pressed = false;
      Serial.print("release "); Serial.println(pin);
    }
};


Button phosphorusBtn(PHOSPHORUS_BTN_PIN);
Button saltBtn(SALT_BTN_PIN);
Button sedimentBtn(SEDIMENT_BTN_PIN);
Button trashBtn(TRASH_BTN_PIN);

Button rainA(RAINA_BTN_PIN);
Button rainB(RAINB_BTN_PIN);
Button stormA(STORMA_BTN_PIN);
Button stormB(STORMB_BTN_PIN);


void setup() {
  Serial.begin(115200);
  Serial.println("== boot ==");

  Wire.begin();

  smm::setup();

  strip.begin();
  memset(drawBuffer, 0, sizeof(drawBuffer));
  strip.show();

  setupPollutants();
  setupRain();

  Serial.println("setup complete.");
  delay(500);
}



void loop() {
  static unsigned long emissionTick = 0;
  static unsigned long weatherTick = 0;
  static unsigned long weatherEnd = 0;
  static bool raining = false;
  static bool storming = false;

  // process weather inputs
  if ((rainA.pressed || rainB.pressed) && !storming && !storming) {
    raining = true;
    matrixActivate(MATRIX_RAIN);
    weatherEnd = millis()+8000;
    phosphorus.drain(true);
    salt.drain(true);
    sediment.drain(true);
    trash.drain(true);
  }
  if ((stormA.pressed || stormB.pressed) && !raining && !storming) {
    storming = true;
    matrixActivate(MATRIX_STORM);
    weatherEnd = millis() + 8000;
    phosphorus.drain(true);
    salt.drain(true);
    sediment.drain(true);
    trash.drain(true);
  }

  // update weather
  if (millis() > weatherEnd) {
    raining = false;
    storming = false;
    matrixDeactivate(MATRIX_RAIN);
    matrixDeactivate(MATRIX_STORM);
  }
  if (millis() > weatherTick) {
    rainABC.update(raining);
    rainDEF.update(raining);
    stormABC.update(storming);
    stormDEF.update(storming);
    weatherTick = millis()+25;
  }
  
  // update pollutants
  if (phosphorus.draining) {
    phosphorus.drain(raining || storming);
  }
  if (!phosphorus.draining && phosphorusBtn.pressed) {
    phosphorus.fill();
  }

  if (salt.draining) {
    salt.drain(raining || storming);
  }
  if (!salt.draining && saltBtn.pressed) {
    salt.fill();
  }

  if (sediment.draining) {
    sediment.drain(raining || storming);
  }
  if (!sediment.draining && sedimentBtn.pressed) {
    sediment.fill();
  }

  if (trash.draining) {
    trash.drain(raining || storming);
  }
  if (!trash.draining && trashBtn.pressed) {
    trash.fill();
  }

  if (millis() > emissionTick) {
    matrixSet(MATRIX_PHOSPHORUS, phosphorus.emit);
    matrixSet(MATRIX_SALT, salt.emit);
    matrixSet(MATRIX_SEDIMENT, sediment.emit);
    matrixSet(MATRIX_TRASH, trash.emit);
  }

  // draw to led strip
  memset(drawBuffer, 0, sizeof(drawBuffer));

  // under-table lake
  for (size_t i=LAKE_CORE_START; i<LAKE_CORE_END; i++) {
    strip.setPixel(i, 0x0000ff);
  }


  drawRainZone(strip, rainABC);
  drawRainZone(strip, rainDEF);
  drawStormZone(strip, stormABC);
  drawStormZone(strip, stormDEF);

  drawPollutant(strip, phosphorus, 0xff0000);
  drawPollutant(strip, salt,       0xffffff);
  drawPollutant(strip, sediment,   0x3fff00);
  drawPollutant(strip, trash,      0x00ff00);

  strip.show();
}
