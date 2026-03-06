#define SMM_IMPLEMENTATION
#include <math.h>
#include <stdlib.h>
#include "pins.h"
#include "matrix.h"
#include "texture.h"
#include "color.h"

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
  return (rand()>>4) & 0xff;
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
  int fadeStartTime;
  int time;
  int decayTime;
  int active;

  void startDiffusion(int x, int y, int vx, int vy, color_t c) {
    for (int i=0; i<NUM_CONST_CIRCLES; i++) {
      constantCircles[i].x = x;
      constantCircles[i].y = y;
      constantCircles[i].r = 5;
      constantCircles[i].vx = 0;
      constantCircles[i].vy = 0;
      constantCircles[i].vr = 0;
      constantCircles[i].active = 1;
    }
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
    fadeStartTime = 0;
  }

  void stopDiffusion() {
    if (!fadeStartTime) {
      fadeStartTime = time;
    }
  }

  int age_sample(int x, int y) {
    int X = clamp(x, 0, MAT_WIDTH-1);
    int Y = clamp(y, 0, MAT_HEIGHT-1);
    return ages[X + MAT_WIDTH*Y];
  }


#define SATURATION 300
  void update(int period) {
    time += 8;
    // for (int i=0; i<NUM_DIFF_CIRCLES; i++) {
    //   if (!constantCircles[i].update(ages, MAT_WIDTH, MAT_HEIGHT, time, period)) {
    //     constantCircles[i].active = 1;
    //     constantCircles[i].x = MAT_WIDTH>>1;
    //     constantCircles[i].y = MAT_HEIGHT>>1;
    //   }
    // }

    if (active) {
      if (!fadeStartTime || time < SATURATION + fadeStartTime) {
        int more = 0;
        for (int i=0; i<NUM_DIFF_CIRCLES; i++) {
          more = more || circles[i].update(ages, MAT_WIDTH, MAT_HEIGHT, time, period);
        }
        if (!more && !fadeStartTime) {
          fadeStartTime = time;
          active = 0;
        }
      }
    }
  }

  color_t stack_color(color_t bg, int x, int y) {
    if (!active) {
      return bg;
    }

    int pixelAge = age_sample(x, y);

    if (pixelAge == 0) {
      return bg;
    }

    int s;

    if (fadeStartTime) {
      int saturationTime = SATURATION + pixelAge;
      if (fadeStartTime < saturationTime) {
        if (time < saturationTime) {
          s = time - pixelAge;
        } else {
          s = 2*SATURATION + pixelAge - time;
        }
      } else { // already saturated
        s = SATURATION + fadeStartTime - time;
      }
    } else {
      // still growing
      s = clamp(time - pixelAge, 0, SATURATION);
    }

    return color_lerp(bg, color, clamp(s, 0, 0xff));
  }
};


DiffusionRegion pollutant0, pollutant1;


uint8_t topography_data[MAT_WIDTH * MAT_HEIGHT];
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
    deep.set(0x00, 0x00, 0xff);
    return deep;

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

      int xd = x + sample(displacement, xt, yt);
      int yd = y + sample(displacement, yt, xt);
      int xd_next = x + sample(displacement, xt+T_STEP, yt+T_STEP);
      int yd_next = y + sample(displacement, yt+T_STEP, xt+T_STEP);

      uint8_t wave = sample(waves, xd, yd);
      uint8_t wave_next = sample(waves, xd_next, yd_next);

      uint8_t depth = sample(topography, xd, yd);
      uint8_t depth_next = sample(topography, xd_next, yd_next);

      color_t c = lake_color(level, depth, wave);
      color_t c_next = lake_color(level, depth_next, wave_next);
      if (depth > 4) {
        // c = color_lerp(c, pollutant.c, clamp(pollutant.time >> 4, 0, 0xf));
        c = pollutant0.stack_color(c, xd, yd);
        c = pollutant1.stack_color(c, xd, yd);
      }
      if (depth_next > 4) {
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

  ProtomatterStatus status = matrix.begin();
  Serial.print("  matrix status: ");
  Serial.println((int)status);
  if (status != PROTOMATTER_OK) {
    Serial.println("BAD MATRIX STATUS; halting!");
    for (;;);
  }
  Serial.println("  matrix initialization: OK");

  setupTopography();
  Serial.println("  topography initialization: OK");

  pollutant0.startDiffusion(32, 32, 32, 0, { 0xff, 0, 0 });
  pollutant1.startDiffusion(64+32, 32, -32, 0, { 0xff, 0xff, 0 });
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");
  Serial.println("================================================================");

  Serial.println("setup complete.");
  delay(500);
}


void loop() {
  static unsigned long t = 0;
  pollutant0.update(2);
  pollutant1.update(2);
  draw_lake(4);
  matrix.show();
  if (millis() > t) {
    Serial.println(matrix.getFrameCount());
    t = millis() + 1000;
  }
  if (millis() > 8000) {
    pollutant0.stopDiffusion();
    pollutant1.stopDiffusion();
  }

  if (!pollutant1.active) {
    pollutant1.startDiffusion(64+32, 32, -32, 0, { 0xff, 0xff, 0 });
  }
}
