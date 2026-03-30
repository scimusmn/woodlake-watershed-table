#include <stdlib.h>
#include "rain.h"
#include "led-map.h"


void RainStrip::update(bool raining) {
  bool newDrop = raining && (rand() < rate);
  drops = (drops << 1) | (newDrop ? 1 : 0);
}



void RainZone::setup(
  int start0, int end0, 
  int start1, int end1, 
  int start2, int end2, 
  int rate
) {
  r0.start = start0;
  r0.end = end0;

  r1.start = start1;
  r1.end = end1;

  r2.start = start2;
  r2.end = end2;

  r0.rate = rate; r0.drops = 0;
  r1.rate = rate; r1.drops = 0;
  r2.rate = rate; r2.drops = 0;
}


void RainZone::update(bool raining) {
  r0.update(raining);
  r1.update(raining);
  r2.update(raining);
}


void StormZone::update(bool storming) {
  if (storming && rand() < lightningRate) {
    lightning = 1;
  } else {
    lightning = 0;
  }

  RainZone::update(storming);
}


void drawRainStrip(OctoWS2811 &strip, RainStrip &rain) {
  for (int i=0; i<_length(rain.start, rain.end); i++) {
    if (rain.drops & 1<<i) {
      strip.setPixel(_addr(rain.start, rain.end, i), 0x0000ff);
    }
  }
}


void drawLightning(OctoWS2811 &strip, RainStrip &rain) {
  for (int i=0; i<_length(rain.start, rain.end); i++) {
    strip.setPixel(_addr(rain.start, rain.end, i), 0xffffff);
  }
}



void drawRainZone(OctoWS2811 &strip, RainZone &zone) {
  drawRainStrip(strip, zone.r0);
  drawRainStrip(strip, zone.r1);
  drawRainStrip(strip, zone.r2);
}


void drawStormZone(OctoWS2811 &strip, StormZone &zone) {
  if (zone.lightning) {
    drawLightning(strip, zone.r0);
    drawLightning(strip, zone.r1);
    drawLightning(strip, zone.r2);
  } else {
    drawRainStrip(strip, zone.r0);
    drawRainStrip(strip, zone.r1);
    drawRainStrip(strip, zone.r2);
  }
}


RainZone rainABC;
RainZone rainDEF;
StormZone stormABC;
StormZone stormDEF;

void setupRain() {
  rainABC.setup(
    RAINA_START, RAINA_END,
    RAINB_START, RAINB_END,
    RAINC_START, RAINC_END,
    RAND_MAX >> 3
  );

  rainDEF.setup(
    RAIND_START, RAIND_END,
    RAINE_START, RAINE_END,
    RAINF_START, RAINF_END,
    RAND_MAX >> 3
  );

  stormABC.setup(
    STORMA_START, STORMA_END,
    STORMB_START, STORMB_END,
    STORMC_START, STORMC_END,
    RAND_MAX >> 2
  );

  stormDEF.setup(
    STORMD_START, STORMD_END,
    STORME_START, STORME_END,
    STORMF_START, STORMF_END,
    RAND_MAX >> 2
  );

  stormABC.lightning = 0;
  stormDEF.lightning = 0;
  stormABC.lightningRate = RAND_MAX >> 5;
  stormDEF.lightningRate = RAND_MAX >> 5;
}
