#include "intercom_outgoing.h"
#include "vs1063a_codec.h"
#include "plf_event_counter.h"
#include "messages.h"

void Intercom_Outgoing::transfer(void) {
  int num_encoded_bytes;
  static Intercom_Message intercom_message;
  static voice_data_t voice_data;
  int recorded_num_bytes;

  PLF_COUNT_EVENT(INTERCOM_OUTGOING_TICK);

  PLF_COUNT_MAX(ENCODER_AUDIO_FREE_MAX, VS1063aAudioInputBufferFreeWords());
  PLF_COUNT_MIN(ENCODER_AUDIO_FREE_MIN, VS1063aAudioInputBufferFreeWords());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FREE_MAX, VS1063aStreamOutputBufferFreeWords());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FREE_MIN, VS1063aStreamOutputBufferFreeWords());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FILL_MAX, VS1063aStreamOutputBufferFillWords());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FILL_MIN, VS1063aStreamOutputBufferFillWords());

  if (VS1063aStreamOutputBufferFillWords() < (int)(sizeof(voice_data.data)))
    return;

  recorded_num_bytes = VS1063RecordBuf((uint8_t*)voice_data.data, sizeof(voice_data.data));
  if (recorded_num_bytes==0) {
      PLF_COUNT_EVENT(NO_ENCODER_AVL_BYTES);
      return;
  }

  PLF_COUNT_VAL(ENCODER_AVL_BYTES, recorded_num_bytes);
  PLF_COUNT_MAX(ENCODER_AVL_BYTES_MAX, recorded_num_bytes);
  PLF_COUNT_MIN(ENCODER_AVL_BYTES_MIN, recorded_num_bytes);

  if (_source_id_set && _destination_id_set) {
    voice_data.data_size = recorded_num_bytes;
    voice_data.source_id = _source_id;
    voice_data.destination_id = _destination_id;

    num_encoded_bytes = voice_data_t_encode(intercom_message.data, 
      0, sizeof(intercom_message.data), &voice_data);

    intercom_message.id = VOICE_DATA_T_MSG_ID;

    if (_message_handler.send(intercom_message, num_encoded_bytes)) {
      PLF_PRINT(("Voice data send failed\n"));
      return;
    }
  }
  else {
    PLF_PRINT("source_id_set %d, destination_id set %d\n", (int)_source_id_set, (int)_destination_id_set);
  }  
}

void Intercom_Outgoing::set_source_id(int16_t source_id) {
  _source_id = source_id;
  _source_id_set = true;
}

void Intercom_Outgoing::set_destination_id(int16_t destination_id) {
  _destination_id = destination_id;
  _destination_id_set = true;
}

/*RL port ongoing*/
Intercom_Outgoing::Intercom_Outgoing(Message_Handler& message_handler) :
  _message_handler(message_handler), _destination_id_set(false),
  _source_id_set(false) {

    PLF_COUNT_MIN_INIT(ENCODER_AUDIO_FREE_MIN);
    PLF_COUNT_MIN_INIT(ENCODER_OUTBUF_FREE_MIN);
    PLF_COUNT_MIN_INIT(ENCODER_AVL_BYTES_MIN);
}
