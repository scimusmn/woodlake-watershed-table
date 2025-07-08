#define SMM_IMPLEMENTATION
#include <MatrixHardware_Teensy4_ShieldV5.h>
#include <SmartMatrix.h>
#include <FastLED.h>
#include <math.h>
#include <stdlib.h>
#include "pins.h"
#include "particle.h"
#include "messages.h"
#include "state.h"
#include "PolledTimer.h"

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
  float Y[] = { 32, 32, 32, 46, 32 };
  float A[] = { 100, 100, 150, 100, 150 };
  float B[] = { 100, 50, 500, 30, 500 };
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


PollutantState pollutant0;
PollutantState pollutant1;
PollutantState pollutant2;
PollutantState pollutant3;


int targetLevel = LAKE_LOW;
PolledTimer levelTimer;



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


// ================================

void setup() {
  Serial.begin(115200);
  Serial.println("== boot ==");
  matrix.addLayer(&bg);
  matrix.begin();
  setupTopography();

  setupCan(0x00);

  pollutant0.x = 32; pollutant0.y = 40;
  pollutant0.angle = -32;
  pollutant0.r = 0xff; pollutant0.g = 0xff; pollutant0.b = 0xff;
  
  pollutant1.x = 72; pollutant1.y = 32;
  pollutant1.angle = 128;
  pollutant1.r = 0x00; pollutant1.g = 0xff; pollutant1.b = 0;
  
  pollutant2.x = 70; pollutant2.y = 40;
  pollutant2.angle = -96;
  pollutant2.r = 0xff; pollutant2.g = 0x00; pollutant2.b = 0xff;
  
  pollutant3.x = 70; pollutant3.y = 30;
  pollutant3.angle = 96;
  pollutant3.r = 0xff; pollutant3.g = 0xff; pollutant3.b = 0x00;

  Serial.println("setup complete.");
  delay(500);
}


void loop() {
  static unsigned int t = 0;
  static int level = targetLevel;

  levelTimer.update();
  // uint8_t level = 11;
  while (bg.isSwapPending());

  rgb24 *buffer = bg.backBuffer();
  memset(buffer, 0, MAT_WIDTH * MAT_HEIGHT * sizeof(rgb24));

  // update particles
  if ((t & 7) == 0) {
    Serial.print(level); Serial.print(" "); Serial.println(targetLevel);
    if (level < targetLevel) {
      level += 1;
    } else if (level > targetLevel) {
      level -= 1;
    }
    pollutant0.update(topography, MAT_WIDTH, MAT_HEIGHT, level);
    pollutant1.update(topography, MAT_WIDTH, MAT_HEIGHT, level);
    pollutant2.update(topography, MAT_WIDTH, MAT_HEIGHT, level);
    pollutant3.update(topography, MAT_WIDTH, MAT_HEIGHT, level);
  }

  // draw lake + particles
  drawLake(buffer, t, level);
  pollutant0.render(buffer, MAT_WIDTH, MAT_HEIGHT);
  pollutant1.render(buffer, MAT_WIDTH, MAT_HEIGHT);
  pollutant2.render(buffer, MAT_WIDTH, MAT_HEIGHT);
  pollutant3.render(buffer, MAT_WIDTH, MAT_HEIGHT);

  bg.swapBuffers(false);
  matrix.countFPS();
  t += 1;
}
