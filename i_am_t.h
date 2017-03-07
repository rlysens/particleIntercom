// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <stdint.h>
#include <stdlib.h>
#include "lcm_coretypes.h"

#ifndef _i_am_t_h
#define _i_am_t_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _i_am_t i_am_t;
struct _i_am_t
{
    int8_t     name[32];
};

/**
 * Create a deep copy of a i_am_t.
 * When no longer needed, destroy it with i_am_t_destroy()
 */
i_am_t* i_am_t_copy(const i_am_t* to_copy);

/**
 * Destroy an instance of i_am_t created by i_am_t_copy()
 */
void i_am_t_destroy(i_am_t* to_destroy);

/**
 * Encode a message of type i_am_t into binary form.
 *
 * @param buf The output buffer.
 * @param offset Encoding starts at this byte offset into @p buf.
 * @param maxlen Maximum number of bytes to write.  This should generally
 *               be equal to i_am_t_encoded_size().
 * @param msg The message to encode.
 * @return The number of bytes encoded, or <0 if an error occured.
 */
int i_am_t_encode(void *buf, int offset, int maxlen, const i_am_t *p);

/**
 * Decode a message of type i_am_t from binary form.
 * When decoding messages containing strings or variable-length arrays, this
 * function may allocate memory.  When finished with the decoded message,
 * release allocated resources with i_am_t_decode_cleanup().
 *
 * @param buf The buffer containing the encoded message
 * @param offset The byte offset into @p buf where the encoded message starts.
 * @param maxlen The maximum number of bytes to read while decoding.
 * @param msg Output parameter where the decoded message is stored
 * @return The number of bytes decoded, or <0 if an error occured.
 */
int i_am_t_decode(const void *buf, int offset, int maxlen, i_am_t *msg);

/**
 * Release resources allocated by i_am_t_decode()
 * @return 0
 */
int i_am_t_decode_cleanup(i_am_t *p);

/**
 * Check how many bytes are required to encode a message of type i_am_t
 */
int i_am_t_encoded_size(const i_am_t *p);

// LCM support functions. Users should not call these
int64_t __i_am_t_get_hash(void);
uint64_t __i_am_t_hash_recursive(const __lcm_hash_ptr *p);
int     __i_am_t_encode_array(void *buf, int offset, int maxlen, const i_am_t *p, int elements);
int     __i_am_t_decode_array(const void *buf, int offset, int maxlen, i_am_t *p, int elements);
int     __i_am_t_decode_array_cleanup(i_am_t *p, int elements);
int     __i_am_t_encoded_array_size(const i_am_t *p, int elements);
int     __i_am_t_clone_array(const i_am_t *p, i_am_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif
