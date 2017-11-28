#include "intercom_outgoing.h"
#include "plf_event_counter.h"
#include "vs1063a_codec.h"
#include "messages.h"

Intercom_Outgoing::Intercom_Outgoing(Intercom_MessageHandler& messageHandler) : _messageHandler(messageHandler) {

}

void Intercom_Outgoing::transfer(uint32_t buddyId) {
  int numEncodedBytes;
  static voice_data_t voice_data;
  int recordedNumBytes;
  uint32_t myId = _messageHandler.getMyId();

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
      return;
  }

  PLF_COUNT_VAL(ENCODER_AVL_BYTES, recordedNumBytes);
  PLF_COUNT_MAX(ENCODER_AVL_BYTES_MAX, recordedNumBytes);
  PLF_COUNT_MIN(ENCODER_AVL_BYTES_MIN, recordedNumBytes);

  voice_data.data_size = recordedNumBytes;
  voice_data.source_id = myId;
  voice_data.destination_id = buddyId;

  numEncodedBytes = voice_data_t_encode(intercom_message.data, 
    0, sizeof(intercom_message.data), &voice_data);

  if (_messageHandler.send(intercom_message, VOICE_DATA_T_MSG_ID, 
    numEncodedBytes, true)) {
    PLF_PRINT(PRNTGRP_DFLT, ("Voice data send failed\n"));
    return;
  }
}