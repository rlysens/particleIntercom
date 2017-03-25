#ifndef INTERCOM_BUTTONS_H
#define INTERCOM_BUTTONS_H

static inline bool recordButtonPressed(void) {
  return (digitalRead(D0)==HIGH);
}

#endif /*INTERCOM_BUTTONS_H*/