#include "intercom_outgoing.h"
#include "plf_event_counter.h"
#include "vs1063a_codec.h"
#include "messages.h"

Intercom_Outgoing::Intercom_Outgoing(Message_Handler& message_handler) : _message_handler(message_handler) {

}

void Intercom_Outgoing::transfer(uint32_t buddyId) {
  int num_encoded_bytes;
  static voice_data_t voice_data;
  int recorded_num_bytes;
  uint32_t myId = _message_handler.getMyId();

  PLF_COUNT_EVENT(INTERCOM_OUTGOING_TICK);

  PLF_COUNT_MAX(ENCODER_AUDIO_FREE_MAX, VS1063aAudioInputBufferFreeBytes());
  PLF_COUNT_MIN(ENCODER_AUDIO_FREE_MIN, VS1063aAudioInputBufferFreeBytes());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FREE_MAX, VS1063aStreamOutputBufferFreeBytes());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FREE_MIN, VS1063aStreamOutputBufferFreeBytes());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FILL_MAX, VS1063aStreamOutputBufferFillBytes());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FILL_MIN, VS1063aStreamOutputBufferFillBytes());

  if (VS1063aStreamOutputBufferFillBytes() < (int)(sizeof(voice_data.data)))
    return;

  recorded_num_bytes = VS1063RecordBuf((uint8_t*)voice_data.data, sizeof(voice_data.data));
  if (recorded_num_bytes==0) {
      PLF_COUNT_EVENT(NO_ENCODER_AVL_BYTES);
      return;
  }

  PLF_COUNT_VAL(ENCODER_AVL_BYTES, recorded_num_bytes);
  PLF_COUNT_MAX(ENCODER_AVL_BYTES_MAX, recorded_num_bytes);
  PLF_COUNT_MIN(ENCODER_AVL_BYTES_MIN, recorded_num_bytes);

  voice_data.data_size = recorded_num_bytes;
  voice_data.source_id = myId;
  voice_data.destination_id = buddyId;

  num_encoded_bytes = voice_data_t_encode(intercom_message.data, 
    0, sizeof(intercom_message.data), &voice_data);

  if (_message_handler.send(intercom_message, VOICE_DATA_T_MSG_ID, 
    num_encoded_bytes, true)) {
    PLF_PRINT(PRNTGRP_DFLT, ("Voice data send failed\n"));
    return;
  }
}