#define SMM_IMPLEMENTATION
#include <math.h>
#include <stdlib.h>
#include "pins.h"
#include "matrix.h"
#include "texture.h"
#include "color.h"
#include "topo.h"
#include "comms.h"


#define MAT_WIDTH 128
#define MAT_HEIGHT 64


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
  // return (rand()>>4) & 0xff;
}


struct DiffusionCircle {
  int x, y, r, vx, vy, vr;
  int active = 1;
  int cycle = 0;

  int biased_step(int v) {
    int delta = v + rand8();
    if (delta < 85) {
      return -1;
    } else if (delta > 170) {
      return 1;
    } else {
      return 0;
    }
  }

  int update(int *ages, int w, int h, int time, int period) {
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
};


#define NUM_DIFF_CIRCLES 8
#define NUM_CONST_CIRCLES 8
struct DiffusionRegion {
  int ages[MAT_WIDTH * MAT_HEIGHT];
  color_t color;
  DiffusionCircle circles[NUM_DIFF_CIRCLES];
  DiffusionCircle constantCircles[NUM_CONST_CIRCLES];
  unsigned long epoch;
  int time, stopTime, finishTime;
  float riseRate = 0.1;
  float fallRate = 0.1;
  float globalMax = 0.4;
  int active = 0;

  void startDiffusion(int x, int y, int vx, int vy, color_t c) {
    if (active) {
      return;
    }
    Serial.println("pollutant reset!");

    epoch = millis();

    memset(ages, 0, sizeof(int) * MAT_WIDTH * MAT_HEIGHT);
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

  void stopDiffusion() {
    if (stopTime > 0) {
      return;
    }
    stopTime = time;
    finishTime = 0xff*((1.0/riseRate) + (1.0/fallRate)) + time;
  }

  int age_sample(int x, int y) {
    int X = clamp(x, 0, MAT_WIDTH-1);
    int Y = clamp(y, 0, MAT_HEIGHT-1);
    return ages[X + MAT_WIDTH*Y];
  }


#define SATURATION 300
  void update(int period) {
    time = millis() - epoch;

    if (active) {
      active = stopTime < 0 || time < finishTime;
      if (stopTime < 0 || time < stopTime) {
        for (int i=0; i<NUM_DIFF_CIRCLES; i++) {
          circles[i].update(ages, MAT_WIDTH, MAT_HEIGHT, time, period);
        }
      }
    }
  }

  uint8_t pixelIntensity(int x, int y, int t) {
    const int maxIntensity = 0xff;

    int t0 = age_sample(x, y);
    if (t0 == 0) {
      if (stopTime > 0) {
        int centerTime = (finishTime + stopTime) >> 1;
        float s = ((float)maxIntensity) / (centerTime - stopTime);
        int i = maxIntensity - s*abs(t-centerTime);
        return clamp(globalMax * i, 0, maxIntensity);
      } else {
        return 0;
      }
    }

    int tMax = t+1;
    if (stopTime > 0) {
      tMax = max(
        (0xff + riseRate * t0)/riseRate, 
        stopTime
      );
    }

    if (t < tMax) {
      return clamp(riseRate * (t-t0), 0, maxIntensity);
    } else {
      return clamp(maxIntensity - fallRate*(t-tMax), 0, maxIntensity);
    }
  }


  color_t stack_color(color_t bg, int x, int y) {
    if (!active) {
      return bg;
    }

    uint8_t intensity = pixelIntensity(x, y, time);
    return color_lerp(bg, color, intensity);
  }
};


DiffusionRegion pollutant0, pollutant1;


texture_t topography = { topography_data, MAT_WIDTH, MAT_HEIGHT };

void setupTopography() {
  memset(topography_data, 0, sizeof(topography_data));
  float X[] = { 32, 40, 60, 92, 66 };
  float Y[] = { 32, 32, 32, 46, 32 };
  float A[] = { 100, 100, 150, 100, 150 };
  float B[] = { 100, 50, 500, 30, 500 };
  for (int x=0; x<MAT_WIDTH; x++) {
    for (int y=0; y<MAT_HEIGHT; y++) {
      for (int i=0; i<5; i++) {
        float dx = x - X[i];
        float dy = y - Y[i];
        int idx = MAT_WIDTH*y + x;
        uint8_t topo = topography_data[idx];
        topography_data[idx] += A[i]*exp(-(dx*dx + dy*dy)/B[i]);
        if (topography_data[idx] < topo) {
          topography_data[idx] = 255;
        }
      }
    }
  }
}



color_t lake_color(uint8_t level, uint8_t depth, uint8_t wave) {
  if (depth > level) {
    color_t shallow, deep;
    shallow.set(0x00, 0x3f, 0xff);
    deep.set(0x00, 0x00, 0xff);
    // shallow.set(0xff, 0xff, 0xff);
    // deep.set(0xff, 0xff, 0xff);
    // return deep;
    return color_lerp(shallow, deep, depth-level);

    // if (wave) {
    //   shallow.set(0x0f, 0x1f, 0x7f);
    //   deep.set(0x0f, 0x1f, 0x7f);
    // } else {
    //   shallow.set(0, 0, 0x7f);
    //   deep.set(0, 0, 0xff);
    // }
    // return color_lerp(shallow, deep, depth);
  } else {
    return { 0, 0, 0 };
  }
}



void draw_lake(uint8_t level) {
  static unsigned long t = 0;
  static unsigned int r = 0;
  const int T_STEP = 1;
  const int R_STEP = 64;

  for (int x=0; x<MAT_WIDTH; x++) {
    for (int y=0; y<MAT_HEIGHT; y++) {
      int xt = x+t; int yt = y+t;

      // int xd = x + sample(displacement, xt, yt);
      // int yd = y + sample(displacement, yt, xt);
      // int xd_next = x + sample(displacement, xt+T_STEP, yt+T_STEP);
      // int yd_next = y + sample(displacement, yt+T_STEP, xt+T_STEP);
      int xd = x; int yd = y;
      int xd_next = x;
      int yd_next = y;

      uint8_t wave = sample(waves, xd, yd);
      uint8_t wave_next = sample(waves, xd_next, yd_next);

      uint8_t depth = 0xff - sample(topography, xd, yd);
      uint8_t depth_next = 0xff - sample(topography, xd_next, yd_next);

      color_t c = lake_color(level, depth, wave);
      color_t c_next = lake_color(level, depth_next, wave_next);
      if (depth > level) {
        // c = color_lerp(c, pollutant.c, clamp(pollutant.time >> 4, 0, 0xf));
        c = pollutant0.stack_color(c, xd, yd);
        c = pollutant1.stack_color(c, xd, yd);
      }
      if (depth_next > level) {
        c_next = pollutant0.stack_color(c_next, xd_next, yd_next);
        c_next = pollutant1.stack_color(c_next, xd_next, yd_next);
      }

      matrix.drawPixel(x, y, color_lerp(c, c_next, r).value());

    }
  }
  r = adds(r, R_STEP);
  if (r == 0xff) {
    t += T_STEP;
    r = 0;
  }
}


// ================================

void setup() {
  Serial.begin(115200);
  Serial.println("== boot ==");

  setupComms();

  ProtomatterStatus status = matrix.begin();
  Serial.print("  matrix status: ");
  Serial.println((int)status);
  if (status != PROTOMATTER_OK) {
    Serial.println("BAD MATRIX STATUS; halting!");
    for (;;);
  }
  Serial.println("  matrix initialization: OK");

  // setupTopography();
  Serial.println("  topography initialization: OK");

  pollutant0.startDiffusion(32,    32,   0, 0, { 0xff, 0, 0 });
  // pollutant1.startDiffusion(64+32, 32,   0, 0, { 0xff, 0xff, 0 });
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");

  Serial.println("setup complete.");
  delay(500);
}



#define WATER_LOW 0.99
#define WATER_MID 0.5
#define WATER_HIGH 0.0
#define WATER_RAMP_UP 0.1
#define WATER_RAMP_DOWN 0.01


void rampTowards(float &x, float desired, float step);
void ramp(float &x, float desired, float up, float down);


void loop() {
  static float depth = WATER_LOW;

  pollutant0.update(2);
  pollutant1.update(2);

  // update water depth
  float targetDepth = WATER_LOW;
  if (queryState(MATRIX_STORM)) {
    targetDepth = WATER_HIGH;
  } else if (queryState(MATRIX_RAIN)) {
    targetDepth = WATER_MID;
  }
  ramp(depth, targetDepth, WATER_RAMP_UP, WATER_RAMP_DOWN);

  if (queryState(MATRIX_RAIN)) {
    pollutant0.startDiffusion(32, 32,   0, 0, { 0xff, 0, 0 });
  } else {
    pollutant0.stopDiffusion();
  }

  draw_lake(0xff*depth);
  matrix.show();

  if (millis() > 8000) {
    pollutant0.stopDiffusion();
  }
// 
//   if (!pollutant1.active) {
//     pollutant1.startDiffusion(64+32, 32, -32, 0, { 0xff, 0xff, 0 });
//   }
}


void ramp(float &x, float desired, float up, float down) {
  if (x > desired) {
    rampTowards(x, desired, down);
  } else {
    rampTowards(x, desired, up);
  }
}



void rampTowards(float &x, float desired, float step) {
  if (fabs(x - desired) < step) {
    x = desired;
  } else {
    if (x < desired) {
      x += step;
    } else {
      x -= step;
    }
  }
}
