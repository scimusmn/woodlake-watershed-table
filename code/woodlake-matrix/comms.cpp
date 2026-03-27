#include <Wire.h>
#include "comms.h"


bool states[NUM_MATRIX_VARS];


void i2cReceive(int numBytes) {
  if (numBytes != 1) {
    return;
  }

  uint8_t x = Wire.read();
  bool activate = x & 0x80;
  uint8_t id = x & 0x7f;
  if (id >= NUM_MATRIX_VARS) {
    return;
  }
  states[id] = activate;
}


void setupComms() {
  Wire.begin(MATRIX_ADDR);
  Wire.onReceive(i2cReceive);
  memset(states, 0, sizeof(states));
}


bool queryState(matrix_var_t x) {
  return states[x];
}
