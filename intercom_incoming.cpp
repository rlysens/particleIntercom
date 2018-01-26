#include "Particle.h"
#include "intercom_incoming.h"
#include "vs1063a_codec.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "messages.h"
#include "plf_data_dump.h"

#define MODULE_ID 500

#define BUFFER_MID_POINT (CIRCULAR_BUFFER_SIZE/2)
#define BUFFER_DRAIN_THRESHOLD (CIRCULAR_BUFFER_SIZE*8/10)
#define INTERCOM_INCOMING_TICK_INTER_MS 100

#if 0
#define BUFFER_NEARLY_EMPTY (CIRCULAR_BUFFER_SIZE/8)
#define BUFFER_HALF (CIRCULAR_BUFFER_SIZE/2)
#define BUFFER_NEARLY_FULL (CIRCULAR_BUFFER_SIZE - (CIRCULAR_BUFFER_SIZE/8))

#define DRAIN_STATE_FILL 0
#define DRAIN_STATE_DRAIN 1
#endif

#define INCOMING_FSM_STATE_LISTENING 0
#define INCOMING_FSM_STATE_BUFFERING 1
#define INCOMING_FSM_STATE_DRAINING 2
#define INCOMING_FSM_NUM_STATES (INCOMING_FSM_STATE_DRAINING+1)

static int messageHandlerHelper(Intercom_Message &msg, 
  int payloadSize, void *ctxt) {
  Intercom_Incoming *intercom_incomingp = (Intercom_Incoming*)ctxt;

  plf_assert("NULL ctxt ptr", intercom_incomingp);

  return intercom_incomingp->handleMessage(msg, payloadSize);
}

int Intercom_Incoming::_fsmUpdate(void) {
  int usedSpace = _circularBuf.usedSpace();

  switch (_fsmState) {
    case INCOMING_FSM_STATE_LISTENING:
      if (usedSpace > BUFFER_DRAIN_THRESHOLD) {
        _movingAvg = BUFFER_DRAIN_THRESHOLD;
        _fsmState = INCOMING_FSM_STATE_DRAINING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> DRAINING.\n");
      }
      else if (usedSpace > 0) {
        _fsmState = INCOMING_FSM_STATE_BUFFERING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> BUFFERING.\n");
      }

      break;

    case INCOMING_FSM_STATE_BUFFERING:
      if (usedSpace > BUFFER_DRAIN_THRESHOLD) {
        _movingAvg = BUFFER_DRAIN_THRESHOLD;
        _fsmState = INCOMING_FSM_STATE_DRAINING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> DRAINING.\n");
      }
      else if (usedSpace == 0) {
        _fsmState = INCOMING_FSM_STATE_LISTENING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> LISTENING.\n");
      }

      break;

    case INCOMING_FSM_STATE_DRAINING:
      if (usedSpace == 0) {
        _fsmState = INCOMING_FSM_STATE_LISTENING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> LISTENING.\n");
      }

      break;
  }

  return _fsmState;
}

int Intercom_Incoming::_rxVoiceDataMsg(Intercom_Message &msg, int payloadSize) {
  static voice_data_t voiceData;
  int retCode = 0;
  int numDecodedMsgBytes = voice_data_t_decode(msg.data, 0, payloadSize, &voiceData);
  uint32_t senderId = voiceData.source_id;

  /*Only accept this message if not already in sending state or if it's from the
   *currently active sender*/
  if ((_fsmState == INCOMING_FSM_STATE_LISTENING) || (senderId == _activeSender)) {
    if (numDecodedMsgBytes < 0) {
      retCode = -(MODULE_ID+1);
    }
    else {
      if (_fsmState == INCOMING_FSM_STATE_LISTENING) {
        _seqNumber = voiceData.seq_number; /*Restart tracking seq. number when coming out of listening state*/
      }

      /*Did we miss anything?*/
      if (_seqNumber != voiceData.seq_number) {
        uint32_t bytesMissed = voiceData.seq_number - _seqNumber;
        PLF_COUNT_VAL(BYTES_MISSED, bytesMissed);
        /*stuff the circular buffer with the amount of bytes missed*/
        retCode |= _stuff(bytesMissed); 
      }

      retCode |= _receive(voiceData.data, voiceData.data_size);

      _activeSender = senderId;
      _seqNumber = voiceData.seq_number + voiceData.data_size;

      _fsmUpdate();
    }
  }

  return retCode;     
}

int Intercom_Incoming::handleMessage(Intercom_Message &msg, int payloadSize) {
  int retCode = 0;

  switch (msg.msgId) {
    case VOICE_DATA_T_MSG_ID:
      retCode = _rxVoiceDataMsg(msg, payloadSize);
      break;

    case COMM_START_T_MSG_ID:
      _rateTuningEnable = true;
      break;
    
    case COMM_STOP_T_MSG_ID:
      _rateTuningEnable = false;
      break;

    default:
      retCode = -(MODULE_ID+2);
      break;
  }

  return retCode;
}

