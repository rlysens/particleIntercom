#include "plf_circular_buffer.h"
#include "plf_utils.h"
#include "Particle.h"

#define MODULE_ID 900

#if 0
void Plf_CircularBuf::_track(void) {
    static int trackBuf[1000];
    static int timestamps[1000];
    static int trackBufIdx=0;

    if (trackBufIdx<1000) {
        trackBuf[trackBufIdx] = usedSpace();
        timestamps[trackBufIdx++] = millis();
    }
    else {
        int ii;

        for (ii=0; ii<1000; ii++) {
            PLF_PRINT(PRNTGRP_DFLT, "%d %d\n", timestamps[ii], trackBuf[ii]);
        }

        trackBufIdx=0;
    }


}
#endif

Plf_CircularBuf::Plf_CircularBuf(IN uint8_t *bufPtr, int bufSizeBytes) : _bufStart(bufPtr), 
    _bufSizeBytes(bufSizeBytes), _readPtr(bufPtr), _writePtr(bufPtr) {}

void Plf_CircularBuf::reset(void) {
    _readPtr = _bufStart;
    _writePtr = _bufStart;
}

/*Returns free space in number of bytes*/
int Plf_CircularBuf::freeSpace(void) {
    int freeSpace;

    if (_writePtr < _readPtr) {
        freeSpace = _readPtr - _writePtr - 1;
    }
    else {
        freeSpace = _readPtr + _bufSizeBytes - _writePtr - 1;
    }

    return freeSpace;
}

/*Returns used space in number of bytes*/
int Plf_CircularBuf::usedSpace(void) {
    int usedSpace;

    if (_readPtr <= _writePtr)
    {
        usedSpace = _writePtr - _readPtr;
    }
    else
    {
        usedSpace = _writePtr + _bufSizeBytes - _readPtr;
    }

    return usedSpace;
}

/*Returns number of bytes successfully written. In case of overflow, may be less than requested*/
int Plf_CircularBuf::write(IN uint8_t *data, int numBytes) {
    if (_writePtr + numBytes > _bufStart + _bufSizeBytes)
    {
        /*truncate*/
        numBytes = _bufStart + _bufSizeBytes - _writePtr;
        memcpy(_writePtr, data, numBytes);
        _writePtr = _bufStart;
    }
    else
    {
        memcpy(_writePtr, data, numBytes);
        _writePtr += numBytes;
    }

#if 0
    _track();
#endif

    return numBytes;
}

/*Returns size of data chunk returned in bytes. May be less than requested if a boundary is reached*/
int Plf_CircularBuf::readStart(OUT uint8_t **data, int numBytes) {
    if (_readPtr + numBytes > _bufStart + _bufSizeBytes) {
        /*truncate*/
        numBytes = _bufStart + _bufSizeBytes - _readPtr;
    }

    *data = _readPtr;

    return numBytes;
}

void Plf_CircularBuf::readRelease(int numBytes) {
    _readPtr += numBytes;
    while (_readPtr >= _bufStart + _bufSizeBytes) {
        _readPtr -= _bufSizeBytes;
    }

#if 0
    _track();
#endif
}
