#include "Particle.h"
#include "intercom_incoming.h"
#include "vs1063a_codec.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "plf_circular_buffer.h"

#define CIRCULAR_BUFFER_SIZE (8192*2)
#define UDP_PACKET_MAX_DATA_LENGTH (512)

static uint8_t circularBuffer[CIRCULAR_BUFFER_SIZE];
static PlfCircularBuf_t circularBufCtxt;

#define BUFFER_NEARLY_EMPTY (CIRCULAR_BUFFER_SIZE/8)
#define BUFFER_HALF (CIRCULAR_BUFFER_SIZE/2)
#define BUFFER_NEARLY_FULL (CIRCULAR_BUFFER_SIZE - (CIRCULAR_BUFFER_SIZE/8))

#define DRAIN_STATE_FILL 0
#define DRAIN_STATE_DRAIN 1

void Intercom_Incoming::drain(void) {
    int numBytesBuffered = plf_circular_buf_used_space(&circularBufCtxt);
    uint8_t *decoderData;
    int numBytesForCodec, decoderAvlSpace, sleepDurationMs;
    static int drain_state = DRAIN_STATE_FILL;
    static int discardNextByte=0;

    PLF_COUNT_MAX(CIRCULAR_BUF_MAX, numBytesBuffered);
    PLF_COUNT_MIN(CIRCULAR_BUF_MIN, numBytesBuffered);

    if ((drain_state == DRAIN_STATE_FILL) && (numBytesBuffered >= BUFFER_NEARLY_FULL)) {
      drain_state = DRAIN_STATE_DRAIN;
    }

    else if ((drain_state == DRAIN_STATE_DRAIN) && (numBytesBuffered < BUFFER_NEARLY_EMPTY)) {
      drain_state = DRAIN_STATE_FILL;
    }

    PLF_COUNT_VAL(DECODER_UNDERFLOW, VS1063aAudioBufferUnderflow());

    if (drain_state == DRAIN_STATE_FILL) {
        static uint8_t zeroBuf[64]={0};

        decoderAvlSpace = VS1063aStreamBufferFreeWords();
        numBytesForCodec = MIN(decoderAvlSpace, sizeof(zeroBuf)/sizeof(zeroBuf[0]))&0x7ffffffe;
        VS1063PlayBuf(zeroBuf, numBytesForCodec);
    }

    if (drain_state == DRAIN_STATE_DRAIN) {
        decoderAvlSpace = VS1063aStreamBufferFreeWords();
        numBytesBuffered = plf_circular_buf_used_space(&circularBufCtxt);
        numBytesForCodec = MIN(decoderAvlSpace, numBytesBuffered);

        if (numBytesForCodec && discardNextByte) {
          plf_circular_buf_read_start(&circularBufCtxt, &decoderData, 1);
          plf_circular_buf_read_release(&circularBufCtxt, 1);
          discardNextByte = 0;
        }

        numBytesBuffered = plf_circular_buf_used_space(&circularBufCtxt);
        numBytesForCodec = MIN(decoderAvlSpace, numBytesBuffered);
        numBytesForCodec = plf_circular_buf_read_start(&circularBufCtxt, &decoderData, numBytesForCodec);

        if (numBytesForCodec) {
          if ((numBytesForCodec&1)==0) {
            VS1063PlayBuf(decoderData, numBytesForCodec);
          }
          else { /*uneven number of bytes, make it even for decoder*/
            VS1063PlayBuf(decoderData, numBytesForCodec&0x7ffffffe);
            /*discard next byte too to fall back into even alignment*/
            discardNextByte=1;
          }

          plf_circular_buf_read_release(&circularBufCtxt, numBytesForCodec);

          PLF_COUNT_VAL(BYTES_SENT_TO_DECODER, numBytesForCodec);
          PLF_COUNT_MAX(BYTES_SENT_TO_DECODER_MAX, numBytesForCodec);
          PLF_COUNT_MIN(BYTES_SENT_TO_DECODER_MIN, numBytesForCodec);
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
      else {
        plf_circular_buf_write(&circularBufCtxt, rx_data, rx_data_length);
      }
    }
}

Intercom_Incoming::Intercom_Incoming(int local_port) {
    plf_circular_buf_init(&circularBufCtxt, circularBuffer, CIRCULAR_BUFFER_SIZE);

    PLF_COUNT_MIN_INIT(BYTES_SENT_TO_DECODER_MIN);
    PLF_COUNT_MIN_INIT(CIRCULAR_BUF_MIN);

    _udp.begin(local_port);
}
