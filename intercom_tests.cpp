#include "intercom_tests.h"
#include "message_handler.h"
#include "intercom_outgoing.h"
#include "intercom_incoming.h"
#include "intercom_controller.h"
#include "vs1063a_codec.h"
#include "vs1063a_spi.h"
#include "plf_utils.h"

#if 0
#include "platform.h"
#include "platform_peripheral.h"
#include "plf_utils.h"
#endif

/*RL port complete*/
void test1(void) {
  VS1063InitSPI();
  digitalWrite(INTERCOM_CODEC_XRESET, LOW); //  Reset  VS10xx
  delay( 10 );
  digitalWrite(INTERCOM_CODEC_XRESET, HIGH); //Using Logic Analyzer, check DREQ signal
  delay( 10 );
}

/*RL port complete*/
void test2(void) {
  VS1063InitSPI();
  WriteSci(SCI_MODE,  SM_RESET);
  delay( 10 ); //Using Logic Analyzer, check DREQ signal
}

/*RL port complete*/
void test3_loop(void) {
  WriteSci(SCI_VOL, 0);
  delay(500);
  WriteSci(SCI_VOL, 0xffff);
  delay( 500 );
  PLF_PRINT(".");
}

/*RL port complete*/
void test4_loop(void) {
  uint16_t res;
  static uint16_t count=0;

  WriteSci(SCI_VOL,  0xA2F5+count);
  delay( 500 );
  res = ReadSci(SCI_VOL);
  PLF_PRINT("Vol=0x%x, count=%u\n", res, (unsigned int)count++);
  //delay( 500 );
}

/*RL port complete*/
void test5_loop(void) {
  uint8_t activateData[] = {0x53, 0xEF, 0x6E, 0x44, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};
  uint8_t deactivateData[] = {0x45, 0x78, 0x69, 0x74, 0, 0, 0, 0};

  WriteSdi(activateData, sizeof(activateData));
  delay( 500 );
  WriteSdi(deactivateData, sizeof(deactivateData));
  delay( 500 );
  PLF_PRINT(".");
}

/* RL port complete */
#undef TEST6_ENABLE /*define to enable this test.*/
void test6(void) {
#ifdef TEST6_ENABLE
    static int counter=0;
    int energy=0;
    int ii;

#define SAMPLE_BUFFER_SZ (2048*16)
    static uint8_t sampleBuffer[SAMPLE_BUFFER_SZ] = {0};
    int numRxSamples = SAMPLE_BUFFER_SZ;

    VS1063RecordBufFull(sampleBuffer, numRxSamples);

    VS1063PlayBuf(sampleBuffer, numRxSamples);

    for (ii=0; ii<numRxSamples; ii++)
    {
      energy += sampleBuffer[ii]*sampleBuffer[ii];
    }

    PLF_PRINT("%d, energy=%d\n", counter++, energy);
#endif /*TEST6_ENABLE*/
}

/* RL port complete */
void test7(void) {
    while (1) {
      PLF_PRINT("%d\n", (int)digitalRead(INTERCOM_CODEC_DREQ));
    }
}

static Intercom_Incoming *intercom_incomingp=0;
static Intercom_Outgoing *intercom_outgoingp=0;
static Message_Handler *message_handlerp=0;

void test8_setup(void) {
  static IPAddress localIP = WiFi.localIP();
  static Message_Handler message_handler(50007 /*local_port*/,
    IPAddress(52,26,112,44) /*remote_ip*/, 50007 /*remote port*/);
  static Intercom_Incoming intercom_incoming(message_handler);
  static Intercom_Outgoing intercom_outgoing(message_handler);
  static Intercom_Controller intercom_controller(message_handler, intercom_outgoing);

  intercom_incomingp = &intercom_incoming;
  intercom_outgoingp = &intercom_outgoing;
  message_handlerp = &message_handler;
}

static bool recordButtonPressed(void) {
  return (digitalRead(D0)==HIGH);
}

void test8_loop(void) {
  if (message_handlerp) {
    message_handlerp->receive();
  }

  if (intercom_incomingp) {
    intercom_incomingp->drain();
  }

  if ((intercom_outgoingp) && recordButtonPressed()) {
    intercom_outgoingp->transfer();
  }
}

#if 0
void test9(void)
{
    VS1063RecordInit();
    ICOM_PRINT( ("VS1063RecordInit done\n") );

    while (1)
    {
        /*Decode Path*/
        int temp = VS1063aStreamBufferFillWords();
        ICOM_PRINT( ("VS1063aStreamBufferFillWords %d\n",temp));
        temp = VS1063aStreamBufferFreeWords();
        ICOM_PRINT( ("VS1063aStreamBufferFreeWords %d\n", temp));
        temp = VS1063aAudioBufferFillWords();
        ICOM_PRINT( ("VS1063aAudioBufferFillWords %d\n", temp));
        temp = VS1063aAudioBufferFreeWords();
        ICOM_PRINT( ("VS1063aAudioBufferFreeWords %d\n", temp));
        temp = VS1063aAudioBufferUnderflow();
        WPRINT_APP_INFO( ("VS1063aAudioBufferUnderflow %d\n", temp));

        /*Encode Path*/
        ICOM_PRINT( ("VS1063aStreamOutputBufferFillWords %d\n", VS1063aStreamOutputBufferFillWords()));
        ICOM_PRINT( ("VS1063aStreamOutputBufferFreeWords %d\n", VS1063aStreamOutputBufferFreeWords()));
        ICOM_PRINT( ("VS1063aAudioInputBufferFillWords %d\n", VS1063aAudioInputBufferFillWords()));
        ICOM_PRINT( ("VS1063aAudioInputBufferFreeWords %d\n", VS1063aAudioInputBufferFreeWords()));
    }
}
#endif
