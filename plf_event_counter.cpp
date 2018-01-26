#include "plf_event_counter.h"
#include "plf_utils.h"
#include "plf_data_dump.h"

#define MODULE_ID 1000

#define READ_RESET_MODE

Plf_EvenCounter plf_eventCounter;

Plf_EvenCounter::Plf_EvenCounter() {
  memset(_plfEventArray, 0, sizeof(_plfEventArray));
  dataDump.registerFunction("Stats", &Plf_EvenCounter::_dataDump, this);
}

void Plf_EvenCounter::_dataDump(void) {
  int eventIndex;
  
  for (eventIndex=0; eventIndex<PLF_EVENT_LAST; eventIndex++) {
    //if ((icomEventArray[eventIndex].eventCount != 0) || (icomEventArray[eventIndex].initVal != 0)
    if (_plfEventArray[eventIndex].eventName != 0) {
        PLF_PRINT(PRNTGRP_DFLT, "%s: %d", _plfEventArray[eventIndex].eventName, _plfEventArray[eventIndex].eventCount);
        _plfEventArray[eventIndex].eventCount = _plfEventArray[eventIndex].initVal;
    }
  }
}
