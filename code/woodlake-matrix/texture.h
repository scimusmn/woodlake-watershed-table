#ifndef TEXTURE_H
#define TEXTURE_H


#include <stdint.h>


struct texture_t {
  uint8_t *data;
  int w, h;
};


int mod(int a, int b);
uint8_t sample(texture_t tex, int x, int y);


extern texture_t displacement;
extern texture_t waves;


#endif
