#ifndef POLLUTANT_H 
#define POLLUTANT_H 

#include <OctoWS2811.h>

#define RING_SCALE 20

struct PollutantPathSegment {
  int length();
  int addr(int idx);

  int start, end;
  PollutantPathSegment * next;
  PollutantPathSegment * mirror;
};


struct PollutantPath {
  int length();
  int addr(int idx, int &mirrorAddr);

  PollutantPathSegment * HEAD;
  int polluteTrailStart, polluteTrailEnd;
  int pollutantLevel;
};


struct PollutantRing {
  int length();
  int addr(int idx);
  int start, end;
  int level = 0;
};


struct Pollutant {
  PollutantRing ring;
  PollutantPath path;
  void fill();
  void drain(bool);
  bool draining = false;
  bool emit = false;
};


extern Pollutant phosphorus, salt, sediment, trash;

void setupPollutants();
void drawPollutant(OctoWS2811 &strip, Pollutant &pollutant, unsigned long color);

#endif
