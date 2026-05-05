#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fluid.h"

#define IDX(w, x, y) (w*y + x)


float lerp(float a, float b, float t) {
  return a*(1-t) + b*t;
}


float grid_value(float *grid, bool *mask, int width, int height, int x, int y) {
  if (x < 0 || x >= width) {
    return 0.0f;
  }

  if (y < 0 || y >= height) {
    return 0.0f;
  }

  int idx = IDX(width, x, y);
  if (mask[idx]) {
    return 0.0f;
  }
  return grid[idx];
}


float grid_interpolate(float *grid, bool *mask, int width, int height, float x, float y) {
  float corners[4];
  corners[0] = grid_value(grid, mask, width, height, x, y);
  corners[1] = grid_value(grid, mask, width, height, x+1, y);
  corners[2] = grid_value(grid, mask, width, height, x, y+1);
  corners[3] = grid_value(grid, mask, width, height, x+1, y+1);

  float lo = lerp(corners[0], corners[1], x-((int)x));
  float hi = lerp(corners[2], corners[3], x-((int)x));
  return lerp(lo, hi, y-((int)y));
}


void advect(float *dst_x, float *dst_y, float *src_x, float *src_y, bool *mask, int width, int height, float scale) {
  for (int x=0; x<width; x++) {
    for (int y=0; y<height; y++) {
      float dx = grid_value(src_x, mask, width, height, x, y);
      float dy = grid_value(src_y, mask, width, height, x, y);
      float r = sqrt(x*x + y*y);
      if (r == 0) {
        r = 1;
      }

      float xf = ((float)x) - (dx/r);
      float yf = ((float)y) - (dy/r);
      float ax = grid_interpolate(src_x, mask, width, height, xf, yf);
      float ay = grid_interpolate(src_y, mask, width, height, xf, yf);
      unsigned int index = IDX(width, x, y);
      dst_x[index] = lerp(dx, ax, scale);
      dst_y[index] = lerp(dy, ay, scale);
    }
  }
}


#define X(dx, dy) grid_value(grid_x, mask, width, height, x+(dx), y+(dy))
#define Y(dx, dy) grid_value(grid_y, mask, width, height, x+(dx), y+(dy))

float div_grad_x(float *grid_x, float *grid_y, bool *mask, int width, int height, int x, int y) {
  float f0 = X(1, -1) - Y(1, -1);
  float f1 = -2*X(0, -1);
  float f2 = X(-1, -1) + Y(-1, -1);

  float f3 = 2*X(1, 0);
  float f4 = -4*X(0, 0);
  float f5 = 2*X(-1, 0);

  float f6 = X(1, 1) + Y(1, 1);
  float f7 = -2*X(0, 1);
  float f8 = X(-1, 1) - Y(-1, 1);

  return f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
}


float div_grad_y(float *grid_x, float *grid_y, bool *mask, int width, int height, int x, int y) {
  float f0 = -X(1, -1) + Y(1, -1);
  float f1 = 2*Y(0, -1);
  float f2 = X(-1, -1) + Y(-1, -1);
  
  float f3 = -2*Y(1, 0);
  float f4 = -4*Y(0, 0);
  float f5 = -2*Y(-1, 0);

  float f6 = X(1, 1) + Y(1, 1);
  float f7 = 2*Y(0, 1);
  float f8 = -X(-1, 1) + Y(-1, 1);

  return f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
}
#undef X
#undef Y

void strip_div(float *dst_x, float *dst_y, float *src_x, float *src_y, bool *mask, int width, int height, float scale) {
  for (int x=0; x<width; x++) {
    for (int y=0; y<height; y++) {
      int index = IDX(width, x, y);
      float gx = div_grad_x(src_x, src_y, mask, width, height, x, y);
      float gy = div_grad_y(src_x, src_y, mask, width, height, x, y);
      dst_x[index] = src_x[index] + scale*gx;
      dst_y[index] = src_y[index] + scale*gy;
    }
  }
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


fluid_t * fluid_create(int width, int height) {
  fluid_t *f = malloc(sizeof(fluid_t));
  if (f == NULL) {
    return NULL;
  }

  f->width = width;
  f->height = height;

  size_t buf_size = width * height;

  f->x = calloc(buf_size, sizeof(float));
  if (f->x == NULL) {
    free(f);
    return NULL;
  }

  f->y = calloc(buf_size, sizeof(float));
  if (f->y == NULL) {
    free(f->x);
    free(f);
    return NULL;
  }

  f->x_buf = calloc(buf_size, sizeof(float));
  if (f->x_buf == NULL) {
    free(f->y);
    free(f->x);
    free(f);
    return NULL;
  }

  f->y_buf = calloc(buf_size, sizeof(float));
  if (f->y_buf == NULL) {
    free(f->x_buf);
    free(f->y);
    free(f->x);
    free(f);
    return NULL;
  }

  f->mask = calloc(buf_size, sizeof(bool));
  if (f->y_buf == NULL) {
    free(f->y_buf);
    free(f->x_buf);
    free(f->y);
    free(f->x);
    free(f);
    return NULL;
  }

  return f;
}


void fluid_destroy(fluid_t *f) {
  free(f->x);
  free(f->y);
  free(f->x_buf);
  free(f->y_buf);
  free(f->mask);
  free(f);
}


void fluid_swap(fluid_t *f) {
  float *tmp = f->x_buf;
  f->x_buf = f->x;
  f->x = tmp;

  tmp = f->y_buf;
  f->y_buf = f->y;
  f->y = tmp;
}


void fluid_update(fluid_t *f, int rounds) {
  advect(f->x_buf, f->y_buf, f->x, f->y, f->mask, f->width, f->height, 0.8);  
  fluid_swap(f);
  for (int i=0; i<rounds; i++) {
    strip_div(f->x_buf, f->y_buf, f->x, f->y, f->mask, f->width, f->height, 0.125);
    fluid_swap(f);
  }
}
