#include "Particle.h"
#include "intercom_incoming.h"
#include "vs1063a_codec.h"
#include "plf_utils.h"
#include "plf_event_counter.h"
#include "messages.h"
#include "plf_data_dump.h"
#include "plf_registry.h"

#define MODULE_ID 500

#define RATE_TUNE_LIMIT 60000
#define INTERCOM_INCOMING_TICK_INTER_MS 100

#define INCOMING_FSM_STATE_LISTENING 0
#define INCOMING_FSM_STATE_BUFFERING 1
#define INCOMING_FSM_STATE_DRAINING 2
#define INCOMING_FSM_NUM_STATES (INCOMING_FSM_STATE_DRAINING+1)

int Intercom_Incoming::_fsmUpdate(void) {
  int usedSpace = _circularBuf.usedSpace();

  switch (_fsmState) {
    case INCOMING_FSM_STATE_LISTENING:
      if (usedSpace > INTERCOM_INCOMING_BUFFER_DRAIN_THRESHOLD) {
        _volumeControl.enableVol(true);
        _movingAvg = INTERCOM_INCOMING_BUFFER_DRAIN_THRESHOLD;
        _fsmState = INCOMING_FSM_STATE_DRAINING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> DRAINING.\n");
      }
      else if (usedSpace > 0) {
        _volumeControl.enableVol(true);
        _fsmState = INCOMING_FSM_STATE_BUFFERING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> BUFFERING.\n");
      }

      break;

    case INCOMING_FSM_STATE_BUFFERING:
      if (usedSpace > INTERCOM_INCOMING_BUFFER_DRAIN_THRESHOLD) {
        _movingAvg = INTERCOM_INCOMING_BUFFER_DRAIN_THRESHOLD;
        _fsmState = INCOMING_FSM_STATE_DRAINING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> DRAINING.\n");
      }
      else if (usedSpace == 0) {
        _volumeControl.enableVol(false);
        _fsmState = INCOMING_FSM_STATE_LISTENING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> LISTENING.\n");
      }

      break;

    case INCOMING_FSM_STATE_DRAINING:
      if (usedSpace == 0) {
        _volumeControl.enableVol(false);
        _fsmState = INCOMING_FSM_STATE_LISTENING;
        PLF_PRINT(PRNTGRP_DFLT, "Intercom_Incoming FSM -> LISTENING.\n");
      }

      break;
  }

  return _fsmState;
}

int Intercom_Incoming::_setServerAddr(int key) {
  int value;
  bool valid;

  plf_registry.getInt(key, value, valid);

  if (valid) {
    IPAddress address = IPAddress(value);
    _myServerAddress = address;
  }

  return 0;
}

void Intercom_Incoming::_sendRetransmitReq(uint32_t destinationId, uint32_t seqNumber) {
  if (_myServerAddress) {
    int numEncodedBytes;
    retransmit_req_t retransmit_req;
    uint32_t myId = _messageHandler.getMyId();

    retransmit_req.source_id = myId;
    retransmit_req.destination_id = destinationId;
    retransmit_req.seq_number = seqNumber;

    numEncodedBytes = retransmit_req_t_encode(intercom_message.data, 0, 
      sizeof(intercom_message.data), &retransmit_req);
    plf_assert("Msg Encode Error", numEncodedBytes>=0);

    _messageHandler.send(intercom_message, RETRANSMIT_REQ_T_MSG_ID, destinationId, 
      numEncodedBytes, _myServerAddress);
  }
}

