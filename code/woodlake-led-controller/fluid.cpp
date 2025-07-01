#include "fluid.h"
#include <arduino.h>


float dot(vec2d a, vec2d b) {
  return (a.x*b.x) + (a.y*b.y);
}


vec2d add(vec2d a, vec2d b) {
  return (vec2d){ a.x+b.x, a.y+b.y };
}

vec2d sub(vec2d a, vec2d b) {
  return (vec2d){ a.x-b.x, a.y-b.y };
}

vec2d scale(vec2d v, float a) {
  return (vec2d){ a*v.x, a*v.y };
}

vec2d mult(vec2d a, vec2d b) {
  return (vec2d){ a.x*b.x, a.y*b.y };
}


vec2d vec(vec2d *src, unsigned int width, unsigned int height, int x, int y) {
  if (x < 0 || x >= width) {
    return (vec2d){ 0, 0 };
  } else if (y < 0 || y >= height) {
    return (vec2d){ 0, 0 };
  } else {
    return src[y*width + x];
  }
}


vec2d divergenceGradient(vec2d *src, unsigned int w, unsigned int h, int x, int y) {
  vec2d v[9];
  int idx = 0;
  for (int dy=-1; dy<2; dy++) {
    for (int dx=-1; dx<2; dx++) {
      v[idx] = vec(src, w, h, x+dx, y+dy);
      idx += 1;
    }
  }

  #define X(i) v[i].x
  #define Y(i) v[i].y

  vec2d result;
  result.x = 
    X(0)-Y(0) - 2*X(1) + X(2)+Y(2) +
    2*X(3) - 4*X(4) + 2*X(5) +
    X(6)+Y(6) - 2*X(7) + X(8)+Y(8);

  result.y =
    -X(0)+Y(0) + 2*Y(1) + X(2)+Y(2) +
    -2*Y(3) - 4*Y(4) - 2*Y(5) +
    X(6)+Y(6) + 2*Y(7) - X(8)+Y(8);

  return result;
}


void removeDivergence(vec2d *dst, vec2d *src, unsigned int width, unsigned int height) {
  for (int x=0; x<width; x++) {
    for (int y=0; y<height; y++) {
      int idx = width*y + x;
      vec2d grad = scale(divergenceGradient(src, width, height, x, y), 0.125);
      dst[idx] = add(src[idx], grad);
    }
  }
}
