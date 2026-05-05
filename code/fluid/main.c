#include <stdio.h>
#include "fluid.h"


#define F_IDX(f, x, y) (f->width*y + x)


void fluid_fprint(FILE *F, fluid_t *f) {
  for (int x=0; x<f->width; x++) {
    for (int y=0; y<f->height; y++) {
      int idx = F_IDX(f, x, y);
      fprintf(F, "%d %d %f %f\n", x, y, f->x[idx], f->y[idx]);
    }
  }
}


int main() {
  fluid_t *f = fluid_create(128, 64);

  for (int x=0; x<f->width; x++) {
    for (int y=0; y<f->width; y++) {
      int r2 = (x-32)*(x-32) + (y-32)*(y-32);
      if (r2 <=16) {
        f->mask[F_IDX(f, x, y)] = true;
      }
    }
  }

  float v = 8;
  for (int i=0; i<800; i++) {
    for (int y=2; y<5; y++) {
      f->y[F_IDX(f, 64, y)] = v;
      f->y[F_IDX(f, 65, y)] = v;
      f->y[F_IDX(f, 64, 59-y)] = -v;
      f->y[F_IDX(f, 65, 59-y)] = -v;
    }
    fluid_update(f, 10);
  }

  fluid_fprint(stdout, f);
  fluid_destroy(f);
  return 0;
}
