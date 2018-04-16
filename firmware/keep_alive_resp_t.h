// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <stdint.h>
#include <stdlib.h>
#include "lcm_coretypes.h"

#ifndef _keep_alive_resp_t_h
#define _keep_alive_resp_t_h

#ifdef __cplusplus
extern "C" {
#endif

#define KEEP_ALIVE_RESP_T_MSG_ID 8

typedef struct _keep_alive_resp_t keep_alive_resp_t;
struct _keep_alive_resp_t
{
    int32_t    redirect_addr;
    int32_t    dummy;
};

/**
 * Create a deep copy of a keep_alive_resp_t.
 * When no longer needed, destroy it with keep_alive_resp_t_destroy()
 */
keep_alive_resp_t* keep_alive_resp_t_copy(const keep_alive_resp_t* to_copy);

/**
 * Destroy an instance of keep_alive_resp_t created by keep_alive_resp_t_copy()
 */
void keep_alive_resp_t_destroy(keep_alive_resp_t* to_destroy);

/**
 * Encode a message of type keep_alive_resp_t into binary form.
 *
 * @param buf The output buffer.
 * @param offset Encoding starts at this byte offset into @p buf.
 * @param maxlen Maximum number of bytes to write.  This should generally
 *               be equal to keep_alive_resp_t_encoded_size().
 * @param msg The message to encode.
 * @return The number of bytes encoded, or <0 if an error occured.
 */
int keep_alive_resp_t_encode(void *buf, int offset, int maxlen, const keep_alive_resp_t *p);

/**
 * Decode a message of type keep_alive_resp_t from binary form.
 * When decoding messages containing strings or variable-length arrays, this
 * function may allocate memory.  When finished with the decoded message,
 * release allocated resources with keep_alive_resp_t_decode_cleanup().
 *
 * @param buf The buffer containing the encoded message
 * @param offset The byte offset into @p buf where the encoded message starts.
 * @param maxlen The maximum number of bytes to read while decoding.
 * @param msg Output parameter where the decoded message is stored
 * @return The number of bytes decoded, or <0 if an error occured.
 */
int keep_alive_resp_t_decode(const void *buf, int offset, int maxlen, keep_alive_resp_t *msg);

/**
 * Release resources allocated by keep_alive_resp_t_decode()
 * @return 0
 */
int keep_alive_resp_t_decode_cleanup(keep_alive_resp_t *p);

/**
 * Check how many bytes are required to encode a message of type keep_alive_resp_t
 */
int keep_alive_resp_t_encoded_size(const keep_alive_resp_t *p);

// LCM support functions. Users should not call these
int64_t __keep_alive_resp_t_get_hash(void);
uint64_t __keep_alive_resp_t_hash_recursive(const __lcm_hash_ptr *p);
int     __keep_alive_resp_t_encode_array(void *buf, int offset, int maxlen, const keep_alive_resp_t *p, int elements);
int     __keep_alive_resp_t_decode_array(const void *buf, int offset, int maxlen, keep_alive_resp_t *p, int elements);
int     __keep_alive_resp_t_decode_array_cleanup(keep_alive_resp_t *p, int elements);
int     __keep_alive_resp_t_encoded_array_size(const keep_alive_resp_t *p, int elements);
int     __keep_alive_resp_t_clone_array(const keep_alive_resp_t *p, keep_alive_resp_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif
