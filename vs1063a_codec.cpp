/*

  VLSI Solution generic microcontroller example player / recorder for
  VS1063.

  vA.01 2017-01-03 RL  Port to Particle.
  vA.00 2016-01-03 RL  Port to WICED.
  v1.02 2012-11-28 HH  Untabified
  v1.01 2012-11-27 HH  Bug fixes
  v1.00 2012-11-23 HH  First release

*/
#include "Particle.h"
#include "plf_utils.h"
#include "vs1063a_codec.h"

#include "vs1063a_spi.h"

/* Download the latest VS1063a Patches package and its vs1063a-patches.plg.
   The patches package is available at
   http://www.vlsi.fi/en/support/software/vs10xxpatches.html */
#include "vs1063a_patches_201.h"

#define SDI_MAX_TRANSFER_SIZE 32
#define SDI_END_FILL_BYTES_FLAC 12288
#define SDI_END_FILL_BYTES       2050

#define SPEED_SHIFT_CHANGE 128

/* How many transferred bytes between collecting data.
   A value between 1-8 KiB is typically a good value.
   If REPORT_ON_SCREEN is defined, a report is given on screen each time
   data is collected. */
#define REPORT_INTERVAL 4096

#define INIT_VOL 0x0e0e

enum AudioFormat {
  afUnknown,
  afRiff,
  afOggVorbis,
  afMp1,
  afMp2,
  afMp3,
  afAacMp4,
  afAacAdts,
  afAacAdif,
  afFlac,
  afWma,
} audioFormat = afUnknown;

const char *afName[] = {
  "unknown",
  "RIFF",
  "Ogg",
  "MP1",
  "MP2",
  "MP3",
  "AAC MP4",
  "AAC ADTS",
  "AAC ADIF",
  "FLAC",
  "WMA",
};

static int endFillByte = 0;          // What byte value to send after file

/*
  Read 32-bit increasing counter value from addr.
  Because the 32-bit value can change while reading it,
  read MSB's twice and decide which is the correct one.
*/
/* RL port complete*/
uint32_t ReadVS10xxMem32Counter(uint16_t addr) {
  uint16_t msbV1, lsb, msbV2;
  uint32_t res;

  WriteSci(SCI_WRAMADDR, addr+1);
  msbV1 = ReadSci(SCI_WRAM);
  WriteSci(SCI_WRAMADDR, addr);
  lsb = ReadSci(SCI_WRAM);
  msbV2 = ReadSci(SCI_WRAM);
  if (lsb < 0x8000U) {
    msbV1 = msbV2;
  }
  res = ((uint32_t)msbV1 << 16) | lsb;

  return res;
}

/*
  Read 32-bit non-changing value from addr.
*/
/*RL port complete*/
uint32_t ReadVS10xxMem32(uint16_t addr) {
    uint32_t readResult;
    uint16_t lsb;

    WriteSci(SCI_WRAMADDR, addr);
    lsb = ReadSci(SCI_WRAM);
    readResult = lsb | ((uint32_t)ReadSci(SCI_WRAM) << 16);

    return readResult;
}

/*
  Read 16-bit value from addr.
*/
/*RL port complete*/
uint16_t ReadVS10xxMem(uint16_t addr) {
    uint16_t readResult;

    WriteSci(SCI_WRAMADDR, addr);
    readResult = ReadSci(SCI_WRAM);

    return readResult;
}

/*
  Write 16-bit value to given VS10xx address
*/
/*RL port complete*/
void WriteVS10xxMem(uint16_t addr, uint16_t data) {
    WriteSci(SCI_WRAMADDR, addr);
    WriteSci(SCI_WRAM, data);
}

/*
  Write 32-bit value to given VS10xx address
*/
/*RL port complete*/
void WriteVS10xxMem32(uint16_t addr, uint32_t data) {
    WriteSci(SCI_WRAMADDR, addr);
    WriteSci(SCI_WRAM, (uint16_t)data);
    WriteSci(SCI_WRAM, (uint16_t)(data>>16));
}

void LoadUserCode(void) {
  int i = 0;

  while (i<sizeof(plugin)/sizeof(plugin[0])) {
    unsigned short addr, n, val;
    addr = plugin[i++];
    n = plugin[i++];
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = plugin[i++];
      while (n--) {
        WriteSci(addr, val);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = plugin[i++];
        WriteSci(addr, val);
      }
    }
  }
}

/*RL port complete*/
/* Support function. Do not call directly. */
int VS1063aStreamOutputBuffer(uint16_t invert) {
  int16_t wrp, rdp;
  int16_t /*bufStart, bufEnd,*/ bufSize;
  int16_t res;

  WriteSci(SCI_WRAMADDR, 0x5A8F);
  wrp = ReadSci(SCI_WRAM);
  rdp = ReadSci(SCI_WRAM);
  /*bufStart =*/ ReadSci(SCI_WRAM);
  /*bufEnd =*/ ReadSci(SCI_WRAM);
  bufSize = ReadSci(SCI_WRAM);
  res = wrp-rdp;
  if (res < 0) {
    res += bufSize;
  }
  if (invert) {
    res = bufSize - res - 2;
    if (res < 0) {
      res = 0;
    }
  }

  return res;
}

/* RL port complete*/
/* Support function. Do not call directly. */
int VS1063aAudioInputBuffer(uint16_t invert) {
  uint16_t wrp, rdp;
  int16_t /*bufStart, bufEnd,*/ bufSize;
  int16_t res;

  WriteSci(SCI_WRAMADDR, 0x1a70);
  wrp = ReadSci(SCI_WRAM);
  rdp = ReadSci(SCI_WRAM);
  WriteSci(SCI_WRAMADDR, 0x2033);
  /*bufStart =*/ ReadSci(SCI_WRAM);
  /*bufEnd =*/ ReadSci(SCI_WRAM);
  bufSize = ReadSci(SCI_WRAM);
  res = wrp-rdp;
  if (res < 0) {
    res += bufSize;
  }
  if (invert) {
    res = bufSize - res - 2;
    if (res < 0) {
      res = 0;
    }
  }

  return res;
}

/* RL port complete*/
int VS1063aStreamBufferFillWords(void) {
  int16_t wrp, rdp;
  /* For FLAC files, stream buffer is larger */
  int16_t bufSize = (ReadSci(SCI_HDAT1) == 0x664C) ? 0x1800 : 0x400;
  int16_t res;

  WriteSci(SCI_WRAMADDR, 0x5A7D);
  wrp = ReadSci(SCI_WRAM);
  rdp = ReadSci(SCI_WRAM);

  res = wrp-rdp;
  if (res < 0) {
    return res + bufSize;
  }
  return res;
}

/* RL port complete*/
int VS1063aStreamBufferFreeWordsAlt(void) {
  int16_t bufSize, res;

  /* For FLAC files, stream buffer is larger */
  bufSize = (ReadSci(SCI_HDAT1) == 0x664C) ? 0x1800 : 0x400;
  res = bufSize - VS1063aStreamBufferFillWords();

  if (res < 2) {
      return 0;
  }

  return res-2;
}

/* RL port complete*/
int VS1063aStreamBufferFreeWords(void) {
  int16_t res;

  WriteSci(SCI_WRAMADDR, 0xc0c0); /*Force an update of sdiFree*/
  WriteSci(SCI_WRAM, 0);
  WriteSci(SCI_WRAMADDR, 0x1e1f); /*Read sdiFree*/
  res = ReadSci(SCI_WRAM);

  return res;
}

/* RL port complete*/
int VS1063aAudioBufferFillWords(void) {
  uint16_t wrp, rdp;
  int16_t bufSize;
  int16_t res;
  WriteSci(SCI_WRAMADDR, 0x5B0F);
  wrp = ReadSci(SCI_WRAM);
  rdp = ReadSci(SCI_WRAM);
  bufSize = (ReadSci(SCI_WRAM) & 0x7FFF) - 1;
  res = wrp-rdp;
  if (res < 0) {
    return res + bufSize;
  }
  return res;
}

/* RL port complete*/
int VS1063aAudioBufferFreeWords(void) {
  int16_t bufSize;
  int16_t res;
  WriteSci(SCI_WRAMADDR, 0x5B0F+2);
  bufSize = (ReadSci(SCI_WRAM) & 0x7FFF) - 1;
  res = bufSize - VS1063aAudioBufferFillWords();
  if (res < 2) {
    return 0;
  }
  return res-2;
}

/*
   Function returns number of audio buffer underflow samples since function was last called.
   Note that as the value is only 16 bits, it may overflow.
   Before calling this function for the first time, you have to do the following:
   1) Load and start the VS1063a Patches set 1.32 or newer, from
      http://www.vlsi.fi/en/support/software/vs10xxpatches.html
   2) After the Patches set has started, write 0x60 to SCI_AIADDR
*/
/*RL port complete*/
int16_t VS1063aAudioBufferUnderflow(void) {
  static uint16_t oldUFlows = 0;
  int16_t uFlows, newUFlows;

  WriteSci(SCI_WRAMADDR, 0x5B0F+5);
  newUFlows = ReadSci(SCI_WRAM);
  uFlows = newUFlows - oldUFlows;
  oldUFlows = newUFlows;

  return uFlows;
}

#if 0
void VS1063PlayBufOrig(uint8_t *bufP, uint32_t bytesInBuffer) {
  uint32_t pos=0;                // File position
  int playMode = ReadVS10xxMem(PAR_PLAY_MODE);
  long nextReportPos=0; // File pointer where to next collect/report

  WriteSci(SCI_DECODE_TIME, 0);         // Reset DECODE_TIME

  while (bytesInBuffer) {
      if (!(playMode & PAR_PLAY_MODE_PAUSE_ENA))
      {
          int t = MIN(SDI_MAX_TRANSFER_SIZE, bytesInBuffer);

          // This is the heart of the algorithm: on the following line
          // actual audio data gets sent to VS10xx.
          WriteSdi(bufP, t);

          bufP += t;
          bytesInBuffer -= t;
          pos += t;
      }

      /* If playback is going on as normal, see if we need to collect and
         possibly report */
      if (pos >= nextReportPos) {
        nextReportPos += REPORT_INTERVAL;
        /* It is important to collect endFillByte while still in normal
           playback. If we need to later cancel playback or run into any
           trouble with e.g. a broken file, we need to be able to repeatedly
           send this byte until the decoder has been able to exit. */
        endFillByte = ReadVS10xxMem(PAR_END_FILL_BYTE);
      }
    } /* if (playerState == psPlayback && pos >= nextReportPos) */
}
#endif

/*RL port complete*/
static void VS1063PlayBufCPU(uint8_t *bufP, uint32_t bytesInBuffer) {
    while (bytesInBuffer) {
        int t = MIN(SDI_MAX_TRANSFER_SIZE, bytesInBuffer);

        // This is the heart of the algorithm: on the following line
        // actual audio data gets sent to VS10xx.
        WriteSdi(bufP, t);

        bufP += t;
        bytesInBuffer -= t;
    }
}

#if 0
static void VS1063PlayBufDMA(uint8_t *bufP, uint32_t bytesInBuffer) {
    WriteSdiDMA(bufP, bytesInBuffer);
}
#endif

/*RL port complete*/
void VS1063PlayBuf(uint8_t *bufP, uint32_t bytesInBuffer) {
    if (bytesInBuffer) {
        VS1063PlayBufCPU(bufP, bytesInBuffer);
    }
}

/*RL port complete*/
void VS1063PlayCancel(void) {
    static uint8_t playBuf[SDI_END_FILL_BYTES];
    unsigned short oldMode;
    int i;

    /* Earlier we collected endFillByte. Now, just in case the file was
       broken, or if a cancel playback command has been given, write
       lots of endFillBytes. */
    memset(playBuf, endFillByte, sizeof(playBuf));
    for (i=0; i<SDI_END_FILL_BYTES; i+=SDI_MAX_TRANSFER_SIZE)
        WriteSdi(playBuf, SDI_MAX_TRANSFER_SIZE);

    oldMode = ReadSci(SCI_MODE);
    WriteSci(SCI_MODE, oldMode | SM_CANCEL);
    while (ReadSci(SCI_MODE) & SM_CANCEL)
        WriteSdi(playBuf, 2);
}

/* RL port complete */
void VS1063SetVol(uint32_t volLevel) {
    if (volLevel > MAX_VOL)
        volLevel = MAX_VOL;

    WriteSci(SCI_VOL, volLevel*0x101);
}

/* RL port complete */
void VS1063RecordInit(void) {
    /* Initialize recording */
    WriteSci(SCI_RECRATE, 8000 /*Sample rate*/);
    WriteSci(SCI_RECGAIN, 0); /* 1024 = gain 1 = best quality */
    WriteSci(SCI_RECMAXAUTO, 8192); /* if RECGAIN = 0, define max auto gain */
    WriteSci(SCI_RECMODE, RM_63_ADC_MODE_MONO | RM_63_FORMAT_G711_ULAW | RM_63_CODEC | RM_63_NO_RIFF);
    audioFormat = afRiff;

    WriteSci(SCI_MODE, ReadSci(SCI_MODE)| SM_ENCODE);
    /*Disabling automatic sample fill is N/A in codec mode => Keeping vector at 0x50*/
    WriteSci(SCI_AIADDR, 0x0050); /* Activate recording! */
}

/* RL port complete */
int VS1063RecordBuf(uint8_t *recBuf, uint32_t recBufferSize) {
    unsigned n=0;

    /* See if there is some data available */
    if ((n = ReadSci(SCI_RECWORDS)) > 0) {
        unsigned i;

        n = MIN(n, recBufferSize/2);
        for (i=0; i<n; i++) {
            uint16_t w = ReadSci(SCI_RECDATA);
            *recBuf++ = (uint8_t)(w >> 8);
            *recBuf++ = (uint8_t)(w & 0xFF);
        }
    }

    return n*2;
}

