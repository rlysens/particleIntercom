#include "Particle.h"

#include "vs1063a_spi.h"
#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "intercom_buttons.h"

// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  IPAddress localIP = WiFi.localIP();
  String myID = System.deviceID();

  /*Only enable default printgroup by default*/
  printGroupEnable(PRNTGRP_DFLT, true);

  while (!recordButtonPressed());

  PLF_PRINT(PRNTGRP_DFLT, "Entered setup()");
  Serial.println(localIP);

  // Prints out the device ID over Serial
  Serial.println(myID);

  VS1063InitHardware();
  VS1063InitSoftware();

  VS1063RecordInit();
  PLF_PRINT(PRNTGRP_DFLT, "VS1063RecordInit done\n");

  VS1063PrintState();

  Particle.connect();

  test8_setup();
  PLF_PRINT(PRNTGRP_DFLT, "Starting test, exiting setup()");
}

void loop() {
  test8_loop();
  plf_event_counter_tick();
}
