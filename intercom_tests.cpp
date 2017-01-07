#include "intercom_tests.h"
#include "vs1063a_codec.h"
#include "vs1063a_spi.h"

#if 0
#include "platform.h"
#include "platform_peripheral.h"
#include "plf_utils.h"
#include "intercom_udp_server.h"
#include "intercom_proxy.h"
#include "intercom_print.h"
#endif

void test1(void) {
  digitalWrite(INTERCOM_CODEC_XRESET, LOW); //  Reset  VS10xx
  delay( 10 );
  digitalWrite(INTERCOM_CODEC_XRESET, HIGH); //Using Logic Analyzer, check DREQ signal
  delay( 10 );
}

#if 0
void test2(void) {
    WriteSci(SCI_MODE,  SM_RESET);
    wiced_rtos_delay_milliseconds( 10 ); //Using Logic Analyzer, check DREQ signal
}

void test3(void) {
    while (1) {
        WriteSci(SCI_VOL,  0);
        wiced_rtos_delay_milliseconds( 500 );
        WriteSci(SCI_VOL,  0xffff);
        wiced_rtos_delay_milliseconds( 500 );
        ICOM_PRINT( (".\n") );
    }
}

void test4(void) {
    uint16_t res;
    uint32_t count=0;

    while (1) {
        WriteSci(SCI_VOL,  0xA2F5);
        wiced_rtos_delay_milliseconds( 500 );
        res = ReadSci(SCI_VOL);
        ICOM_PRINT( ("Vol=0x%x, count=%u\n", res, (unsigned int)count++) );
        wiced_rtos_delay_milliseconds( 500 );
    }
}

void test5(void) {
    uint8_t activateData[] = {0x53, 0xEF, 0x6E, 0x44, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};
    uint8_t deactivateData[] = {0x45, 0x78, 0x69, 0x74, 0, 0, 0, 0};

    while (1) {
        WriteSdi(activateData, sizeof(activateData)/sizeof(activateData[0]));
        wiced_rtos_delay_milliseconds( 500 );
        WriteSdi(deactivateData, sizeof(deactivateData)/sizeof(deactivateData[0]));
        wiced_rtos_delay_milliseconds( 500 );
    }
}

void test6(void) {
    int counter=0;
#define SAMPLE_BUFFER_SZ 2048
    static uint8_t sampleBuffer[SAMPLE_BUFFER_SZ] = {0};
    int numRxSamples = SAMPLE_BUFFER_SZ;

    VS1063RecordInit();
    ICOM_PRINT( ("VS1063RecordInit done\n") );

    while(1)
    {

        //wiced_rtos_delay_milliseconds( 40 );

        VS1063RecordBufFull(sampleBuffer, numRxSamples);

        VS1063PlayBuf(sampleBuffer, numRxSamples);

        if (counter++%128==0)
        {
            ICOM_PRINT(("%d, numRxSamples=%d\n", counter, numRxSamples));
        }

        numRxSamples = SAMPLE_BUFFER_SZ/2;
    }
}

void test7(void) {
    while (1) {
    ICOM_PRINT(("%d\n",wiced_gpio_input_get(INTERCOM_CODEC_DREQ)));
    }
}

void test8(void) {
    wiced_result_t result;
    static Intercom_proxy intercom_proxy;

    intercom_udp_server_start();
    ICOM_PRINT( ("udp_server started\n") );

    intercom_proxy_create(&intercom_proxy);
    ICOM_PRINT( ("intercom_proxy created\n") );

    VS1063RecordInit();
    ICOM_PRINT( ("VS1063RecordInit done\n") );

    result = intercom_proxy_connect(&intercom_proxy);
    icom_assert("intercom_proxy_connect err\n",result == WICED_SUCCESS);

    ICOM_PRINT( ("intercom_proxy connected.\n") );
}

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
