#ifndef INTERCOM_INCOMING_H
#define INTERCOM_INCOMING_H

#include "intercom_message_handler.h"
#include "plf_circular_buffer.h"
#include "plf_ticker_base.h"

#define CIRCULAR_BUFFER_SIZE (8192)
#define INTERCOM_INCOMING_BUFFER_DRAIN_THRESHOLD (CIRCULAR_BUFFER_SIZE/2)

class Intercom_Incoming : public Plf_TickerBase {
private:
	uint8_t _circularBuffer[CIRCULAR_BUFFER_SIZE];
	Plf_CircularBuf _circularBuf;
  Intercom_MessageHandler& _messageHandler;
  
#if 0
  int _drainState;
#endif
  int _discardNextByte;
  int _fsmState;
  int32_t _movingAvg;
  uint32_t _activeSender;
  uint32_t _seqNumber;
  int32_t _rateTuneValue;
  bool _rateTuningEnable;

  int _rxVoiceDataMsg(Intercom_Message &msg, int payloadSize);

  int _stuff(int rxDataLength);
  int _receive(int8_t *rxData, int rxDataLength);
  int _fsmUpdate(void);

  virtual void _tickerHook(void);

  void _dataDump(void);
  
  int _handleMessage(Intercom_Message &msg, int payloadSize);

public:
  Intercom_Incoming(Intercom_MessageHandler& messageHandler);

  bool isSenderActive(uint32_t senderId);
  
  void drain(void);
};

#endif /*INTERCOM_INCOMING_H*/
