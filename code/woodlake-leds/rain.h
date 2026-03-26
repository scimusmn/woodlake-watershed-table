#ifndef RAIN_H
#define RAIN_H

#include <OctoWS2811.h>

struct RainStrip {
  void update();

  int start, end;
  int rate;
  unsigned int drops;
};


struct RainZone {
  void setup(int start0, int end0, int start1, int end1, int start2, int end2, int rate);
  void update();

  RainStrip r0, r1, r2;
};


struct StormZone : public RainZone {
  void update();
  int lightning, lightningRate;
};


extern RainZone rainABC;
extern StormZone stormABC;


void setupRain();


void drawRainStrip(OctoWS2811&, RainStrip&);
void drawRainZone(OctoWS2811&, RainZone&);
void drawStormZone(OctoWS2811&, StormZone&);

#endif
