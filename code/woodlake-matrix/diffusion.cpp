#include <arduino.h>
#include <stdlib.h>
#include "diffusion.h"
#include "topo.h"


int clamp(int x, int lo, int hi) {
  if (x < lo) {
    return lo;
  } else if (x > hi) {
    return hi;
  } else {
    return x;
  }
}


uint8_t adds(uint8_t a, uint8_t b) {
  int value = (int)a + (int)b;
  if (value > 0xff) {
    return 0xff;
  } else {
    return value;
  }
}


uint8_t subs(uint8_t a, uint8_t b) {
  int value = (int)a - (int)b;
  if (value < 0) {
    return 0;
  } else {
    return value;
  }
}


int rand8() {
  return rand() % 0x100;
}



/********************************\
 *                              *
 *     DiffusionCircle          *
 *                              *
\********************************/


int DiffusionCircle::biased_step(int v) {
  int delta = v + rand8();
  if (delta < 85) {
    return -1;
  } else if (delta > 170) {
    return 1;
  } else {
    return 0;
  }
}

int DiffusionCircle::update(short *ages, int w, int h, int time, int period) {
  if (!active) {
    return 0;
  }

  cycle += 1;
  if (cycle < period) {
    return 1;
  } else {
    cycle = 0;
  }

  x += biased_step(vx);
  y += biased_step(vy);
  r += biased_step(vr);

  if (x < 0 || x > w) {
    active = false;
    return 0;
  }
  if (y < 0 || y > h) {
    active = false;
    return 0;
  }


  if (r == 0) {
    r = 1;
  }

  for (int dx=-r; dx<=r; dx++) {
    for (int dy=-r; dy<=r; dy++) {
      int r2 = dx*dx + dy*dy;
      if (r2 < r*r) {
        int X = x+dx;
        int Y = y+dy;
        if (X >= 0 && X < w && Y >= 0 && Y < h) {
          int idx = (x+dx) + w*(y+dy);
          if (!ages[idx]) {
            ages[idx] = time;
          }
        }
      }
    }
  }

  return 1;
}




/********************************\
 *                              *
 *     DiffusionRegion          *
 *                              *
\********************************/


// start diffusing. this function is idempotent until stopDiffusion is called
void DiffusionRegion::startDiffusion(int x, int y, int vx, int vy, color_t c) {
  // idempotency c:
  if (active) {
    return;
  }

  epoch = millis();

  memset(ages, 0, sizeof(ages));
  color = c;
  for (int i=0; i<NUM_DIFF_CIRCLES; i++) {
    circles[i].x = x;
    circles[i].y = y;
    circles[i].r = 1;
    circles[i].vx = vx;
    circles[i].vy = vy;
    circles[i].vr = 64;
    circles[i].active = 1;
  }
  time = 1;
  active = 1;
  stopTime = -1;
  finishTime = -1;
}


// stop diffusing. this function is idempotent until startDiffusion is called
void DiffusionRegion::stopDiffusion() {
  // idempotency c:
  if (stopTime > 0) {
    return;
  }
  stopTime = time;
  finishTime = 0xff*((1.0/riseRate) + (1.0/fallRate)) + time;
}


// get the age of a pixel
int DiffusionRegion::age_sample(int x, int y) {
  int X = clamp(x, 0, (TOPO_WIDTH)-1);
  int Y = clamp(y, 0, (TOPO_HEIGHT)-1);
  return ages[X + (TOPO_WIDTH)*Y];
}


// update the diffusion region
void DiffusionRegion::update(int period) {
  time = millis() - epoch;

  if (active) {
    active = stopTime < 0 || time < finishTime;
    if (stopTime < 0 || time < stopTime) {
      for (int i=0; i<NUM_DIFF_CIRCLES; i++) {
        circles[i].update(ages, TOPO_WIDTH, TOPO_HEIGHT, time, period);
      }
    }
  }
}


// get the intensity of color at a particular pixel and time
uint8_t DiffusionRegion::pixelIntensity(int x, int y, int t) {
  const int maxIntensity = 0xff;

  int t0 = age_sample(x, y);
  if (t0 == 0) {
    // pixel has not been touched by a circle
    if (stopTime > 0) {
      // main diffusion is fading out; fade in the whole region
      int centerTime = (finishTime + stopTime) >> 1;
      float s = ((float)maxIntensity) / (centerTime - stopTime);
      int i = maxIntensity - s*abs(t-centerTime);
      return clamp(globalMax * i, 0, maxIntensity);
    } else {
      // no substance here
      return 0;
    }
  }

  // compute time when pixel will be at maximum intensity
  int tMax = t+1;
  if (stopTime > 0) {
    tMax = max(
      (0xff + riseRate * t0)/riseRate, 
      stopTime
    );
  }

  // get intensity
  if (t < tMax) {
    return clamp(riseRate * (t-t0), 0, maxIntensity);
  } else {
    return clamp(maxIntensity - fallRate*(t-tMax), 0, maxIntensity);
  }
}


// stack pixel color on background color
color_t DiffusionRegion::stack_color(color_t bg, int x, int y) {
  if (!active) {
    return bg;
  }

  uint8_t intensity = pixelIntensity(x, y, time);
  return color_lerp(bg, color, intensity);
}
