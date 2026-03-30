#ifndef COMMS_H
#define COMMS_H

#include <stdint.h>


#define MATRIX_ADDR 0x70


#define MATRIX_RAIN       0
#define MATRIX_STORM      1
#define MATRIX_PHOSPHORUS 2
#define MATRIX_SALT       3
#define MATRIX_TRASH      4
#define MATRIX_SEDIMENT   5


void matrixActivate(uint8_t x);
void matrixDeactivate(uint8_t x);
void matrixSet(uint8_t x, bool b);


#endif
