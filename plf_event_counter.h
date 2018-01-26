#ifndef PLF_EVENT_COUNTER_H
#define PLF_EVENT_COUNTER_H

#include "Particle.h"
#include "plf_utils.h"

#define PLF_EVENT_COUNTER_PERIOD (10000)

typedef struct {
    int eventCount;
    const char *eventName;
    int initVal;
} EventTuple_t;

enum EventIds {
    UDP_BYTES_RX,
    RX_BUF_PREFILL_DONE,
    DECODER_UNDERFLOW,
    BYTES_SENT_TO_DECODER,
    BYTES_SENT_TO_DECODER_MAX,
    BYTES_SENT_TO_DECODER_MIN,
    ENCODER_AVL_BYTES,
    ENCODER_AVL_BYTES_MAX,
    ENCODER_AVL_BYTES_MIN,
    NO_ENCODER_AVL_BYTES,
    ENCODER_AUDIO_FREE_MAX,
    ENCODER_AUDIO_FREE_MIN,
    ENCODER_OUTBUF_FREE_MAX,
    ENCODER_OUTBUF_FREE_MIN,
    ENCODER_OUTBUF_FILL_MAX,
    ENCODER_OUTBUF_FILL_MIN,
    UDP_BYTES_TX,
    UDP_PACKET_CREATED,
    CIRCULAR_BUF_OFL,
    CIRCULAR_BUF_UFL,
    CIRCULAR_BUF_MAX,
    CIRCULAR_BUF_MIN,
    CODEC_THREAD_SLEEP_TIME,
    INTERCOM_OUTGOING_TICK,
    BYTES_MISSED,
    PLF_EVENT_LAST
};

#define PLF_COUNT_EVENT(eventId) plf_eventCounter.countEvent(eventId, #eventId)
#define PLF_COUNT_VAL(eventId, val) plf_eventCounter.countVal(eventId, val, #eventId)

#define PLF_COUNT_MAX(eventId, val) plf_eventCounter.countMax(eventId, val, #eventId)
#define PLF_COUNT_MIN(eventId, val) plf_eventCounter.countMin(eventId, val, #eventId)

#define PLF_COUNT_MIN_INIT(eventId) plf_eventCounter.countMinInit(eventId)

void plf_eventCounterTick(void);

class Plf_EvenCounter {
private:
    EventTuple_t _plfEventArray[PLF_EVENT_LAST];

    void _dataDump(void);

public:
    Plf_EvenCounter();

    inline void countEvent(int eventId, const char* eventName) {
        ++(_plfEventArray[eventId].eventCount); 
        _plfEventArray[eventId].eventName=eventName;    
    }

    inline void countVal(int eventId, int val, const char* eventName) {
        _plfEventArray[eventId].eventCount += val; 
        _plfEventArray[eventId].eventName = eventName;
    }

    inline void countMax(int eventId, int val, const char* eventName) {
        int temp=val;
        _plfEventArray[eventId].eventCount = MAX(_plfEventArray[eventId].eventCount, temp);
        _plfEventArray[eventId].eventName = eventName;
    }

    inline void countMin(int eventId, int val, const char* eventName) {
        int temp=val;
        _plfEventArray[eventId].eventCount = MIN(_plfEventArray[eventId].eventCount, temp);
        _plfEventArray[eventId].eventName = eventName;
    }

    inline void countMinInit(int eventId) {
        _plfEventArray[eventId].initVal = 0x7fffffff; 
        _plfEventArray[eventId].eventCount = _plfEventArray[eventId].initVal;
    }
};

extern Plf_EvenCounter plf_eventCounter;
#endif /*PLF_EVENT_COUNTER_H*/
