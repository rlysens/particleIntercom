#include "Particle.h"
// This #include statement was automatically added by the Particle IDE.
#include "vs1063a_codec.h"

// Use primary serial over USB interface for logging output
SerialLogHandler logHandler;

void setup() {
  VS1063InitHardware();
  VS1063InitSoftware();
}

void loop() {
}
