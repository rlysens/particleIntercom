#include "Particle.h"
#include "plf_utils.h"
#include "vs1063a_spi.h"

/*60MHz/64*/
int SPI_CLOCK_SPEED_SLOW_HZ = 937500;
/*60MHz/16*/
int SPI_CLOCK_SPEED_FAST_HZ = 7500000;

/*RL port complete*/
int  WriteSdi(const  uint8_t  *data,  uint32_t  bytes)
{
    uint8_t ii;

    plf_assert("too many bytes\n", bytes <= 32);
    plf_assert("Uneven bytes\n", (bytes&1)==0);

    while  (digitalRead(INTERCOM_CODEC_DREQ)  ==  LOW); //  Wait  until  DREQ  is  high
    digitalWrite(INTERCOM_CODEC_XDCS, LOW); //  Activate  xDCS

    for (ii=0; ii<bytes; ii++) {
        SPI.transfer(*data++);
    }

    digitalWrite(INTERCOM_CODEC_XDCS, HIGH); //  Dectivate  xDCS

    return 0; // Ok
}

/*RL port complete*/
void  WriteSci(uint8_t  addr,  uint16_t  data)  {
    while  (digitalRead(INTERCOM_CODEC_DREQ)  ==  LOW); //  Wait  until  DREQ  is  high
    digitalWrite(INTERCOM_CODEC_XCS, LOW); //  Activate  xCS
    SPI.transfer(2); // Write command code
    SPI.transfer(addr); // SCI register number
    SPI.transfer((uint8_t)(data >> 8));
    SPI.transfer((uint8_t)(data & 0xFF));
    digitalWrite(INTERCOM_CODEC_XCS, HIGH); //  Deactivate  xCS
}

/*RL port complete*/
uint16_t  ReadSci(uint8_t  addr)  {
    uint16_t  res;
    while  (digitalRead(INTERCOM_CODEC_DREQ)  ==  LOW); //  Wait  until  DREQ  is  high
    digitalWrite(INTERCOM_CODEC_XCS, LOW); //  Activate  xCS
    SPI.transfer(3); // Read command code
    SPI.transfer(addr); // SCI register number
    res = (uint16_t)SPI.transfer(0) << 8;
    res |= SPI.transfer(0);
    digitalWrite(INTERCOM_CODEC_XCS, HIGH); //  Deactivate  xCS
    return  res;
}

/*RL: Port complete*/
static void SpiInit( int slow )
{
    SPI.setBitOrder(MSBFIRST);

    /* Configure mode CPHA and CPOL */
    SPI.setDataMode(SPI_MODE1);

    /* Enable SPI peripheral clock */
    SPI.setClockSpeed(slow ? SPI_CLOCK_SPEED_SLOW_HZ : SPI_CLOCK_SPEED_FAST_HZ);

    /* Init and enable SPI */
    SPI.begin(SPI_MODE_MASTER, INTERCOM_CODEC_XCS);
}

/*RL: Port complete*/
void  VS1063InitSPI(void)
{
    SpiInit(1 /*=slow*/);

    pinMode(INTERCOM_CODEC_XRESET, OUTPUT);
    pinMode(INTERCOM_CODEC_XCS, OUTPUT);
    pinMode(INTERCOM_CODEC_XDCS, OUTPUT);
    pinMode(INTERCOM_CODEC_XRESET, OUTPUT);
    pinMode(INTERCOM_CODEC_DREQ, INPUT);

    digitalWrite(INTERCOM_CODEC_XRESET, LOW); //  Reset  VS10xx
    digitalWrite(INTERCOM_CODEC_XCS, HIGH); //  VS10xx  xCS  high
    digitalWrite(INTERCOM_CODEC_XDCS, HIGH); //  VS10xx  xDCS  high
    digitalWrite(INTERCOM_CODEC_XRESET, HIGH); //  VS10xx  out  of  reset
}

/*RL: Port complete*/
void VS1063AdjustSPIFast(void)  {
    SpiInit(0 /*=fast*/);
}
