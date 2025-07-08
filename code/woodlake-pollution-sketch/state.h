#pragma once

#include "PolledTimer.h"
#include "particle.h"
#include <MatrixCommon.h>
#include <FastLED.h>

#define N_PARTICLES 512

struct PollutantState {
  int x, y;
  uint8_t angle;
  uint8_t r, g, b;
  Particle particles[N_PARTICLES];
  void activateParticle(unsigned int lifetime);
  void render(rgb24 *buffer, int w, int h);
  void update(uint8_t *topography, int w, int h, unsigned int level);
  PolledTimer flowTimer;
  PolledInterval flowInterval;
  void startFlow(int amt);
  void stopFlow();
};


extern PollutantState pollutant0;
extern PollutantState pollutant1;
extern PollutantState pollutant2;
extern PollutantState pollutant3;


#define LAKE_LOW 100
#define LAKE_HIGH 1


extern int targetLevel;
extern PolledTimer levelTimer;
