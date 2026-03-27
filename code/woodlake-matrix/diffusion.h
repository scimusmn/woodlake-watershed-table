#ifndef DIFFUSION_H
#define DIFFUSION_H

#include <stdint.h>
#include "color.h"
#include "topo.h"


int clamp(int x, int lo, int hi);
uint8_t adds(uint8_t a, uint8_t b);
uint8_t subs(uint8_t a, uint8_t b);
int rand8();


struct DiffusionCircle {
  int x, y, r, vx, vy, vr;
  int active = 1;
  int cycle = 0;

  int biased_step(int v);
  int update(short *ages, int w, int h, int time, int period);
};



#define NUM_DIFF_CIRCLES 8
struct DiffusionRegion {
  short ages[TOPO_WIDTH * TOPO_HEIGHT];
  color_t color;
  DiffusionCircle circles[NUM_DIFF_CIRCLES];
  unsigned long epoch;
  int time, stopTime, finishTime;
  float riseRate = 0.1;
  float fallRate = 0.1;
  float globalMax = 0.4;
  int active = 0;

  void startDiffusion(int x, int y, int vx, int vy, color_t c);
  void stopDiffusion();
  void update(int period);
  int age_sample(int x, int y);
  uint8_t pixelIntensity(int x, int y, int t);
  color_t stack_color(color_t bg, int x, int y);
};



#endif
