#include "state.h"


void PollutantState::activateParticle(unsigned int lifetime) {
  for (int i=0; i<N_PARTICLES; i++) {
    if (!particles[i].isActive()) {
      particles[i].activate(lifetime, x, y, cos8(angle), sin8(angle));
      return;
    }
  }
}

void drawParticle(rgb24 *buffer, int w, int h, Particle *p, rgb24 color) {
  if (p->isActive()) {
    buffer[w*p->getY() + p->getX()] = color;
  }
}

void PollutantState::render(rgb24 *buffer, int w, int h) {
  for (int i=0; i<N_PARTICLES; i++) {
    drawParticle(buffer, w, h, particles+i, (rgb24){ r, g, b });
  }
}

void PollutantState::update(uint8_t *topography, int w, int h, unsigned int level) {
  flowTimer.update();
  flowInterval.update();
  for (int i=0; i<N_PARTICLES; i++) {
    particles[i].update(topography, w, h, level);
  }
}

void PollutantState::startFlow(int amt) {
  flowInterval.start([](void *ptr) {
    PollutantState *self = (PollutantState*) ptr;
    self->activateParticle(N_PARTICLES);
  }, this, 1);
  flowTimer.start([](void *ptr) {
    PollutantState *self = (PollutantState*) ptr;
    self->stopFlow();
  }, this, 100*amt);
}
void PollutantState::stopFlow() {
  flowInterval.clear();
}
