#include "Particle.h"
#include "intercom_incoming.h"
#include "vs1063a_codec.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "plf_circular_buffer.h"
#include "messages.h"

#define CIRCULAR_BUFFER_SIZE (8192*2)
#define UDP_PACKET_MAX_DATA_LENGTH (512)

static uint8_t circularBuffer[CIRCULAR_BUFFER_SIZE];
static PlfCircularBuf_t circularBufCtxt;

#define BUFFER_NEARLY_EMPTY (CIRCULAR_BUFFER_SIZE/8)
#define BUFFER_HALF (CIRCULAR_BUFFER_SIZE/2)
#define BUFFER_NEARLY_FULL (CIRCULAR_BUFFER_SIZE - (CIRCULAR_BUFFER_SIZE/8))

#define DRAIN_STATE_FILL 0
#define DRAIN_STATE_DRAIN 1

static int message_handler_helper(Message &msg, 
  int payload_size, void *ctxt) {
  Intercom_Incoming *intercom_incomingp = (Intercom_Incoming*)ctxt;

  plf_assert("NULL ctxt ptr", intercom_incomingp);

  return intercom_incomingp->handle_message(msg, payload_size);
}

int Intercom_Incoming::handle_message(Message &msg, int payload_size) {
  switch (msg.id) {
    case MSG_ID_VOICE_DATA:
      return _receive(msg.data, payload_size);

    default:
      return -1;
  }

  return 0;
}

void Intercom_Incoming::drain(void) {
    int numBytesBuffered = plf_circular_buf_used_space(&circularBufCtxt);
    uint8_t *decoderData;
    int numBytesForCodec, decoderAvlSpace;
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
        numBytesForCodec = MIN(decoderAvlSpace, (int)(sizeof(zeroBuf)&0x7ffffffe));
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

int Intercom_Incoming::_receive(uint8_t *rx_data, int rx_data_length)
{
    int free_space;

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

    return 0;
}

Intercom_Incoming::Intercom_Incoming(Message_Handler& message_handler) :
  _message_handler(message_handler) {

    plf_circular_buf_init(&circularBufCtxt, circularBuffer, CIRCULAR_BUFFER_SIZE);

    PLF_COUNT_MIN_INIT(BYTES_SENT_TO_DECODER_MIN);
    PLF_COUNT_MIN_INIT(CIRCULAR_BUF_MIN);

    _message_handler.register_handler(MSG_ID_VOICE_DATA, message_handler_helper, this);
}
