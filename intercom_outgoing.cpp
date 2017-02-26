#include "intercom_outgoing.h"
#include "vs1063a_codec.h"
#include "plf_event_counter.h"
#include "messages.h"

void Intercom_Outgoing::transfer(void) {
  static Message msg;
  uint16_t available_data_length;

  PLF_COUNT_EVENT(INTERCOM_OUTGOING_TICK);

  PLF_COUNT_MAX(ENCODER_AUDIO_FREE_MAX, VS1063aAudioInputBufferFreeWords());
  PLF_COUNT_MIN(ENCODER_AUDIO_FREE_MIN, VS1063aAudioInputBufferFreeWords());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FREE_MAX, VS1063aStreamOutputBufferFreeWords());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FREE_MIN, VS1063aStreamOutputBufferFreeWords());

  PLF_COUNT_MAX(ENCODER_OUTBUF_FILL_MAX, VS1063aStreamOutputBufferFillWords());
  PLF_COUNT_MIN(ENCODER_OUTBUF_FILL_MIN, VS1063aStreamOutputBufferFillWords());

  if (VS1063aStreamOutputBufferFillWords() < (int)(sizeof(msg.data)))
    return;

  available_data_length = VS1063RecordBuf(msg.data, sizeof(msg.data));
  if (available_data_length==0) {
      PLF_COUNT_EVENT(NO_ENCODER_AVL_BYTES);
      return;
  }

  PLF_COUNT_VAL(ENCODER_AVL_BYTES,available_data_length);
  PLF_COUNT_MAX(ENCODER_AVL_BYTES_MAX, available_data_length);
  PLF_COUNT_MIN(ENCODER_AVL_BYTES_MIN, available_data_length);

  /* Send the data */
  msg.id = MSG_ID_VOICE_DATA;
  if (_message_handler.send(msg, available_data_length)) {
      PLF_PRINT(("Voice data send failed\n"));
      return;
  }
  else {
      PLF_COUNT_VAL(UDP_BYTES_TX, available_data_length);
  }
}

/*RL port ongoing*/
Intercom_Outgoing::Intercom_Outgoing(Message_Handler& message_handler) :
  _message_handler(message_handler) {

    PLF_COUNT_MIN_INIT(ENCODER_AUDIO_FREE_MIN);
    PLF_COUNT_MIN_INIT(ENCODER_OUTBUF_FREE_MIN);
    PLF_COUNT_MIN_INIT(ENCODER_AVL_BYTES_MIN);
}
