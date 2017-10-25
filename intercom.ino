#include "Particle.h"

#include "vs1063a_spi.h"
#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "intercom_root.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "intercom_buttons.h"
#include "SparkFunMAX17043.h"

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

  recordButtonInit();

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

  // Set up the MAX17043 LiPo fuel gauge:
  lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

  // Quick start restarts the MAX17043 in hopes of getting a more accurate
  // guess for the SOC.
  lipo.quickStart();

  // We can set an interrupt to alert when the battery SoC gets too low.
  // We can alert at anywhere between 1% - 32%:
  lipo.setThreshold(20); // Set alert threshold to 20%.

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