/* RL port complete*/
void VS1063RecordBufFull(uint8_t *recBuf, uint32_t recBufferSize) {
    unsigned recBufCount=0;

    while (recBufCount < recBufferSize) {
        recBufCount += VS1063RecordBuf(recBuf + recBufCount, recBufferSize-recBufCount);
    }
}

/* RL port complete*/
void VS1063RecordCancel(void) {
    /* The following read from SCI_RECWORDS may appear redundant.
       But it's not: SCI_RECWORDS needs to be rechecked AFTER we
       have seen that SM_CANCEL have cleared. */
    ReadSci(SCI_MODE);
    ReadSci(SCI_RECWORDS);
}

/*
  Hardware Initialization for VS1063.
*/
/* RL port complete */
void VS1063InitHardware(void) {
  /* Write here your microcontroller code which puts VS10xx in hardware
     reset and back (set xRESET to 0 for at least a few clock cycles,
     then to 1). */
    VS1063InitSPI();
}

/*
  Software Initialization for VS1063.
  Note that you need to check whether SM_SDISHARE should be set in
  your application or not.
*/
/*RL port complete*/
void VS1063InitSoftware(void) {
    /* Start initialization with a dummy read, which makes sure our
     microcontoller chips selects and everything are where they
     are supposed to be and that VS10xx's SCI bus is in a known state. */
    ReadSci(SCI_MODE);

    /* First real operation is a software reset. After the software
     reset we know what the status of the IC is. You need, depending
     on your application, either set or not set SM_SDISHARE. See the
     Datasheet for details. */
    /* Not setting: SDISHARE, using the 7 pin connection, i.e. with xDCS*/
    /* Or-in SM_TESTS to allow SDI tests like sine test*/
    WriteSci(SCI_MODE, SM_SDINEW|SM_RESET);

    /* A quick sanity check: write to two registers, then test if we
     get the same results. Note that if you use a too high SPI
     speed, the MSB is the most likely to fail when read again. */
    WriteSci(SCI_HDAT0, 0xABAD);
    WriteSci(SCI_HDAT1, 0x1DEA);
    if (ReadSci(SCI_HDAT0) != 0xABAD || ReadSci(SCI_HDAT1) != 0x1DEA) {
        plf_assert("WriteSci sanity check failed\n", 0);
    }

    /* Set the clock. Until this point we need to run SPI slow so that
     we do not exceed the maximum speeds mentioned in
     Chapter SPI Timing Diagram in the Datasheet. */
    WriteSci(SCI_CLOCKF,
           HZ_TO_SC_FREQ(12288000) | SC_MULT_53_45X | SC_ADD_53_00X);

    /* Now when we have upped the VS10xx clock speed, the microcontroller
     SPI bus can run faster. Do that before you start playing or
     recording files. */
    VS1063AdjustSPIFast();

    /* Set up other parameters. */
    //WriteVS10xxMem(PAR_CONFIG1, PAR_CONFIG1_AAC_SBR_SELECTIVE_UPSAMPLE);

    /* Set volume level */
    WriteSci(SCI_VOL, INIT_VOL);

    /* Now it's time to load the proper patch set. */
    //LoadPlugin(plugin, sizeof(plugin)/sizeof(plugin[0]));
    LoadUserCode();

    /* We're ready to go. */
}

/*RL port complete*/
void VS1063PrintState(void) {
    PLF_PRINT(PRNTGRP_DFLT, "\nMODE %x ST %x \n", ReadSci(SCI_MODE), ReadSci(SCI_STATUS));
    PLF_PRINT(PRNTGRP_DFLT, ", HDAT1 %x HDAT0 %x", ReadSci(SCI_HDAT1), ReadSci(SCI_HDAT0));
    PLF_PRINT(PRNTGRP_DFLT, ", sampleCounter %lu",ReadVS10xxMem32Counter(PAR_SAMPLE_COUNTER));
    PLF_PRINT(PRNTGRP_DFLT, ", sdiFree %u", ReadVS10xxMem(PAR_SDI_FREE));
    PLF_PRINT(PRNTGRP_DFLT, ", audioFill %u", ReadVS10xxMem(PAR_AUDIO_FILL));
    PLF_PRINT(PRNTGRP_DFLT, "\n  positionMSec %lu", ReadVS10xxMem32Counter(PAR_POSITION_MSEC));
    PLF_PRINT(PRNTGRP_DFLT, ", config1 0x%04x", ReadVS10xxMem(PAR_CONFIG1));
    PLF_PRINT(PRNTGRP_DFLT, ", vol 0x%04x", ReadSci(SCI_VOL));
    PLF_PRINT(PRNTGRP_DFLT, "\n");
}

/*RL port complete*/
uint16_t VS1063AdjustClock(int val) {
    uint16_t clock = ReadSci(SCI_CLOCKF) + val;
    WriteSci(SCI_CLOCKF, clock);
    return clock;
}
