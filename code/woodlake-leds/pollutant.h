#ifndef POLLUTANT_H 
#define POLLUTANT_H 

#include <OctoWS2811.h>

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
  int start, end, level;
};


struct Pollutant {
  PollutantRing ring;
  PollutantPath path;
  void fill();
  void drain(bool, bool&, bool&);
};


extern Pollutant phosphorus;

void setupPollutants();
void drawPollutant(OctoWS2811 &strip, Pollutant &pollutant, unsigned long color);

#endif
