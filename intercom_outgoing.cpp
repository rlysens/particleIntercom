#include "intercom_outgoing.h"
#include "plf_event_counter.h"
#include "vs1063a_codec.h"
#include "messages.h"
#include "plf_data_dump.h"
#include "intercom_incoming.h"
#include "plf_utils.h"

#define MODULE_ID 700

#define INTERCOM_OUTGOING_FSM_IDLE 0
#define INTERCOM_OUTGOING_FSM_RECORDING 1

#define MIN_BYTES_TO_SEND (INTERCOM_INCOMING_BUFFER_DRAIN_THRESHOLD+1024)

Intercom_Outgoing::Intercom_Outgoing() {
  _initialized = false;
}

void Intercom_Outgoing::init(Intercom_MessageHandler& messageHandler, Intercom_Buddy intercom_buddies[NUM_BUDDIES]) {
  _messageHandlerp = &messageHandler;
  _intercom_buddiesp = intercom_buddies;
  _fsmState = INTERCOM_OUTGOING_FSM_IDLE;
  _numBytesSentAcc = 0;
  _seqNumber = 0;

  plf_assert("NULL ptr in Outgoing ctor", _intercom_buddiesp);
  dataDump.registerFunction("Outgoing", &Intercom_Outgoing::_dataDump, this);

  _initialized = true;
}

void Intercom_Outgoing::_fsmUpdate(void) {
  int ii;

  switch (_fsmState) {
    case INTERCOM_OUTGOING_FSM_IDLE:
      for (ii=0; ii<NUM_BUDDIES; ++ii) {
        if (_intercom_buddiesp[ii].outgoingCommRequested()) {
          _fsmState = INTERCOM_OUTGOING_FSM_RECORDING;
          _numBytesSentAcc = 0;
          PLF_PRINT(PRNTGRP_DFLT, "Intercom_Outgoing FSM Idle->Recording requested by buddy(Idx) %d\n", (int)ii);
        }
      }
      break;

    case INTERCOM_OUTGOING_FSM_RECORDING:
      /*The idea behind this check is to make sure that once we start sending,
       *we send enough to get the receiver into the draining state*/
      if (_numBytesSentAcc >= MIN_BYTES_TO_SEND) {
        bool recordRequested = false;
        for (ii=0; ii<NUM_BUDDIES; ++ii) {
          if (_intercom_buddiesp[ii].outgoingCommRequested()) {
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

void Intercom_Outgoing::run(void) {
  int numEncodedBytes;
  static voice_data_t voice_data;
  int recordedNumBytes;
  uint32_t myId = _messageHandlerp->getMyId();

  plf_assert("Outgoing not initialized", _initialized);
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
  voice_data.seq_number = _seqNumber;

  _seqNumber += recordedNumBytes;

  uint32_t buddyId;
  int ii;

  for (ii=0; ii<NUM_BUDDIES;++ii) {
    if (_intercom_buddiesp[ii].outgoingCommRequested()) {
      voice_data.destination_id = _intercom_buddiesp[ii].getBuddyId();

      numEncodedBytes = voice_data_t_encode(intercom_message.data, 
        0, sizeof(intercom_message.data), &voice_data);

      if (_messageHandlerp->send(intercom_message, VOICE_DATA_T_MSG_ID, 
        numEncodedBytes, true)) {
        PLF_PRINT(PRNTGRP_DFLT, "Voice data send failed.\n");
      }
    }
  }

  _numBytesSentAcc += recordedNumBytes;
}

void Intercom_Outgoing::_dataDump(void) {
  int ii;
  PLF_PRINT(PRNTGRP_DFLT, "FSMstate: %s", _fsmState == INTERCOM_OUTGOING_FSM_IDLE ? "Idle" : "Recording");
  PLF_PRINT(PRNTGRP_DFLT, "NumBytesSent: %d", _numBytesSentAcc);
}
