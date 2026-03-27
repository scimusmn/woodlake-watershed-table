#define SMM_IMPLEMENTATION
#include <math.h>
#include <stdlib.h>
#include "pins.h"
#include "matrix.h"
#include "texture.h"
#include "color.h"
#include "topo.h"
#include "comms.h"
#include "diffusion.h"


DiffusionRegion phosphorus, salt, trash, sediment;

#define PHOSPHORUS_X 32
#define PHOSPHORUS_Y 32
#define PHOSPHORUS_VX 0
#define PHOSPHORUS_VY 0
#define PHOSPHORUS_COLOR { 0x00, 0xff, 0x00 }

#define SALT_X 32
#define SALT_Y 32
#define SALT_VX 0
#define SALT_VY 0
#define SALT_COLOR { 0xff, 0xff, 0xff }

#define TRASH_X 32
#define TRASH_Y 32
#define TRASH_VX 0
#define TRASH_VY 0
#define TRASH_COLOR { 0xff, 0x00, 0x00 }

#define SEDIMENT_X 32
#define SEDIMENT_Y 32
#define SEDIMENT_VX 0
#define SEDIMENT_VY 0
#define SEDIMENT_COLOR { 0xff, 0x7f, 0x00 }


texture_t topography = { topography_data, TOPO_WIDTH, TOPO_HEIGHT };


color_t lake_color(uint8_t level, uint8_t depth, uint8_t wave) {
  if (depth > level) {
    color_t shallow, deep;
    shallow.set(0x00, 0x3f, 0xff);
    // deep.set(0x00, 0x00, 0xff);
    deep.set(0x00, 0x3f, 0xff);
    return color_lerp(shallow, deep, depth-level);
  } else {
    return { 0, 0, 0 };
  }
}



void draw_lake(uint8_t level, bool raining, bool storming) {
  static unsigned long t = 0;
  static unsigned int r = 0;
  const int T_STEP = 1;
  const int R_STEP = 64;

  for (int x=0; x<TOPO_WIDTH; x++) {
    for (int y=0; y<TOPO_HEIGHT; y++) {
      int xt = x+t; int yt = y+t;

      int xd = x + sample(displacement, xt, yt);
      int yd = y + sample(displacement, yt, xt);
      int xd_next = x + sample(displacement, xt+T_STEP, yt+T_STEP);
      int yd_next = y + sample(displacement, yt+T_STEP, xt+T_STEP);

      uint8_t wave = sample(waves, xd, yd);
      uint8_t wave_next = sample(waves, xd_next, yd_next);

      uint8_t depth = 0xff - sample(topography, x, y);
      uint8_t depth_next = 0xff - sample(topography, xd_next, yd_next);

      if (depth > level && (storming || raining) && rand() < RAND_MAX>>5) {
        matrix.drawPixel(x, y, 0x00003f);
        continue;
      }


      color_t c = lake_color(level, depth, wave);
      color_t c_next = lake_color(level, depth, wave);
      if (depth > level) {
        c = phosphorus.stack_color(c, xd, yd);
        c = salt.stack_color(c, xd, yd);
        c = trash.stack_color(c, xd, yd);
        c = sediment.stack_color(c, xd, yd);
      }
      if (depth > level) {
        c_next = phosphorus.stack_color(c_next, xd_next, yd_next);
        c_next = salt.stack_color(c_next, xd_next, yd_next);
        c_next = trash.stack_color(c_next, xd_next, yd_next);
        c_next = sediment.stack_color(c_next, xd_next, yd_next);
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

  Serial.println("================================================================");

  Serial.println("setup complete.");
  delay(500);
}



#define WATER_LOW 0.99
#define WATER_MID 0.5
#define WATER_HIGH 0.0

#define WATER_RAMP_FILL 0.02
#define WATER_RAMP_DRAIN 0.05


void rampTowards(float &x, float desired, float step);
void ramp(float &x, float desired, float up, float down);


void loop() {
  static float depth = WATER_LOW;

  // update water depth
  float targetDepth = WATER_LOW;
  if (queryState(MATRIX_STORM)) {
    targetDepth = WATER_HIGH;
  } else if (queryState(MATRIX_RAIN)) {
    targetDepth = WATER_MID;
  }
  ramp(depth, targetDepth, WATER_RAMP_FILL, WATER_RAMP_DRAIN);

  // diffusion regions
  
  phosphorus.update(2);
  if (queryState(MATRIX_PHOSPHORUS)) {
    phosphorus.startDiffusion(PHOSPHORUS_X, PHOSPHORUS_Y, PHOSPHORUS_VX, PHOSPHORUS_VY, PHOSPHORUS_COLOR);
  } else {
    phosphorus.stopDiffusion();
  }

  salt.update(2);
  if (queryState(MATRIX_SALT)) {
    salt.startDiffusion(SALT_X, SALT_Y, SALT_VX, SALT_VY, SALT_COLOR);
  } else {
    salt.stopDiffusion();
  }

  trash.update(2);
  if (queryState(MATRIX_TRASH)) {
    trash.startDiffusion(TRASH_X, TRASH_Y, TRASH_VX, TRASH_VY, TRASH_COLOR);
  } else {
    trash.stopDiffusion();
  }

  sediment.update(2);
  if (queryState(MATRIX_SEDIMENT)) {
    sediment.startDiffusion(SEDIMENT_X, SEDIMENT_Y, SEDIMENT_VX, SEDIMENT_VY, SEDIMENT_COLOR);
  } else {
    sediment.stopDiffusion();
  }

  // draw everything
  draw_lake(0xff*depth, queryState(MATRIX_RAIN), queryState(MATRIX_STORM));
  matrix.show();
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
