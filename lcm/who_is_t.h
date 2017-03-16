// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <stdint.h>
#include <stdlib.h>
#include <lcm/lcm_coretypes.h>

#ifndef _who_is_t_h
#define _who_is_t_h

#ifdef __cplusplus
extern "C" {
#endif

#define WHO_IS_T_MSG_ID 4

typedef struct _who_is_t who_is_t;
struct _who_is_t
{
    int8_t     name[32];
};

/**
 * Create a deep copy of a who_is_t.
 * When no longer needed, destroy it with who_is_t_destroy()
 */
who_is_t* who_is_t_copy(const who_is_t* to_copy);

/**
 * Destroy an instance of who_is_t created by who_is_t_copy()
 */
void who_is_t_destroy(who_is_t* to_destroy);

/**
 * Encode a message of type who_is_t into binary form.
 *
 * @param buf The output buffer.
 * @param offset Encoding starts at this byte offset into @p buf.
 * @param maxlen Maximum number of bytes to write.  This should generally
 *               be equal to who_is_t_encoded_size().
 * @param msg The message to encode.
 * @return The number of bytes encoded, or <0 if an error occured.
 */
int who_is_t_encode(void *buf, int offset, int maxlen, const who_is_t *p);

/**
 * Decode a message of type who_is_t from binary form.
 * When decoding messages containing strings or variable-length arrays, this
 * function may allocate memory.  When finished with the decoded message,
 * release allocated resources with who_is_t_decode_cleanup().
 *
 * @param buf The buffer containing the encoded message
 * @param offset The byte offset into @p buf where the encoded message starts.
 * @param maxlen The maximum number of bytes to read while decoding.
 * @param msg Output parameter where the decoded message is stored
 * @return The number of bytes decoded, or <0 if an error occured.
 */
int who_is_t_decode(const void *buf, int offset, int maxlen, who_is_t *msg);

/**
 * Release resources allocated by who_is_t_decode()
 * @return 0
 */
int who_is_t_decode_cleanup(who_is_t *p);

/**
 * Check how many bytes are required to encode a message of type who_is_t
 */
int who_is_t_encoded_size(const who_is_t *p);

// LCM support functions. Users should not call these
int64_t __who_is_t_get_hash(void);
uint64_t __who_is_t_hash_recursive(const __lcm_hash_ptr *p);
int     __who_is_t_encode_array(void *buf, int offset, int maxlen, const who_is_t *p, int elements);
int     __who_is_t_decode_array(const void *buf, int offset, int maxlen, who_is_t *p, int elements);
int     __who_is_t_decode_array_cleanup(who_is_t *p, int elements);
int     __who_is_t_encoded_array_size(const who_is_t *p, int elements);
int     __who_is_t_clone_array(const who_is_t *p, who_is_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif
