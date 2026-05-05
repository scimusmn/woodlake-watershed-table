#ifndef FLUID_H
#define FLUID_H

#include <stdbool.h>


typedef struct {
  float *x, *x_buf, *y, *y_buf;
  bool *mask;
  int width, height;
} fluid_t;


fluid_t * fluid_create(int width, int height);
void fluid_destroy(fluid_t *f);
void fluid_update(fluid_t *f, int rounds);


#endif
