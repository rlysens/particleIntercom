#include "plf_circular_buffer.h"

void plf_circular_buf_init(OUT PlfCircularBuf_t *ctxt, IN uint8_t *bufPtr, int bufSizeBytes)
{
    plf_assert("NULL ptr", ctxt);
    plf_assert("NULL ptr", bufPtr);

    ctxt->bufStart = bufPtr;
    ctxt->bufSizeBytes = bufSizeBytes;
    ctxt->readPtr = bufPtr;
    ctxt->writePtr = bufPtr;
}

void plf_circular_buf_reset(INOUT PlfCircularBuf_t *ctxt)
{
    plf_assert("NULL ptr", ctxt);

    ctxt->readPtr = ctxt->bufStart;
    ctxt->writePtr = ctxt->bufStart;
}

/*Returns free space in number of bytes*/
int plf_circular_buf_free_space(IN PlfCircularBuf_t *ctxt)
{
    int freeSpace;

    plf_assert("NULL ptr", ctxt);

    if (ctxt->writePtr < ctxt->readPtr)
    {
        freeSpace = ctxt->readPtr - ctxt->writePtr - 1;
    }
    else
    {
        freeSpace = ctxt->readPtr + ctxt->bufSizeBytes - ctxt->writePtr - 1;
    }

    return freeSpace;
}

/*Returns used space in number of bytes*/
int plf_circular_buf_used_space(IN PlfCircularBuf_t *ctxt)
{
    int usedSpace;

    plf_assert("NULL ptr", ctxt);

    if (ctxt->readPtr <= ctxt->writePtr)
    {
        usedSpace = ctxt->writePtr - ctxt->readPtr;
    }
    else
    {
        usedSpace = ctxt->writePtr + ctxt->bufSizeBytes - ctxt->readPtr;
    }

    return usedSpace;
}

/*Returns number of bytes successfully written. In case of overflow, may be less than requested*/
int plf_circular_buf_write(INOUT PlfCircularBuf_t *ctxt, IN uint8_t *data, int numBytes)
{
    plf_assert("NULL ptr", ctxt);
    plf_assert("NULL ptr", data);

    if (ctxt->writePtr + numBytes > ctxt->bufStart + ctxt->bufSizeBytes)
    {
        /*truncate*/
        numBytes = ctxt->bufStart + ctxt->bufSizeBytes - ctxt->writePtr;
        memcpy(ctxt->writePtr, data, numBytes);
        ctxt->writePtr = ctxt->bufStart;
    }
    else
    {
        memcpy(ctxt->writePtr, data, numBytes);
        ctxt->writePtr += numBytes;
    }

    return numBytes;
}

/*Returns size of data chunk returned in bytes. May be less than requested if a boundary is reached*/
int plf_circular_buf_read_start(IN PlfCircularBuf_t *ctxt, OUT uint8_t **data, int numBytes)
{
    plf_assert("NULL ptr", ctxt);
    plf_assert("NULL ptr", data);

    if (ctxt->readPtr + numBytes > ctxt->bufStart + ctxt->bufSizeBytes)
    {
        /*truncate*/
        numBytes = ctxt->bufStart + ctxt->bufSizeBytes - ctxt->readPtr;
    }

    *data = ctxt->readPtr;

    return numBytes;
}

void plf_circular_buf_read_release(INOUT PlfCircularBuf_t *ctxt, int numBytes)
{
    plf_assert("NULL ptr", ctxt);

    ctxt->readPtr += numBytes;
    while (ctxt->readPtr >= ctxt->bufStart + ctxt->bufSizeBytes)
    {
        ctxt->readPtr -= ctxt->bufSizeBytes;
    }
}
