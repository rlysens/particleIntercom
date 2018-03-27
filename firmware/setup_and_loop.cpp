#include "application.h"
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
#include "intercom_power_management.h"

#define WIFI_CONNECT_TIMEOUT_MS 10000

PRODUCT_ID(3891);
PRODUCT_VERSION(1);

SYSTEM_THREAD(ENABLED);

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

retained bool enterListenMode;
bool inListenMode;

// Use primary serial over USB interface for logging output. Used by PLF_PRINT
SerialLogHandler logHandler;
Plf_DataDump dataDump;
Plf_Registry plf_registry;
Plf_EvenCounter plf_eventCounter;

Intercom_Root *intercom_rootp = NULL;
Intercom_PowerManagement *intercom_powerManagementp = NULL;

MAX17043 *lipop = NULL;

SYSTEM_MODE(MANUAL);

static bool isDummySetup(void) {
  /*On a real setup the SX1509 reset pin is pulled high. On a dummy setup it's pulled low*/
  pinMode(SX1509_RESET_PIN, INPUT);
  bool dummySetup = (digitalRead(SX1509_RESET_PIN) == HIGH);
  
  return dummySetup;
}

static bool listenModeCheck() {
  if (enterListenMode) {
      enterListenMode = false;

      printGroupEnable(PRNTGRP_DFLT, true);
      PLF_PRINT(PRNTGRP_DFLT, "Entered Listen mode.");
      if (!isDummySetup()) {
        // Begin I2C
        Wire.begin();
        Intercom_ButtonsAndLeds *intercom_buttonsAndLeds = new Intercom_ButtonsAndLeds_SX1509();
        if (intercom_buttonsAndLeds) {
          intercom_buttonsAndLeds->getLedBar().blink(200,200);
        }
      }

      WiFi.on();
      WiFi.listen();

      return true;
  }
  else {
    return false;
  }
}

void setup() {
  delay(3000); /*Just enough to connect serial collect first traces*/

  intercom_powerManagementp = new Intercom_PowerManagement();
  plf_assert("intercom_powerManagementp NULL ptr", intercom_powerManagementp);

  /*Listening mode requires a lot of memory and can't coexist with the footprint
   *of our application. As a workaround we enter listening mode very early in setup(), 
   *i.e. before setup() allocates the memory for the majority of our application objects
   *(i.e. new Intercom).*/
  inListenMode = listenModeCheck();

  if (!inListenMode) {
    /*Using an init function instead of constructor for these globals to avoid running
     *code before setup()*/
    plf_registry.init();
    plf_eventCounter.init();

    /*Only enable default printgroup by default*/
    printGroupEnable(PRNTGRP_DFLT, true);

    String deviceID = System.deviceID();
    PLF_PRINT(PRNTGRP_DFLT, "Entered setup(). Device ID: %s", deviceID.c_str());

    bool dummySetup = isDummySetup();
    PLF_PRINT(PRNTGRP_DFLT, "This is a %s setup.", dummySetup ? "dummy" : "real");
    
    WiFi.on();
    WiFi.selectAntenna(dummySetup? ANT_INTERNAL : ANT_EXTERNAL);

    /*If we don't have any credentials, we enter listening mode through a reset.*/
    if (!WiFi.hasCredentials()) {
      enterListenMode = true;
      System.reset();
    }

    WiFi.connect(); /*We know we have credentials so this call won't drop into listening mode.*/

    /*If we failed to connect, we enter listening mode through a reset.*/
    if (!waitFor(WiFi.ready, WIFI_CONNECT_TIMEOUT_MS)) {
      PLF_PRINT(PRNTGRP_DFLT, "Can't connect to WiFi. Entering listen mode.");
      enterListenMode = true;
      System.reset();
    }

    VS1063InitHardware();
    PLF_PRINT(PRNTGRP_DFLT, "VS1063InitHardware done");
    VS1063InitSoftware();
    PLF_PRINT(PRNTGRP_DFLT, "VS1063InitSoftware done");
    VS1063RecordInit();
    PLF_PRINT(PRNTGRP_DFLT, "VS1063RecordInit done");

    if (dummySetup) {
      Intercom_ButtonsAndLeds *intercom_buttonsAndLeds = new Intercom_ButtonsAndLeds_Stub();
      plf_assert("intercom_buttonsAndLeds NULL ptr", intercom_buttonsAndLeds);
      /*We use dynamic memory allocation here to ensure that this memory is
       *only allocated when we get here, i.e. when we're not in listening mode.*/
      intercom_rootp = new Intercom_Root(*intercom_buttonsAndLeds);
      plf_assert("intercom_rootp NULL ptr", intercom_rootp);
    }
    else {
      lipop = new MAX17043();
      plf_assert("lipop NULL ptr", lipop);

      // Begin I2C
      Wire.begin();

      // Set up the MAX17043 LiPo fuel gauge:
      lipop->begin(); // Initialize the MAX17043 LiPo fuel gauge

      // Quick start restarts the MAX17043 in hopes of getting a more accurate
      // guess for the SOC.
      lipop->quickStart();

      // We can set an interrupt to alert when the battery SoC gets too low.
      // We can alert at anywhere between 1% - 32%:
      lipop->setThreshold(20); // Set alert threshold to 20%.

      Intercom_ButtonsAndLeds *intercom_buttonsAndLeds = new Intercom_ButtonsAndLeds_SX1509();
      plf_assert("intercom_buttonsAndLeds NULL ptr", intercom_buttonsAndLeds);

      /*We use dynamic memory allocation here to ensure that this memory is
       *only allocated when we get here, i.e. when we're not in listening mode.*/
      intercom_rootp = new Intercom_Root(*intercom_buttonsAndLeds);
      plf_assert("intercom_rootp NULL ptr", intercom_rootp);
    }

    /*Connect to the cloud.*/
    Particle.connect();

    PLF_PRINT(PRNTGRP_DFLT, "Free memory: %d", (int)System.freeMemory());
    PLF_PRINT(PRNTGRP_DFLT, "Starting loop, exiting setup()");
  }
}

void loop() {
  if (inListenMode) {
    waitUntil(WiFi.listening);
    waitUntil(WiFi.ready);
    
    PLF_PRINT(PRNTGRP_DFLT, "Listening mode completed. Restarting...");
    /*When done, restart in regular mode.*/
    enterListenMode = false;
    System.reset();
  }
  else {
      intercom_rootp->loop();
      intercom_powerManagementp->checkPowerSwitch();
  }

  Particle.process();
}
