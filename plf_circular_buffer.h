#ifndef PLF_CIRCULAR_BUFFER_H
#define PLF_CIRCULAR_BUFFER_H

#include "Particle.h"
#include "plf_utils.h"

typedef struct {
    uint8_t *bufStart;
    int bufSizeBytes;
    uint8_t *readPtr;
    uint8_t *writePtr;
} PlfCircularBuf_t;

void plf_circular_buf_init(OUT PlfCircularBuf_t *ctxt, IN uint8_t *bufPtr, int bufSizeBytes);
void plf_circular_buf_reset(INOUT PlfCircularBuf_t *ctxt);
/*Returns free space in number of bytes*/
int plf_circular_buf_free_space(IN PlfCircularBuf_t *ctxt);
/*Returns used space in number of bytes*/
int plf_circular_buf_used_space(IN PlfCircularBuf_t *ctxt);
/*Returns number of bytes successfully written. In case of overflow, may be less than requested*/
int plf_circular_buf_write(INOUT PlfCircularBuf_t *ctxt, IN uint8_t *data, int numBytes);
/*Returns size of data chunk returned in bytes. May be less than request if a boundary is reached*/
int plf_circular_buf_read_start(IN PlfCircularBuf_t *ctxt, OUT uint8_t **data, int numBytes);

void plf_circular_buf_read_release(INOUT PlfCircularBuf_t *ctxt, int numBytes);

#endif /*PLF_CIRCULAR_BUFFER_H*/
