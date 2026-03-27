#ifndef COMMS_H
#define COMMS_H

#include <stdint.h>


#define MATRIX_ADDR 0x70


typedef enum {
  MATRIX_RAIN,
  MATRIX_STORM,
  MATRIX_PHOSPHORUS,
  MATRIX_SALT,
  MATRIX_TRASH,
  MATRIX_SEDIMENT,
  NUM_MATRIX_VARS
} matrix_var_t;


void setupComms();
bool queryState(matrix_var_t x);


#endif
