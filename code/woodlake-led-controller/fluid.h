/* simple fluid dynamics simulation
 * based on Karl Sims' implementation at https://karlsims.com/fluid-flow.html
 */
#pragma once


typedef struct {
  float x, y;
} vec2d;


// dot product; a * b
float dot(vec2d a, vec2d b);

// vector addition; a+b
vec2d add(vec2d a, vec2d b);

// vector subtraction; a-b
vec2d sub(vec2d a, vec2d b);

// scalar multiplication; a * v;
vec2d scale(vec2d v, float a);

// component-wise multiplication
vec2d mult(vec2d a, vec2d b);


// remove the divergence from a vector field
void removeDivergence(vec2d *dst, vec2d *src, unsigned int width, unsigned int height);


struct FluidField {
  vec2d *bufA, *bufB;
  unsigned int width, height;

  void allocBuffers(unsigned int w, unsigned int h) {
    width = w;
    height = h;
    // bufA = malloc(w*h*sizeof(vec2d));
    // bufB = malloc(w*h*sizeof(vec2d));
    bufA = new vec2d[w*h];
    bufB = new vec2d[w*h];
  }

  void swapBuffers() {
    vec2d *tmp = bufA;
    bufA = bufB;
    bufB = tmp;
  }
};
