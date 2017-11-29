#include "Particle.h"
#include "intercom_incoming.h"
#include "vs1063a_codec.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "messages.h"

#define MODULE_ID 500

#define BUFFER_NEARLY_EMPTY (CIRCULAR_BUFFER_SIZE/8)
#define BUFFER_HALF (CIRCULAR_BUFFER_SIZE/2)
#define BUFFER_NEARLY_FULL (CIRCULAR_BUFFER_SIZE - (CIRCULAR_BUFFER_SIZE/8))

#define DRAIN_STATE_FILL 0
#define DRAIN_STATE_DRAIN 1

static int messageHandlerHelper(Intercom_Message &msg, 
  int payloadSize, void *ctxt) {
  Intercom_Incoming *intercom_incomingp = (Intercom_Incoming*)ctxt;

  plf_assert("NULL ctxt ptr", intercom_incomingp);

  return intercom_incomingp->handleMessage(msg, payloadSize);
}

int Intercom_Incoming::handleMessage(Intercom_Message &msg, int payloadSize) {
  
  switch (msg.msg_id) {
    case VOICE_DATA_T_MSG_ID:
    {
      static voice_data_t voice_data;
      int numDecodedMsgBytes = voice_data_t_decode(msg.data, 0, payloadSize, &voice_data);

      if (numDecodedMsgBytes < 0) {
        return -(MODULE_ID+1);
      }

      return _receive(voice_data.data, voice_data.data_size);
    }

    default:
      return -(MODULE_ID+2);
  }

  return 0;
}

void Intercom_Incoming::drain(void) {
  int numBytesBuffered = _circularBuf.usedSpace();
  uint8_t *decoderData;
  int numBytesForCodec, decoderAvlSpace;
  static int drainState = DRAIN_STATE_FILL;
  static int discardNextByte=0;

  PLF_COUNT_MAX(CIRCULAR_BUF_MAX, numBytesBuffered);
  PLF_COUNT_MIN(CIRCULAR_BUF_MIN, numBytesBuffered);

  if ((drainState == DRAIN_STATE_FILL) && (numBytesBuffered >= BUFFER_NEARLY_FULL)) {
    drainState = DRAIN_STATE_DRAIN;
  }

  else if ((drainState == DRAIN_STATE_DRAIN) && (numBytesBuffered < BUFFER_NEARLY_EMPTY)) {
    drainState = DRAIN_STATE_FILL;
  }

  PLF_COUNT_VAL(DECODER_UNDERFLOW, VS1063aAudioBufferUnderflow());

  if (drainState == DRAIN_STATE_FILL) {
    static uint8_t zeroBuf[64]={0};

    decoderAvlSpace = VS1063aStreamBufferFreeBytes();
    numBytesForCodec = MIN(decoderAvlSpace, (int)(sizeof(zeroBuf)&0x7ffffffe));
    VS1063PlayBuf(zeroBuf, numBytesForCodec);
  }

  if (drainState == DRAIN_STATE_DRAIN) {
    decoderAvlSpace = VS1063aStreamBufferFreeBytes();
    numBytesBuffered = _circularBuf.usedSpace();
    numBytesForCodec = MIN(decoderAvlSpace, numBytesBuffered);

    if (numBytesForCodec && discardNextByte) {
      _circularBuf.readStart(&decoderData, 1);
      _circularBuf.readRelease(1);
      discardNextByte = 0;
    }

    numBytesBuffered = _circularBuf.usedSpace();
    numBytesForCodec = MIN(decoderAvlSpace, numBytesBuffered);
    numBytesForCodec = _circularBuf.readStart(&decoderData, numBytesForCodec);

    if (numBytesForCodec) {
      if ((numBytesForCodec&1)==0) {
        VS1063PlayBuf(decoderData, numBytesForCodec);
      }
      else { /*uneven number of bytes, make it even for decoder*/
        VS1063PlayBuf(decoderData, numBytesForCodec&0x7ffffffe);
        /*discard next byte too to fall back into even alignment*/
        discardNextByte=1;
      }

      _circularBuf.readRelease(numBytesForCodec);

      PLF_COUNT_VAL(BYTES_SENT_TO_DECODER, numBytesForCodec);
      PLF_COUNT_MAX(BYTES_SENT_TO_DECODER_MAX, numBytesForCodec);
      PLF_COUNT_MIN(BYTES_SENT_TO_DECODER_MIN, numBytesForCodec);
    }
  }
}

int Intercom_Incoming::_receive(int8_t *rxData, int rxDataLength)
{
  int freeSpace;

  if (rxDataLength > 0) {
    freeSpace = _circularBuf.freeSpace();
    if (freeSpace < rxDataLength) {
      PLF_COUNT_EVENT(CIRCULAR_BUF_OFL);
    }
    else {
      _circularBuf.write((uint8_t*)rxData, rxDataLength);
    }
  }

  return 0;
}

Intercom_Incoming::Intercom_Incoming(Intercom_MessageHandler& messageHandler) :
  _circularBuf(_circularBuffer, CIRCULAR_BUFFER_SIZE), _messageHandler(messageHandler) {

  PLF_COUNT_MIN_INIT(BYTES_SENT_TO_DECODER_MIN);
  PLF_COUNT_MIN_INIT(CIRCULAR_BUF_MIN);

  _messageHandler.registerHandler(VOICE_DATA_T_MSG_ID, messageHandlerHelper, this, true);
}
