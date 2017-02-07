#include "Particle.h"
#include "intercom_incoming.h"
#include "vs1063a_codec.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "plf_circular_buffer.h"

#define CIRCULAR_BUFFER_SIZE (8192)
#define UDP_PACKET_MAX_DATA_LENGTH (1024)

static uint8_t circularBuffer[CIRCULAR_BUFFER_SIZE];
static PlfCircularBuf_t circularBufCtxt;

void Intercom_Incoming::drain(void) {
    int numBytesBuffered = plf_circular_buf_used_space(&circularBufCtxt);
    uint8_t *decoderData;
    int totalNumBytesForCodec, decoderAvlSpace, sleepDurationMs;

    /*Let the SW buffer fill up a bit*/
    if (numBytesBuffered >= CIRCULAR_BUFFER_SIZE/2) {
        PLF_COUNT_VAL(DECODER_UNDERFLOW, VS1063aAudioBufferUnderflow());

        decoderAvlSpace = VS1063aStreamBufferFreeWords();

        numBytesBuffered = plf_circular_buf_used_space(&circularBufCtxt);

        if (numBytesBuffered==0)
        {
            PLF_COUNT_EVENT(CIRCULAR_BUF_UFL);
        }

        totalNumBytesForCodec = MIN(decoderAvlSpace, numBytesBuffered)&0xfffffffe; /*make even*/
        PLF_COUNT_VAL(BYTES_SENT_TO_DECODER, totalNumBytesForCodec);
        PLF_COUNT_MAX(BYTES_SENT_TO_DECODER_MAX, totalNumBytesForCodec);
        PLF_COUNT_MIN(BYTES_SENT_TO_DECODER_MIN, totalNumBytesForCodec);

        while (totalNumBytesForCodec) {
            int numBytesForCodec = plf_circular_buf_read_start(&circularBufCtxt, &decoderData, totalNumBytesForCodec);

            VS1063PlayBuf(decoderData, numBytesForCodec);

            plf_circular_buf_read_release(&circularBufCtxt, numBytesForCodec);
            totalNumBytesForCodec -= numBytesForCodec;
        }
    }
}

void Intercom_Incoming::receive(void)
{
    int rx_data_length;
    int free_space;
    static uint8_t rx_data[UDP_PACKET_MAX_DATA_LENGTH];

    rx_data_length = _udp.receivePacket(rx_data, sizeof(rx_data)/sizeof(rx_data[0]));

    if (rx_data_length > 0) {
      PLF_COUNT_VAL(UDP_BYTES_RX, rx_data_length);

      free_space = plf_circular_buf_free_space(&circularBufCtxt);
      if (free_space < rx_data_length) {
        PLF_COUNT_EVENT(CIRCULAR_BUF_OFL);
      }

      plf_circular_buf_write(&circularBufCtxt, rx_data, rx_data_length);
    }
}

Intercom_Incoming::Intercom_Incoming(int local_port) {
    plf_circular_buf_init(&circularBufCtxt, circularBuffer, CIRCULAR_BUFFER_SIZE);

    PLF_COUNT_MIN_INIT(BYTES_SENT_TO_DECODER_MIN);

    _udp.begin(local_port);
}
