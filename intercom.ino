#include "Particle.h"

#include "vs1063a_spi.h"
#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "intercom_root.h"
#include "plf_utils.h"
#include "SparkFunMAX17043.h"
#include "SparkFunSX1509.h"
#include "plf_data_dump.h"
#include "plf_registry.h"
#include "plf_event_counter.h"
#include "intercom_buttons_and_leds_sx1509.h"
#include "intercom_buttons_and_leds_stub.h"

PRODUCT_ID(3891);
PRODUCT_VERSION(1);

// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;
Plf_DataDump dataDump;
Plf_Registry plf_registry;
Plf_EvenCounter plf_eventCounter;

Intercom_Root *intercom_rootp = NULL;
MAX17043 *lipop = NULL;

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  delay(3000);

  /*Only enable default printgroup by default*/
  printGroupEnable(PRNTGRP_DFLT, true);

  String deviceID = System.deviceID();
  PLF_PRINT(PRNTGRP_DFLT, "Entered setup(). Device ID: %s", deviceID.c_str());

  /*On a real setup the SX1509 reset pin is pulled high. On a dummy setup it's pulled low*/
  pinMode(SX1509_RESET_PIN, INPUT);
  bool dummySetup = (digitalRead(SX1509_RESET_PIN) == LOW);
  if (dummySetup) {
    PLF_PRINT(PRNTGRP_DFLT, "This is a dummy setup.");
  }
  else {
    PLF_PRINT(PRNTGRP_DFLT, "This is a real setup.");
  }

  if (dummySetup) {
    STARTUP(WiFi.selectAntenna(ANT_INTERNAL));
  }
  else {
    STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));
  }
  

  VS1063InitHardware();

  PLF_PRINT(PRNTGRP_DFLT, "VS1063InitHardware done");
  VS1063InitSoftware();
  PLF_PRINT(PRNTGRP_DFLT, "VS1063InitSoftware done");
  VS1063RecordInit();
  PLF_PRINT(PRNTGRP_DFLT, "VS1063RecordInit done");

  //VS1063PrintState();

  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, "Photon");
  Particle.connect();

  if (dummySetup) {
    static Intercom_ButtonsAndLeds_Stub intercom_buttonsAndLeds_stub;
    static Intercom_Root intercom_root(intercom_buttonsAndLeds_stub);
    intercom_rootp = &intercom_root;
  }
  else {
    // Define a static MAX17043 object called lipo, which we'll use in the sketches.
    static MAX17043 lipo;

    lipop = &lipo;

    // Begin I2C
    Wire.begin();

    // Set up the MAX17043 LiPo fuel gauge:
    lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

    // Quick start restarts the MAX17043 in hopes of getting a more accurate
    // guess for the SOC.
    lipo.quickStart();

    // We can set an interrupt to alert when the battery SoC gets too low.
    // We can alert at anywhere between 1% - 32%:
    lipo.setThreshold(20); // Set alert threshold to 20%.

    static Intercom_ButtonsAndLeds_SX1509 intercom_buttonsAndLeds_sx1509;
    static Intercom_Root intercom_root(intercom_buttonsAndLeds_sx1509);
    intercom_rootp = &intercom_root;
  }

  PLF_PRINT(PRNTGRP_DFLT, "Starting loop, exiting setup()");
}

void loop() {
  if (intercom_rootp) {
    intercom_rootp->loop();
  }
}
