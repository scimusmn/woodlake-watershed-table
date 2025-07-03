#include <MatrixHardware_Teensy4_ShieldV5.h>
#include <SmartMatrix.h>
#include <FastLED.h>
#include <math.h>
#include <stdlib.h>
#include "pins.h"
#include "particle.h"
#include "messages.h"

#define LED_STRIP_LEN 600
CRGB strip[LED_STRIP_LEN];


#define COLOR_DEPTH 24
#define MAT_WIDTH 128 
#define MAT_HEIGHT 64
#define REFRESH_DEPTH 36
#define DMA_BUF_ROWS 4
#define PANEL_TYPE SM_PANELTYPE_HUB75_64ROW_MOD32SCAN
#define MAT_OPTIONS (SM_HUB75_OPTIONS_NONE)
#define BG_OPTIONS (SM_BACKGROUND_OPTIONS_NONE)


SMARTMATRIX_ALLOCATE_BUFFERS(
  matrix, MAT_WIDTH, MAT_HEIGHT, 
  REFRESH_DEPTH, DMA_BUF_ROWS, PANEL_TYPE, MAT_OPTIONS
);

SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(bg, MAT_WIDTH, MAT_HEIGHT, COLOR_DEPTH, BG_OPTIONS);


uint8_t topography[MAT_WIDTH * MAT_HEIGHT];
void setupTopography() {
  memset(topography, 0, sizeof(topography));
  float X[] = { 32, 40, 60, 92, 66 };
  float Y[] = { 12, 18, 27, 46, 32 };
  float A[] = { 100, 50, 150, 30, 150 };
  float B[] = { 100, 50, 200, 30, 500 };
  for (int x=0; x<MAT_WIDTH; x++) {
    for (int y=0; y<MAT_HEIGHT; y++) {
      for (int i=0; i<5; i++) {
        float dx = x - X[i];
        float dy = y - Y[i];
        int idx = MAT_WIDTH*y + x;
        uint8_t topo = topography[idx];
        topography[idx] += A[i]*exp(-(dx*dx + dy*dy)/B[i]);
        if (topography[idx] < topo) {
          topography[idx] = 255;
        }
      }
    }
  }
}



void drawLake(rgb24 *buffer, unsigned int t, uint8_t level) {
  for (int x=0; x<MAT_WIDTH; x++) {
    for (int y=0; y<MAT_HEIGHT; y++) {
      int idx = y*MAT_WIDTH + x;
      if (topography[idx] >= level) {
        buffer[idx] = (rgb24){ 0, (sin8(x<<3)>>2) + (sin8(y<<3)>>2), 0xff };
      }
    }
  }
}


#define N_PARTICLES 512
Particle orangeParticles[N_PARTICLES];
Particle yellowParticles[N_PARTICLES];
Particle redParticles[N_PARTICLES];


void activateParticle(Particle *particles, unsigned int lifetime, int x, int y, float v, uint8_t angle) {
  for (int i=0; i<N_PARTICLES; i++) {
    if (!particles[i].isActive()) {
      particles[i].activate(lifetime, x, y, v*cos8(angle), v*sin8(angle));
      return;
    }
  }
}

// draw individual particle
void drawParticle(rgb24 *buffer, Particle *p, rgb24 color) {
  if (p->isActive()) {
    buffer[MAT_WIDTH*p->getY() + p->getX()] = color;
  }
}

// draw all particles
void drawParticles(rgb24 *buffer) {
  for (int i=0; i<N_PARTICLES; i++) {
    drawParticle(buffer, orangeParticles+i, (rgb24){ 0xff, 0x7f, 0x00 });
    drawParticle(buffer, yellowParticles+i, (rgb24){ 0xff, 0xff, 0x00 });
    drawParticle(buffer, redParticles+i,    (rgb24){ 0xff, 0x00, 0x00 });
  }
}

// update all particles
void updateParticles(unsigned int level) {
  for (int i=0; i<N_PARTICLES; i++) {
    orangeParticles[i].update(topography, MAT_WIDTH, MAT_HEIGHT, level);
    yellowParticles[i].update(topography, MAT_WIDTH, MAT_HEIGHT, level);
    redParticles[i].update(topography, MAT_WIDTH, MAT_HEIGHT, level);
  }
}


// ================================

void setup() {
  Serial.begin(115200);
  Serial.println("== boot ==");
  matrix.addLayer(&bg);
  matrix.begin();
  setupTopography();

  FastLED.addLeds<WS2811, LED_STRIP_DATA_PIN, GRB>(strip, LED_STRIP_LEN);

  setupCan(0x00);

  Serial.println("setup complete.");
  delay(500);
}


void loop() {
  static unsigned int t = 0;

  uint8_t level = sin8(t>>3);
  // uint8_t level = 11;
  while (bg.isSwapPending());

  rgb24 *buffer = bg.backBuffer();
  memset(buffer, 0, MAT_WIDTH * MAT_HEIGHT * sizeof(rgb24));

  // update particles
  if ((t & 7) == 0) {
    activateParticle(orangeParticles, N_PARTICLES, 44, 32, 1.0, -32);
    activateParticle(yellowParticles, N_PARTICLES, 100, 32, 1.0, -32);
    activateParticle(redParticles, N_PARTICLES, 64, 50, 1.0, 32);
    updateParticles(level);
  }

  // draw lake + particles
  drawLake(buffer, t, level);
  drawParticles(buffer);

  bg.swapBuffers(false);
  matrix.countFPS();
  t += 1;
}
