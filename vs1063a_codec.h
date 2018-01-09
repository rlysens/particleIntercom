#ifndef VS1063A_CODEC_H
#define VS1063A_CODEC_H

#include "Particle.h"
#include "vs10xx_uc.h"

#define MAX_VOL 255

uint32_t ReadVS10xxMem32Counter(uint16_t addr);
uint32_t ReadVS10xxMem32(uint16_t addr);
uint16_t ReadVS10xxMem(uint16_t addr);
void WriteVS10xxMem(uint16_t addr, uint16_t data);
void WriteVS10xxMem32(uint16_t addr, uint32_t data);

void VS1063PlayBuf(uint8_t *bufP, uint32_t bytesInBuffer);
void VS1063PlayCancel(void);
void VS1063SetVol(uint32_t vol); /*0=MAX, 255=MIN*/
void VS1063RecordInit(void);

/*Returns number of bytes recorder*/
int VS1063RecordBuf(uint8_t *recBuf, uint32_t recBufferSize);
void VS1063RecordBufFull(uint8_t *recBuf, uint32_t recBufferSize);
void VS1063RecordCancel(void);
void VS1063InitHardware(void);
void VS1063InitSoftware(void);

void VS1063PrintState(void);

#if 0
/*A positive value increase the clock speed. A negative value decreases it. Returns new clock value.*/
uint16_t VS1063AdjustClock(int val);
#endif /*0*/

/*Decode Path*/
int VS1063aStreamBufferFillWords(void);
int VS1063aStreamBufferFreeWords(void);
int VS1063aStreamBufferFreeWordsAlt(void);
int VS1063aAudioBufferFillWords(void);
int VS1063aAudioBufferFreeWords(void);

#define VS1063aStreamBufferFillBytes() (VS1063aStreamBufferFillWords()*2)
#define VS1063aStreamBufferFreeBytes() (VS1063aStreamBufferFreeWords()*2)
#define VS1063aStreamBufferFreeBytesAlt() (VS1063aStreamBufferFreeWordsAlt()*2)
#define VS1063aAudioBufferFillBytes() (VS1063aAudioBufferFillWords()*2)
#define VS1063aAudioBufferFreeBytes() (VS1063aAudioBufferFreeBytes()*2)

/*
   Function returns number of audio buffer underflow samples since function was last called.
   Note that as the value is only 16 bits, it may overflow.
   Before calling this function for the first time, you have to do the following:
   1) Load and start the VS1063a Patches set 1.32 or newer, from
      http://www.vlsi.fi/en/support/software/vs10xxpatches.html
   2) After the Patches set has started, write 0x60 to SCI_AIADDR
*/

int16_t VS1063aAudioBufferUnderflow(void);

/*Encode Path*/
#define VS1063aStreamOutputBufferFillWords() (VS1063aStreamOutputBuffer(0))
#define VS1063aStreamOutputBufferFreeWords() (VS1063aStreamOutputBuffer(1))
#define VS1063aAudioInputBufferFillWords() (VS1063aAudioInputBuffer(0))
#define VS1063aAudioInputBufferFreeWords() (VS1063aAudioInputBuffer(1))

#define VS1063aStreamOutputBufferFillBytes() (VS1063aStreamOutputBuffer(0)*2)
#define VS1063aStreamOutputBufferFreeBytes() (VS1063aStreamOutputBuffer(1)*2)
#define VS1063aAudioInputBufferFillBytes() (VS1063aAudioInputBuffer(0)*2)
#define VS1063aAudioInputBufferFreeBytes() (VS1063aAudioInputBuffer(1)*2)

/* Support functions. Do not call directly. */
int VS1063aStreamOutputBuffer(uint16_t invert);
int VS1063aAudioInputBuffer(uint16_t invert);
#endif /*VS1063A_CODEC_H*/
