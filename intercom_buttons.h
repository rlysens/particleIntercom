#ifndef INTERCOM_BUTTONS_H
#define INTERCOM_BUTTONS_H

#include "Particle.h"

static inline void recordButtonInit(void) {
	pinMode(D2, INPUT_PULLDOWN);
}

static inline bool recordButtonPressed(void) {
  return (digitalRead(D2)==HIGH);
}

#endif /*INTERCOM_BUTTONS_H*/