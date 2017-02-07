#ifndef PLF_EVENT_COUNTER_H
#define PLF_EVENT_COUNTER_H

#include "Particle.h"

#define PLF_EVENT_COUNTER_PERIOD (10000)

typedef struct {
    int eventCount;
    char *eventName;
    int initVal;
} EventTuple_t;

extern EventTuple_t plfEventArray[];

enum EventIds {
    UDP_BYTES_RX,
    RX_BUF_PREFILL_DONE,
    DECODER_UNDERFLOW,
    BYTES_SENT_TO_DECODER,
    BYTES_SENT_TO_DECODER_MAX,
    BYTES_SENT_TO_DECODER_MIN,
    ENCODER_AVL_BYTES,
    NO_ENCODER_AVL_BYTES,
    ENCODER_AUDIO_FREE,
    ENCODER_OUTBUF_FREE,
    UDP_BYTES_TX,
    UDP_PACKET_CREATED,
    CIRCULAR_BUF_OFL,
    CIRCULAR_BUF_UFL,
    CODEC_THREAD_SLEEP_TIME,
    PLF_EVENT_LAST
};

#define PLF_COUNT_EVENT(eventId) do {++(plfEventArray[eventId].eventCount); plfEventArray[eventId].eventName=#eventId;} while (0)
#define PLF_COUNT_VAL(eventId, val) do {plfEventArray[eventId].eventCount += (val); plfEventArray[eventId].eventName=#eventId;} while (0)

#define PLF_COUNT_MAX(eventId, val) do {int temp=val;plfEventArray[eventId].eventCount = MAX(plfEventArray[eventId].eventCount, temp);plfEventArray[eventId].eventName=#eventId;} while (0)
#define PLF_COUNT_MIN(eventId, val) do {int temp=val;plfEventArray[eventId].eventCount = MIN(plfEventArray[eventId].eventCount, temp);plfEventArray[eventId].eventName=#eventId;} while (0)

#define PLF_COUNT_MIN_INIT(eventId) do {plfEventArray[eventId].initVal = 0x7fffffff; plfEventArray[eventId].eventCount = plfEventArray[eventId].initVal;} while (0)

void plf_event_counter_tick(void);

#endif /*PLF_EVENT_COUNTER_H*/
