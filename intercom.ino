#include "Particle.h"

#include "vs1063a_spi.h"
#include "vs1063a_codec.h"
#include "intercom_tests.h"
#include "intercom_root.h"
#include "plf_utils.h"
#include "SparkFunMAX17043.h"
#include "SparkFunSX1509.h"

PRODUCT_ID(3891);
PRODUCT_VERSION(1);

// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;
Intercom_Root *intercom_rootp = NULL;

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  String deviceID = System.deviceID();

  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, "Photon");
  
  //STARTUP(WiFi.selectAntenna(ANT_INTERNAL)); // selects the CHIP antenna
  STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna

  delay(3000);

  /*Only enable default printgroup by default*/
  printGroupEnable(PRNTGRP_DFLT, true);

  // Begin I2C
  Wire.begin();

  PLF_PRINT(PRNTGRP_DFLT, "Entered setup()");

  // Prints out the device ID over Serial
  Serial.println(deviceID);

  VS1063InitHardware();

  PLF_PRINT(PRNTGRP_DFLT, "VS1063InitHardware done");
  VS1063InitSoftware();
  PLF_PRINT(PRNTGRP_DFLT, "VS1063InitSoftware done");
  VS1063RecordInit();
  PLF_PRINT(PRNTGRP_DFLT, "VS1063RecordInit done");

  VS1063PrintState();

  // Set up the MAX17043 LiPo fuel gauge:
  lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

  // Quick start restarts the MAX17043 in hopes of getting a more accurate
  // guess for the SOC.
  lipo.quickStart();

  // We can set an interrupt to alert when the battery SoC gets too low.
  // We can alert at anywhere between 1% - 32%:
  lipo.setThreshold(20); // Set alert threshold to 20%.

  Particle.connect();

  static Intercom_Root intercom_root;

  intercom_rootp = &intercom_root;

  PLF_PRINT(PRNTGRP_DFLT, "Starting loop, exiting setup()");
}

void loop() {
  if (intercom_rootp) {
    intercom_rootp->loop();
  }
}
