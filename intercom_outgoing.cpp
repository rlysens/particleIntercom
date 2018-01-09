#include "intercom_outgoing.h"
#include "plf_event_counter.h"
#include "vs1063a_codec.h"
#include "messages.h"

#define MODULE_ID 700

#define INTERCOM_OUTGOING_FSM_IDLE 0
#define INTERCOM_OUTGOING_FSM_RECORDING 1

#define MIN_BYTES_TO_SEND (4096+1024)

Intercom_Outgoing::Intercom_Outgoing(Intercom_MessageHandler& messageHandler) : _messageHandler(messageHandler), 
  _fsmState(INTERCOM_OUTGOING_FSM_IDLE), _numBytesSentAcc(0), _seqNumber(0) {
    memset(_recordRequests, 0, sizeof(_recordRequests));
}

void Intercom_Outgoing::_fsmUpdate(void) {
  int ii;

  switch (_fsmState) {
    case INTERCOM_OUTGOING_FSM_IDLE:
      for (ii=0; ii<INTERCOM_OUTGOING_NUM_REQ_IDS; ++ii) {
        if (_recordRequests[ii]) {
          _fsmState = INTERCOM_OUTGOING_FSM_RECORDING;
          _numBytesSentAcc = 0;
          PLF_PRINT(PRNTGRP_DFLT, "Intercom_Outgoing FSM Idle->Recording requested by %d\n", ii);
          break;
        }
      }
      break;

    case INTERCOM_OUTGOING_FSM_RECORDING:
      if (_numBytesSentAcc >= MIN_BYTES_TO_SEND) {
        bool recordRequested = false;
        for (ii=0; ii<INTERCOM_OUTGOING_NUM_REQ_IDS; ++ii) {
          if (_recordRequests[ii]) {
            recordRequested = true;
            break;
          }
        }

        if (!recordRequested) {
          _fsmState = INTERCOM_OUTGOING_FSM_IDLE;
          PLF_PRINT(PRNTGRP_DFLT, "Intercom_Outgoing FSM Recording->Idle\n");
        }
      }
      break;

    default:
      break;
  }
}

void Intercom_Outgoing::recordRequest(unsigned requesterId, bool enable) {
  plf_assert("requesterId out of range", requesterId < INTERCOM_OUTGOING_NUM_REQ_IDS);

  _recordRequests[requesterId] = enable;

  _fsmUpdate();
}

void Intercom_Outgoing::run(uint32_t buddyId) {
  int numEncodedBytes;
  static voice_data_t voice_data;
  int recordedNumBytes;
  uint32_t myId = _messageHandler.getMyId();

  _fsmUpdate();
  if (_fsmState == INTERCOM_OUTGOING_FSM_IDLE) {
    return;
  }

  PLF_COUNT_EVENT(INTERCOM_OUTGOING_TICK);

  PLF_COUNT_MAX(ENCODER_AUDIO_FREE_MAX, VS1063aAudioInputBufferFreeBytes());
  PLF_COUNT_MIN(ENCODER_AUDIO_FREE_MIN, VS1063aAudioInputBufferFreeBytes());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FREE_MAX, VS1063aStreamOutputBufferFreeBytes());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FREE_MIN, VS1063aStreamOutputBufferFreeBytes());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FILL_MAX, VS1063aStreamOutputBufferFillBytes());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FILL_MIN, VS1063aStreamOutputBufferFillBytes());

  if (VS1063aStreamOutputBufferFillBytes() < (int)(sizeof(voice_data.data)))
    return;

  recordedNumBytes = VS1063RecordBuf((uint8_t*)voice_data.data, sizeof(voice_data.data));
  if (recordedNumBytes==0) {
      PLF_COUNT_EVENT(NO_ENCODER_AVL_BYTES);
  }

  PLF_COUNT_VAL(ENCODER_AVL_BYTES, recordedNumBytes);
  PLF_COUNT_MAX(ENCODER_AVL_BYTES_MAX, recordedNumBytes);
  PLF_COUNT_MIN(ENCODER_AVL_BYTES_MIN, recordedNumBytes);

  voice_data.data_size = recordedNumBytes;
  voice_data.source_id = myId;
  voice_data.destination_id = buddyId;
  voice_data.seq_number = _seqNumber;

  _seqNumber += recordedNumBytes;

  numEncodedBytes = voice_data_t_encode(intercom_message.data, 
    0, sizeof(intercom_message.data), &voice_data);

  if (_messageHandler.send(intercom_message, VOICE_DATA_T_MSG_ID, 
    numEncodedBytes, true)) {
    PLF_PRINT(PRNTGRP_DFLT, ("Voice data send failed\n"));
    return;
  }

  //PLF_PRINT(PRNTGRP_DFLT, "O:%d\n", recordedNumBytes);

  _numBytesSentAcc += recordedNumBytes;
}