#include "color.h"


uint8_t scale8(uint8_t x, uint8_t scale) {
  return (((int)x * (int)scale) >> 8) + ((x && scale) ? 1 : 0);
}


uint8_t lerp8(uint8_t a, uint8_t b, uint8_t s) {
  if (b > a) {
    return a + scale8(b - a, s);
  } else {
    return a - scale8(a - b, s);
  }
}


void color_t::set(uint8_t red, uint8_t green, uint8_t blue) {
  r = red;
  g = green;
  b = blue;
}

uint16_t color_t::value() {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


color_t color_lerp(color_t a, color_t b, uint8_t s) {
  color_t result;
  result.r = lerp8(a.r, b.r, s);
  result.g = lerp8(a.g, b.g, s);
  result.b = lerp8(a.b, b.b, s);
  return result;
}
