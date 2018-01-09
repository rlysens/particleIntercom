// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <string.h>
#include "comm_start_ack_t.h"

static int __comm_start_ack_t_hash_computed;
static uint64_t __comm_start_ack_t_hash;

uint64_t __comm_start_ack_t_hash_recursive(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for (fp = p; fp != NULL; fp = fp->parent)
        if (fp->v == __comm_start_ack_t_get_hash)
            return 0;

    __lcm_hash_ptr cp;
    cp.parent =  p;
    cp.v = (void*)__comm_start_ack_t_get_hash;
    (void) cp;

    uint64_t hash = (uint64_t)0x18f1ed95b691afecLL
         + __int32_t_hash_recursive(&cp)
         + __int32_t_hash_recursive(&cp)
        ;

    return (hash<<1) + ((hash>>63)&1);
}

int64_t __comm_start_ack_t_get_hash(void)
{
    if (!__comm_start_ack_t_hash_computed) {
        __comm_start_ack_t_hash = (int64_t)__comm_start_ack_t_hash_recursive(NULL);
        __comm_start_ack_t_hash_computed = 1;
    }

    return __comm_start_ack_t_hash;
}

int __comm_start_ack_t_encode_array(void *buf, int offset, int maxlen, const comm_start_ack_t *p, int elements)
{
    int pos = 0, element;
    int thislen;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].source_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].destination_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int comm_start_ack_t_encode(void *buf, int offset, int maxlen, const comm_start_ack_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __comm_start_ack_t_get_hash();

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    thislen = __comm_start_ack_t_encode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int __comm_start_ack_t_encoded_array_size(const comm_start_ack_t *p, int elements)
{
    int size = 0, element;
    for (element = 0; element < elements; element++) {

        size += __int32_t_encoded_array_size(&(p[element].source_id), 1);

        size += __int32_t_encoded_array_size(&(p[element].destination_id), 1);

    }
    return size;
}

int comm_start_ack_t_encoded_size(const comm_start_ack_t *p)
{
    return 8 + __comm_start_ack_t_encoded_array_size(p, 1);
}

int __comm_start_ack_t_decode_array(const void *buf, int offset, int maxlen, comm_start_ack_t *p, int elements)
{
    int pos = 0, thislen, element;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].source_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].destination_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int __comm_start_ack_t_decode_array_cleanup(comm_start_ack_t *p, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_decode_array_cleanup(&(p[element].source_id), 1);

        __int32_t_decode_array_cleanup(&(p[element].destination_id), 1);

    }
    return 0;
}

int comm_start_ack_t_decode(const void *buf, int offset, int maxlen, comm_start_ack_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __comm_start_ack_t_get_hash();

    int64_t this_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (this_hash != hash) return -1;

    thislen = __comm_start_ack_t_decode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int comm_start_ack_t_decode_cleanup(comm_start_ack_t *p)
{
    return __comm_start_ack_t_decode_array_cleanup(p, 1);
}

int __comm_start_ack_t_clone_array(const comm_start_ack_t *p, comm_start_ack_t *q, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_clone_array(&(p[element].source_id), &(q[element].source_id), 1);

        __int32_t_clone_array(&(p[element].destination_id), &(q[element].destination_id), 1);

    }
    return 0;
}

comm_start_ack_t *comm_start_ack_t_copy(const comm_start_ack_t *p)
{
    comm_start_ack_t *q = (comm_start_ack_t*) malloc(sizeof(comm_start_ack_t));
    __comm_start_ack_t_clone_array(p, q, 1);
    return q;
}

void comm_start_ack_t_destroy(comm_start_ack_t *p)
{
    __comm_start_ack_t_decode_array_cleanup(p, 1);
    free(p);
}

