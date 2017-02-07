#include "intercom_outgoing.h"
#include "vs1063a_codec.h"

#define INTERCOM_OUTGOING_TIMER_INTERVAL_MS 40
#define UDP_PACKET_MAX_DATA_LENGTH (1024)

#include "plf_event_counter.h"

void Intercom_Outgoing::_onTimeout(void) {
  static uint8_t tx_data[UDP_PACKET_MAX_DATA_LENGTH];
  uint16_t available_data_length;

  PLF_COUNT_VAL(ENCODER_AUDIO_FREE, VS1063aAudioInputBufferFreeWords());
  PLF_COUNT_VAL(ENCODER_OUTBUF_FREE, VS1063aStreamOutputBufferFreeWords());

  available_data_length = VS1063RecordBuf((uint8_t*)tx_data, sizeof(tx_data)/sizeof(tx_data[0]));
  if (available_data_length==0) {
      PLF_COUNT_EVENT(NO_ENCODER_AVL_BYTES);
      return;
  }

  PLF_COUNT_VAL(ENCODER_AVL_BYTES,available_data_length);

  /* Send the UDP packet */
  if (_udp.sendPacket(tx_data, available_data_length, _remote_ip_address, _remote_port) != available_data_length)
  {
      PLF_PRINT(("UDP packet send failed. Could not send all data\n"));
      return;
  }
  else
  {
      PLF_COUNT_VAL(UDP_BYTES_TX, available_data_length);
  }
}

/*RL port ongoing*/
Intercom_Outgoing::Intercom_Outgoing(IPAddress remote_ip_address, int remote_port, UDP& udp) :
  _remote_ip_address(remote_ip_address), _remote_port(remote_port),
  _timer(INTERCOM_OUTGOING_TIMER_INTERVAL_MS, &Intercom_Outgoing::_onTimeout, *this), _udp(udp) {

    if (!_udp.setBuffer(UDP_PACKET_MAX_DATA_LENGTH)) {
      PLF_PRINT("Couldn't allocate outgoing packet buffer\n");
    }

    _timer.start();
    PLF_PRINT("Sending to the remote every %d ms ...\n", INTERCOM_OUTGOING_TIMER_INTERVAL_MS);
}
