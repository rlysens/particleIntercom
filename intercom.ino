#include "Particle.h"

#include "vs1063a_spi.h"
#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "intercom_root.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "intercom_buttons.h"

PRODUCT_ID(3891);
PRODUCT_VERSION(1);

// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;
Intercom_Root *intercom_rootp = NULL;

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  IPAddress localIP = WiFi.localIP();
  String myID = System.deviceID();

  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, "Photon");

  /*Only enable default printgroup by default*/
  printGroupEnable(PRNTGRP_DFLT, true);

#if 0
  while (!recordButtonPressed());
#endif

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

  {
    static Intercom_Root intercom_root;

    intercom_rootp = &intercom_root;
  }

  PLF_PRINT(PRNTGRP_DFLT, "Starting loop, exiting setup()");
}

void loop() {
  if (intercom_rootp) {
    intercom_rootp->loop();
  }
  
  plf_event_counter_tick();
}
