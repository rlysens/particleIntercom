// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <stdint.h>
#include <stdlib.h>
#include <lcm/lcm_coretypes.h>

#ifndef _comm_stop_ack_t_h
#define _comm_stop_ack_t_h

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_STOP_ACK_T_MSG_ID 12

typedef struct _comm_stop_ack_t comm_stop_ack_t;
struct _comm_stop_ack_t
{
    int32_t    source_id;
    int32_t    destination_id;
};

/**
 * Create a deep copy of a comm_stop_ack_t.
 * When no longer needed, destroy it with comm_stop_ack_t_destroy()
 */
comm_stop_ack_t* comm_stop_ack_t_copy(const comm_stop_ack_t* to_copy);

/**
 * Destroy an instance of comm_stop_ack_t created by comm_stop_ack_t_copy()
 */
void comm_stop_ack_t_destroy(comm_stop_ack_t* to_destroy);

/**
 * Encode a message of type comm_stop_ack_t into binary form.
 *
 * @param buf The output buffer.
 * @param offset Encoding starts at this byte offset into @p buf.
 * @param maxlen Maximum number of bytes to write.  This should generally
 *               be equal to comm_stop_ack_t_encoded_size().
 * @param msg The message to encode.
 * @return The number of bytes encoded, or <0 if an error occured.
 */
int comm_stop_ack_t_encode(void *buf, int offset, int maxlen, const comm_stop_ack_t *p);

/**
 * Decode a message of type comm_stop_ack_t from binary form.
 * When decoding messages containing strings or variable-length arrays, this
 * function may allocate memory.  When finished with the decoded message,
 * release allocated resources with comm_stop_ack_t_decode_cleanup().
 *
 * @param buf The buffer containing the encoded message
 * @param offset The byte offset into @p buf where the encoded message starts.
 * @param maxlen The maximum number of bytes to read while decoding.
 * @param msg Output parameter where the decoded message is stored
 * @return The number of bytes decoded, or <0 if an error occured.
 */
int comm_stop_ack_t_decode(const void *buf, int offset, int maxlen, comm_stop_ack_t *msg);

/**
 * Release resources allocated by comm_stop_ack_t_decode()
 * @return 0
 */
int comm_stop_ack_t_decode_cleanup(comm_stop_ack_t *p);

/**
 * Check how many bytes are required to encode a message of type comm_stop_ack_t
 */
int comm_stop_ack_t_encoded_size(const comm_stop_ack_t *p);

// LCM support functions. Users should not call these
int64_t __comm_stop_ack_t_get_hash(void);
uint64_t __comm_stop_ack_t_hash_recursive(const __lcm_hash_ptr *p);
int     __comm_stop_ack_t_encode_array(void *buf, int offset, int maxlen, const comm_stop_ack_t *p, int elements);
int     __comm_stop_ack_t_decode_array(const void *buf, int offset, int maxlen, comm_stop_ack_t *p, int elements);
int     __comm_stop_ack_t_decode_array_cleanup(comm_stop_ack_t *p, int elements);
int     __comm_stop_ack_t_encoded_array_size(const comm_stop_ack_t *p, int elements);
int     __comm_stop_ack_t_clone_array(const comm_stop_ack_t *p, comm_stop_ack_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif