#pragma once

#include <stdint.h>
#include <stdlib.h>


#define ZERO_VELOCITY 127


typedef unsigned int uint;


class Particle {
  public:
  void activate(uint lifetime, uint x, uint y, uint vx=ZERO_VELOCITY, uint vy=ZERO_VELOCITY);
  bool isActive();
  uint getX();
  uint getY();
  void update(uint8_t *topology, uint w, uint h, uint level);

  protected:
  int x=0;
  int y=0;
  int vx=0;
  int vy=0;
  uint lifetime=0;
};
