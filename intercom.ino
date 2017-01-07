#include "Particle.h"

#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "plf_utils.h"
// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;

void setup() {
  PLF_PRINT("Entered setup()");
  pinMode(D0, INPUT);
  PLF_PRINT("Pull up D0...");
  while(digitalRead(D0)==LOW);
  //VS1063InitHardware();
  //VS1063InitSoftware();
  PLF_PRINT("Starting test2");
  test2();
  PLF_PRINT("Test2 complete.");
  PLF_PRINT("Exiting setup()");
}

void loop() {
  PLF_PRINT("In loop()");
  delay(3000);
}
