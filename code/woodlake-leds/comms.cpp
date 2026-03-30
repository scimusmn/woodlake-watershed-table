#include <Wire.h>
#include "comms.h"


void matrixActivate(uint8_t x) {
  uint8_t msg = 0x80 | x;
  Wire.beginTransmission(MATRIX_ADDR);
  Wire.write(msg);
  Wire.endTransmission();
}


void matrixDeactivate(uint8_t x) {
  uint8_t msg = 0x7f & x;
  Wire.beginTransmission(MATRIX_ADDR);
  Wire.write(msg);
  Wire.endTransmission();
}


void matrixSet(uint8_t x, bool b) {
  if (b) {
    matrixActivate(x);
  } else {
    matrixDeactivate(x);
  }
}
