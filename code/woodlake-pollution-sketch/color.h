#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>


struct color_t {
  uint8_t r, g, b;

  void set(uint8_t red, uint8_t green, uint8_t blue);
  uint16_t value();
};




uint8_t scale8(uint8_t x, uint8_t scale);
uint8_t lerp8(uint8_t a, uint8_t b, uint8_t s);


color_t color_lerp(color_t a, color_t b, uint8_t s);


#endif