void Intercom_Incoming::drain(void) {
  int numBytesBuffered = _circularBuf.usedSpace();
  int fsmState = _fsmUpdate();

  if (fsmState == INCOMING_FSM_STATE_DRAINING) {
    uint8_t *decoderData;
    int numBytesForCodec, decoderAvlSpace;

    PLF_COUNT_MAX(CIRCULAR_BUF_MAX, numBytesBuffered);
    PLF_COUNT_MIN(CIRCULAR_BUF_MIN, numBytesBuffered);

    decoderAvlSpace = VS1063aStreamBufferFreeBytes();
    numBytesForCodec = MIN(decoderAvlSpace, numBytesBuffered);

    if (numBytesForCodec && _discardNextByte) {
      _circularBuf.readStart(&decoderData, 1);
      _circularBuf.readRelease(1);
      _discardNextByte = 0;
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
        _discardNextByte=1;
      }

      _circularBuf.readRelease(numBytesForCodec);

      //PLF_PRINT(PRNTGRP_DFLT, "D:%d\n", numBytesForCodec);

      PLF_COUNT_VAL(BYTES_SENT_TO_DECODER, numBytesForCodec);
      PLF_COUNT_MAX(BYTES_SENT_TO_DECODER_MAX, numBytesForCodec);
      PLF_COUNT_MIN(BYTES_SENT_TO_DECODER_MIN, numBytesForCodec);
    }
  }
}

void Intercom_Incoming::_tickerHook(void) {
  if (/*_rateTuningEnable &&*/ (_fsmState == INCOMING_FSM_STATE_DRAINING)) {
    const float alpha = 0.1F;
    int newValue = _circularBuf.usedSpace();

    _movingAvg = (int)(alpha*newValue + ((float)1-alpha)*_movingAvg);

    int32_t error = _movingAvg - BUFFER_MID_POINT; /*If getting too full...*/
    int32_t rateTuneValue = (10*error*1000000/BUFFER_MID_POINT); /*...speed up the clock.*/
    if (_rateTuningEnable)
      WriteVS10xxMem32(PAR_RATE_TUNE, (uint32_t)rateTuneValue);
    PLF_PRINT(PRNTGRP_RATETN,"a:%d c:%d e:%d r:%d\n", _movingAvg, newValue, error, rateTuneValue);
  }
}

int Intercom_Incoming::_stuff(int rxDataLength) {
  if (rxDataLength > 0) {
    int freeSpace = _circularBuf.freeSpace();
    if (freeSpace < rxDataLength) {
      PLF_COUNT_EVENT(CIRCULAR_BUF_OFL);
    }
    else {
      _circularBuf.stuff(0, rxDataLength);
      //PLF_PRINT(PRNTGRP_DFLT, "S:%d\n", rxDataLength);
    }
  }

  return 0;
}

int Intercom_Incoming::_receive(int8_t *rxData, int rxDataLength) {
  if (rxDataLength > 0) {
    int freeSpace = _circularBuf.freeSpace();
    if (freeSpace < rxDataLength) {
      PLF_COUNT_EVENT(CIRCULAR_BUF_OFL);
    }
    else {
      _circularBuf.write((uint8_t*)rxData, rxDataLength);
      //PLF_PRINT(PRNTGRP_DFLT, "I:%d\n", rxDataLength);
    }
  }

  return 0;
}

Intercom_Incoming::Intercom_Incoming(Intercom_MessageHandler& messageHandler) :
  Plf_TickerBase(INTERCOM_INCOMING_TICK_INTER_MS),
  _circularBuf(_circularBuffer, CIRCULAR_BUFFER_SIZE), _messageHandler(messageHandler),
  /*_drainState(DRAIN_STATE_FILL),*/ _discardNextByte(0), _fsmState(INCOMING_FSM_STATE_LISTENING), _movingAvg(0),
  _activeSender(ID_UNKNOWN), _seqNumber(0), _rateTuningEnable(false) {

  PLF_COUNT_MIN_INIT(BYTES_SENT_TO_DECODER_MIN);
  PLF_COUNT_MIN_INIT(CIRCULAR_BUF_MIN);

  _messageHandler.registerHandler(VOICE_DATA_T_MSG_ID, messageHandlerHelper, this, true);
  _messageHandler.registerHandler(COMM_START_T_MSG_ID, messageHandlerHelper, this, true);
  _messageHandler.registerHandler(COMM_STOP_T_MSG_ID, messageHandlerHelper, this, true);

  dataDump.registerFunction("Incoming", &Intercom_Incoming::_dataDump, this);
}

void Intercom_Incoming::_dataDump(void) {
  const char* fsmStateStrings[INCOMING_FSM_NUM_STATES] = {"Listening", "Buffering", "Draining"};

  PLF_PRINT(PRNTGRP_DFLT, "FSMstate: %s", fsmStateStrings[_fsmState]);
  PLF_PRINT(PRNTGRP_DFLT, "BufferFillingLevelAvg: %d/%d", _movingAvg, CIRCULAR_BUFFER_SIZE);
  PLF_PRINT(PRNTGRP_DFLT, "ActiveSender: %d", (int)_activeSender);
  PLF_PRINT(PRNTGRP_DFLT, "RateTuningEnabled: %d", (int)_rateTuningEnable);
}