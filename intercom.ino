#include "Particle.h"

#include "vs1063a_spi.h"
#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "plf_utils.h"
#include "plf_event_counter.h"

// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;

void setup() {
  int ii;
  IPAddress localIP = WiFi.localIP();
  String myID = System.deviceID();

  for (ii=0; ii<10; ii++) {
    PLF_PRINT("%d\n", 10-ii);
    delay(1000);
  }

  PLF_PRINT("Entered setup()");
  Serial.println(localIP);

  // Prints out the device ID over Serial
  Serial.println(myID);

  VS1063InitHardware();
  VS1063InitSoftware();

  VS1063RecordInit();
  PLF_PRINT("VS1063RecordInit done\n");

  VS1063PrintState();
  pinMode(D0, INPUT);
  //PLF_PRINT("Pull up D0...");

  test8_setup();
  PLF_PRINT("Starting test, exiting setup()");
}

void loop() {
  test8_loop();
  plf_event_counter_tick();
}
