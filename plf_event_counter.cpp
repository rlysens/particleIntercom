#include "plf_event_counter.h"
#include "plf_utils.h"

#define READ_RESET_MODE
#define PRINT_PERIOD_MS (3000UL)

EventTuple_t plfEventArray[PLF_EVENT_LAST];

void plf_eventCounterTick(void) {
    int eventIndex;
    static int counter=0;
    static unsigned long prevMillis=0;
    unsigned long curMillis = millis();

    /*Handle wraparound by skipping a beat*/
    if (curMillis < prevMillis) {
      prevMillis = curMillis;
    }
    else if (curMillis - prevMillis > PRINT_PERIOD_MS) {
      prevMillis = curMillis;

      PLF_PRINT(PRNTGRP_STATS, "--- Events Counted[%d]--->\n",counter++);

      for (eventIndex=0; eventIndex<PLF_EVENT_LAST; eventIndex++) {
        //if ((icomEventArray[eventIndex].eventCount != 0) || (icomEventArray[eventIndex].initVal != 0)
        if (plfEventArray[eventIndex].eventName != 0) {
            PLF_PRINT(PRNTGRP_STATS, "%s: %d\n", plfEventArray[eventIndex].eventName, plfEventArray[eventIndex].eventCount);
            plfEventArray[eventIndex].eventCount = plfEventArray[eventIndex].initVal;
        }
      }
      PLF_PRINT(PRNTGRP_STATS, "<--- Events Counted---\n");
    }
}
