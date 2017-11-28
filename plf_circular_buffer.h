#ifndef PLF_CIRCULAR_BUFFER_H
#define PLF_CIRCULAR_BUFFER_H

#include "Particle.h"
#include "plf_utils.h"

class Plf_CircularBuf {
private:
    uint8_t *_bufStart;
    int _bufSizeBytes;
    uint8_t *_readPtr;
    uint8_t *_writePtr;
public:
	Plf_CircularBuf(IN uint8_t *bufPtr, int bufSizeBytes);
	void reset(void);
	/*Returns free space in number of bytes*/
	int freeSpace(void);
	/*Returns used space in number of bytes*/
	int usedSpace(void);
	/*Returns number of bytes successfully written. In case of overflow, may be less than requested*/
	int write(IN uint8_t *data, int numBytes);
	/*Returns size of data chunk returned in bytes. May be less than request if a boundary is reached*/
	int readStart(OUT uint8_t **data, int numBytes);

	void readRelease(int numBytes);
};

#endif /*PLF_CIRCULAR_BUFFER_H*/
