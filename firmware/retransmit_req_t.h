// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <stdint.h>
#include <stdlib.h>
#include "lcm_coretypes.h"

#ifndef _retransmit_req_t_h
#define _retransmit_req_t_h

#ifdef __cplusplus
extern "C" {
#endif

#define RETRANSMIT_REQ_T_MSG_ID 4

typedef struct _retransmit_req_t retransmit_req_t;
struct _retransmit_req_t
{
    int32_t    source_id;
    int32_t    destination_id;
    int32_t    seq_number;
};

/**
 * Create a deep copy of a retransmit_req_t.
 * When no longer needed, destroy it with retransmit_req_t_destroy()
 */
retransmit_req_t* retransmit_req_t_copy(const retransmit_req_t* to_copy);

/**
 * Destroy an instance of retransmit_req_t created by retransmit_req_t_copy()
 */
void retransmit_req_t_destroy(retransmit_req_t* to_destroy);

/**
 * Encode a message of type retransmit_req_t into binary form.
 *
 * @param buf The output buffer.
 * @param offset Encoding starts at this byte offset into @p buf.
 * @param maxlen Maximum number of bytes to write.  This should generally
 *               be equal to retransmit_req_t_encoded_size().
 * @param msg The message to encode.
 * @return The number of bytes encoded, or <0 if an error occured.
 */
int retransmit_req_t_encode(void *buf, int offset, int maxlen, const retransmit_req_t *p);

/**
 * Decode a message of type retransmit_req_t from binary form.
 * When decoding messages containing strings or variable-length arrays, this
 * function may allocate memory.  When finished with the decoded message,
 * release allocated resources with retransmit_req_t_decode_cleanup().
 *
 * @param buf The buffer containing the encoded message
 * @param offset The byte offset into @p buf where the encoded message starts.
 * @param maxlen The maximum number of bytes to read while decoding.
 * @param msg Output parameter where the decoded message is stored
 * @return The number of bytes decoded, or <0 if an error occured.
 */
int retransmit_req_t_decode(const void *buf, int offset, int maxlen, retransmit_req_t *msg);

/**
 * Release resources allocated by retransmit_req_t_decode()
 * @return 0
 */
int retransmit_req_t_decode_cleanup(retransmit_req_t *p);

/**
 * Check how many bytes are required to encode a message of type retransmit_req_t
 */
int retransmit_req_t_encoded_size(const retransmit_req_t *p);

// LCM support functions. Users should not call these
int64_t __retransmit_req_t_get_hash(void);
uint64_t __retransmit_req_t_hash_recursive(const __lcm_hash_ptr *p);
int     __retransmit_req_t_encode_array(void *buf, int offset, int maxlen, const retransmit_req_t *p, int elements);
int     __retransmit_req_t_decode_array(const void *buf, int offset, int maxlen, retransmit_req_t *p, int elements);
int     __retransmit_req_t_decode_array_cleanup(retransmit_req_t *p, int elements);
int     __retransmit_req_t_encoded_array_size(const retransmit_req_t *p, int elements);
int     __retransmit_req_t_clone_array(const retransmit_req_t *p, retransmit_req_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif