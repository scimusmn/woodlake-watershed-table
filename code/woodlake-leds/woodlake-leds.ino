#define SMM_IMPLEMENTATION
#include <OctoWS2811.h>
#include "smm.h"
#include "pollutant.h"
#include "rain.h"


#define STRIP_LEN 800
byte stripPins[] = { 2 };
DMAMEM int displayBuffer[STRIP_LEN*6];
OctoWS2811 strip(
  STRIP_LEN, displayBuffer, nullptr, 
  WS2811_RGB | WS2811_800kHz, sizeof(stripPins), stripPins
);



void setup() {
  Serial.begin(115200);
  Serial.println("== boot ==");

  smm::setup();

  strip.begin();
  memset(displayBuffer, 0, sizeof(displayBuffer));
  strip.show();

  setupPollutants();
  setupRain();
  phosphorus.ring.level = 50;

  Serial.println("setup complete.");
  delay(500);
}


void loop() {
  static unsigned long rainDelay = 0;
  bool water = true;
  if (millis() > 10000) {
    water = false;
  }

  if (millis() < 5000) {
    phosphorus.fill();
  } else {
    bool a, b;
    phosphorus.drain(water, a, b);
  }

  if (millis() - rainDelay > 25) {
    rainABC.update();
    stormABC.update();
    rainDelay = millis();
  }

  memset(displayBuffer, 0, sizeof(displayBuffer));

  drawRainZone(strip, rainABC);
  drawStormZone(strip, stormABC);

  // #define SERIAL_APPEND(x) Serial.print(x); Serial.print(" ");
  // SERIAL_APPEND(phosphorus.ring.level);
  // SERIAL_APPEND(phosphorus.path.polluteTrailStart);
  // SERIAL_APPEND(phosphorus.path.polluteTrailEnd);
  // Serial.println();

  drawPollutant(strip, phosphorus, 0x00ff00);
  strip.show();
}
