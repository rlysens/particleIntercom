#include "Particle.h"

#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "plf_utils.h"
// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;

void setup() {
  PLF_PRINT("Entered setup()");
  VS1063InitHardware();
  VS1063InitSoftware();
  test1();
  PLF_PRINT("Exiting setup()");
}

void loop() {
}
