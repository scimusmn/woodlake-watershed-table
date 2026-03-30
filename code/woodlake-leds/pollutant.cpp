#include "led-map.h"
#include "pollutant.h"


int _length(int start, int end) {
  if (start < end) {
    return end - start;
  } else {
    return start - end;
  }
}


int _addr(int start, int end, int idx) {
  if (idx >= _length(start, end)) {
    return -1;
  }

  if (start < end) {
    return start + idx;
  } else {
    return start - idx - 1;
  }
}


int PollutantPathSegment::length() { return _length(start, end); }
int PollutantPathSegment::addr(int idx) { return _addr(start, end, idx); }

int PollutantRing::length() { return _length(start, end); }
int PollutantRing::addr(int idx) { return _addr(start, end, idx); }


int PollutantPath::length() {
  int len = 0;
  for (PollutantPathSegment *segment = HEAD; segment != nullptr; segment = segment->next) {
    len += segment->length();
  }
  return len;
}


int PollutantPath::addr(int idx, int &mirrorAddr) {
  for (PollutantPathSegment *segment = HEAD; segment != nullptr; segment = segment->next) {
    if (idx < segment->length()) {
      if (segment->mirror != nullptr) {
        mirrorAddr = segment->mirror->addr(idx);
      } else {
        mirrorAddr = -1;
      }
      return segment->addr(idx);
    } else {
      idx -= segment->length();
    }
  }

  // out of range!
  return -1;
}



unsigned long waterColor(int idx) {
  return 0x0000ff;
}


void drawPollutantPath(OctoWS2811 &strip, PollutantPath &path, unsigned long polluteColor) {
  int trailLen = path.polluteTrailEnd - path.polluteTrailStart;
  for (int i=0; i<trailLen; i++) {
    int mirrorAddr;
    int addr = path.addr(i+path.polluteTrailStart, mirrorAddr);
    if (addr < 0) {
      continue;
    }

    unsigned long color;

    if (i > trailLen - path.pollutantLevel) {
      color = polluteColor;
    } else {
      color = waterColor(i);
    }

    if (mirrorAddr >= 0) {
      strip.setPixel(mirrorAddr, color);
    }
    strip.setPixel(addr, color);
  }
}


void drawPollutantRing(OctoWS2811 &strip, PollutantRing &ring, unsigned long polluteColor) {
  for (int i=0; i<ring.level; i++) {
    strip.setPixel(ring.addr(i), polluteColor);
  }
}


void drawPollutant(OctoWS2811 &strip, Pollutant &pollutant, unsigned long color) {
  drawPollutantPath(strip, pollutant.path, color);
  drawPollutantRing(strip, pollutant.ring, color);
}


void Pollutant::fill() {
  ring.level += 1;
  if (ring.level > ring.length()) {
    ring.level = ring.length();
  }
  path.polluteTrailStart = 0;
  path.polluteTrailEnd = 0;
}


void Pollutant::drain(bool water) {
  if (ring.level > 0) {
    ring.level -= 1;
    path.polluteTrailEnd += 1;
    path.pollutantLevel += 1;
  } else {
    if (!water) {
      path.polluteTrailStart += 1;
    }
    path.polluteTrailEnd += 1;
  }

  draining = true;
  emit = false;

  if (path.polluteTrailStart > path.length()) {
    draining = false;
    path.polluteTrailStart = 0;
    path.polluteTrailEnd = 0;
    path.pollutantLevel = 0;
    ring.level = 0;
  }


  if (path.polluteTrailEnd > path.length()) {
    emit = true;
  }
}



Pollutant phosphorus, salt, sediment, trash;



void setupPollutants() {
  phosphorus.ring.start = PHOSPHORUS_RING_START;
  phosphorus.ring.end = PHOSPHORUS_RING_END;

  salt.ring.start = SALT_RING_START;
  salt.ring.end = SALT_RING_END;

  sediment.ring.start = SEDIMENT_RING_START;
  sediment.ring.end = SEDIMENT_RING_END;

  trash.ring.start = TRASH_RING_START;
  trash.ring.end = TRASH_RING_END;

  PollutantPathSegment *prev;
  #define START_SEGMENT(A, B) do {\
    POLLUTE.path.HEAD = new PollutantPathSegment(); \
    prev = POLLUTE.path.HEAD; \
    prev->start = A; \
    prev->end = B; \
    prev->next = nullptr; \
    prev->mirror = nullptr; \
  } while (0)

  #define ADD_SEGMENT(A, B) do {\
    prev->next = new PollutantPathSegment(); \
    prev = prev->next; \
    prev->start = A; \
    prev->end = B; \
    prev->next = nullptr; \
    prev->mirror = nullptr; \
  } while (0)

  #define ADD_MIRROR(A, B) do {\
    prev->mirror = new PollutantPathSegment(); \
    prev->mirror->start = A; \
    prev->mirror->end = B; \
    prev->mirror->next = nullptr; \
    prev->mirror->mirror = nullptr; \
  } while (0)

  #define POLLUTE phosphorus
  START_SEGMENT(PHOSPHORUS_FLOWA_START, PHOSPHORUS_FLOWA_END);
  ADD_MIRROR(PHOSPHORUS_FLOWB_START, PHOSPHORUS_FLOWB_END);
  // ADD_SEGMENT(SALT_RING_START, SALT_RING_END);
  // ADD_SEGMENT(SALT_FLOWA_START, SALT_FLOWA_END);
  // ADD_MIRROR(SALT_FLOWB_START, SALT_FLOWB_END);
  #undef POLLUTE

  #define POLLUTE salt
  START_SEGMENT(SALT_FLOWA_START, SALT_FLOWA_END);
  ADD_MIRROR(SALT_FLOWB_START, SALT_FLOWB_END);
  #undef POLLUTE

  #define POLLUTE sediment
  START_SEGMENT(SEDIMENT_FLOWA_START, SEDIMENT_FLOWA_END);
  ADD_MIRROR(SEDIMENT_FLOWB_START, SEDIMENT_FLOWB_END);
  #undef POLLUTE

  #define POLLUTE trash
  START_SEGMENT(TRASH_FLOWA_START, TRASH_FLOWA_END);
  ADD_MIRROR(TRASH_FLOWB_START, TRASH_FLOWB_END);
  #undef TRASH
}
