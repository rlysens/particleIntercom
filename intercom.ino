#include "Particle.h"

#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "plf_utils.h"
// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;

void setup() {
  PLF_PRINT("Entered setup()");
  pinMode(D0, INPUT);
  delay(5000);
  PLF_PRINT("Pull up D0...");
  while(digitalRead(D0)==LOW);
  //VS1063InitHardware();
  //VS1063InitSoftware();
  PLF_PRINT("Starting test1");
  test1();
  PLF_PRINT("Test1 complete.");
  PLF_PRINT("Exiting setup()");
}

void loop() {
  PLF_PRINT("In loop()");
  delay(3000);
}
