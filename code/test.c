#include <stdlib.h>
#include <stdio.h>


int coinFlip() {
  return rand() > (RAND_MAX >> 1) ? 1 : -1;
}


int main(int argc, char **argv) {
  srand(argc);
  int sum = 0;
  for (int i=0; i<10000; i++) {
    sum += coinFlip();
    printf("%d\n", sum);
  }
  return 0;
}