int Intercom_Incoming::_rxRetransmitMsg(Intercom_Message &msg, int payloadSize) {
  static retransmit_t retransmit;
  int retCode = 0;
  int numDecodedMsgBytes = retransmit_t_decode(msg.data, 0, payloadSize, &retransmit);
  uint32_t senderId = retransmit.source_id;

  /*Only accept this message if not already in sending state or if it's from the
   *currently active sender*/
  if ((_fsmState == INCOMING_FSM_STATE_LISTENING) || (senderId == _activeSender)) {
    if (numDecodedMsgBytes < 0) {
      PLF_COUNT_EVENT(RETRANSMIT_RX_FAIL);
      retCode = -(MODULE_ID+1);
    }
    else {
      int offset = _seqNumber - retransmit.seq_number;
      /*Don't go further back than filling level of the buffer*/
      if (offset < _circularBuf.usedSpace()) {
        /*Write it at the correct offset in the circular buffer*/
        _circularBuf.writeAt((uint8_t*)retransmit.data, retransmit.data_size, offset);
        PLF_COUNT_EVENT(RETRANSMIT_RX_SUCCESS);
      }
      else {
        PLF_COUNT_EVENT(RETRANSMIT_RX_FAIL);
      }
    }
  }
  else {
    PLF_COUNT_EVENT(RETRANSMIT_RX_FAIL);
  }

  return retCode;
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
      if (_seqNumber != (uint32_t)voiceData.seq_number) {
        uint32_t bytesMissed = voiceData.seq_number - _seqNumber;

        _sendRetransmitReq(senderId, _seqNumber);

        PLF_COUNT_VAL(BYTES_MISSED, bytesMissed);
        PLF_PRINT(PRNTGRP_DFLT, "Missed seq# %d, Rx: %d, Missing: %d\n", (int)_seqNumber, (int)voiceData.seq_number, 
          (int)(voiceData.seq_number - _seqNumber));

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

int Intercom_Incoming::_handleMessage(Intercom_Message &msg, int payloadSize) {
  int retCode = 0;

  switch (msg.msgId) {
    case VOICE_DATA_T_MSG_ID:
      retCode = _rxVoiceDataMsg(msg, payloadSize);
      break;

    case RETRANSMIT_T_MSG_ID:
      retCode = _rxRetransmitMsg(msg, payloadSize);
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

bool Intercom_Incoming::isSenderActive(uint32_t senderId) {
  return (senderId == _activeSender) && (_fsmState != INCOMING_FSM_STATE_LISTENING);
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
  if (_rateTuningEnable && (_fsmState == INCOMING_FSM_STATE_DRAINING)) {
    const float alpha = 0.1F;
    int newValue = _circularBuf.usedSpace();

    _movingAvg = (int)(alpha*newValue + ((float)1-alpha)*_movingAvg);

    int32_t error = _movingAvg - (CIRCULAR_BUFFER_SIZE/2); /*If getting too full...*/
    int32_t rateTuneValue = (10*RATE_TUNE_LIMIT*error/(CIRCULAR_BUFFER_SIZE/2)); /*...speed up the clock.*/
    rateTuneValue = MIN(RATE_TUNE_LIMIT, rateTuneValue); /*Max 4% deviation from midpoint*/
    rateTuneValue = MAX(-RATE_TUNE_LIMIT, rateTuneValue);
    _rateTuneValue = rateTuneValue;
    WriteVS10xxMem32(PAR_RATE_TUNE, (uint32_t)rateTuneValue);
    PLF_PRINT(PRNTGRP_RATETN,"a:%d c:%d e:%d r:%d\n", (int)_movingAvg, newValue, (int)error, (int)rateTuneValue);
  }
  else {
    _rateTuneValue = 0;
    WriteVS10xxMem32(PAR_RATE_TUNE, 0);
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

Intercom_Incoming::Intercom_Incoming(Intercom_MessageHandler& messageHandler, Intercom_VolumeControl& volumeControl) :
  Plf_TickerBase(INTERCOM_INCOMING_TICK_INTER_MS),
  _circularBuf(_circularBuffer, CIRCULAR_BUFFER_SIZE), _messageHandler(messageHandler),
  _volumeControl(volumeControl),
  /*_drainState(DRAIN_STATE_FILL),*/ _discardNextByte(0), _fsmState(INCOMING_FSM_STATE_LISTENING), _movingAvg(0),
  _activeSender(ID_UNKNOWN), _seqNumber(0), _rateTuneValue(0), _rateTuningEnable(false) {

  PLF_COUNT_MIN_INIT(BYTES_SENT_TO_DECODER_MIN);
  PLF_COUNT_MIN_INIT(CIRCULAR_BUF_MIN);

  _messageHandler.registerHandler(RETRANSMIT_T_MSG_ID, &Intercom_Incoming::_handleMessage, this, true);
  _messageHandler.registerHandler(VOICE_DATA_T_MSG_ID, &Intercom_Incoming::_handleMessage, this, true);
  _messageHandler.registerHandler(COMM_START_T_MSG_ID, &Intercom_Incoming::_handleMessage, this, true);
  _messageHandler.registerHandler(COMM_STOP_T_MSG_ID, &Intercom_Incoming::_handleMessage, this, true);

  PLF_REGISTRY_REGISTER_HANDLER(REG_KEY_SRVR_ADDR, &Intercom_Incoming::_setServerAddr, this);

  dataDump.registerFunction("Incoming", &Intercom_Incoming::_dataDump, this);
}

void Intercom_Incoming::_dataDump(void) {
  const char* fsmStateStrings[INCOMING_FSM_NUM_STATES] = {"Listening", "Buffering", "Draining"};

  PLF_PRINT(PRNTGRP_DFLT, "FSMstate: %s", fsmStateStrings[_fsmState]);
  PLF_PRINT(PRNTGRP_DFLT, "BufferFillingLevelAvg: %d/%d", (int)_movingAvg, CIRCULAR_BUFFER_SIZE);
  PLF_PRINT(PRNTGRP_DFLT, "ActiveSender: %d", (int)_activeSender);
  PLF_PRINT(PRNTGRP_DFLT, "RateTuningEnabled: %d", (int)_rateTuningEnable);
  PLF_PRINT(PRNTGRP_DFLT, "RateTuningValue: %d", (int)_rateTuneValue);
}