#include "Particle.h"

#include "vs1063a_spi.h"
#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "plf_utils.h"
// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;

void setup() {
  PLF_PRINT("Entered setup()");

  //VS1063InitHardware();
  //VS1063InitSoftware();

  VS1063InitSPI();
  //pinMode(D0, INPUT);
  //PLF_PRINT("Pull up D0...");
  //while(digitalRead(D0)==LOW);
  //PLF_PRINT("Starting test4, exiting setup()");
}

void loop() {
  test4();
}
