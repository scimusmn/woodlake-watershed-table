#include "particle.h" 
#include <stdlib.h>


uint8_t randb() {
  return (rand() >> 8) & 0xff;
}


void Particle::activate(uint lifetime, uint x, uint y, uint vx, uint vy) {
  this->lifetime = lifetime;
  this->x = x;
  this->y = y;
  this->vx = vx;
  this->vy = vy;
}


bool Particle::isActive() {
  return lifetime != 0;
}


uint Particle::getX() {
  return x;
}
uint Particle::getY() {
  return y;
}


void Particle::update(uint8_t *topology, uint w, uint h, uint level) {
  if (topology[w*y + x] < level) {
    lifetime = 0;
  }
  if (lifetime == 0) {
    return;
  }

  int X = x;
  int Y = y;
  if ((randb()) & 1) {
    X += randb() < vx ? 1 : -1;
  } else {
    Y += randb() < vy ? 1 : -1;
  }

  // damp bias velocities over time
  if (vx > ZERO_VELOCITY) {
    vx -= 1;
  } else {
    vx += 1;
  }
  if (vy > ZERO_VELOCITY) {
    vy -= 1;
  } else {
    vy += 1;
  }

  lifetime -= 1;
  
  if (X < 0 || X >= w || Y < 0 || Y >= h) {
    // scatter
    vx = ZERO_VELOCITY;
    vy = ZERO_VELOCITY;
    return; 
  }

  if (topology[w*Y + X] >= level) {
    x = X;
    y = Y;
  } else {
    vx = ZERO_VELOCITY;
    vy = ZERO_VELOCITY;
  }
}
