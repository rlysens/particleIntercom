// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <string.h>
#include "keep_alive_t.h"

static int __keep_alive_t_hash_computed;
static uint64_t __keep_alive_t_hash;

uint64_t __keep_alive_t_hash_recursive(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for (fp = p; fp != NULL; fp = fp->parent)
        if (fp->v == __keep_alive_t_get_hash)
            return 0;

    __lcm_hash_ptr cp;
    cp.parent =  p;
    cp.v = (void*)__keep_alive_t_get_hash;
    (void) cp;

    uint64_t hash = (uint64_t)0x18f1ed95b691afecLL
         + __int32_t_hash_recursive(&cp)
         + __int32_t_hash_recursive(&cp)
        ;

    return (hash<<1) + ((hash>>63)&1);
}

int64_t __keep_alive_t_get_hash(void)
{
    if (!__keep_alive_t_hash_computed) {
        __keep_alive_t_hash = (int64_t)__keep_alive_t_hash_recursive(NULL);
        __keep_alive_t_hash_computed = 1;
    }

    return __keep_alive_t_hash;
}

int __keep_alive_t_encode_array(void *buf, int offset, int maxlen, const keep_alive_t *p, int elements)
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

int keep_alive_t_encode(void *buf, int offset, int maxlen, const keep_alive_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __keep_alive_t_get_hash();

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    thislen = __keep_alive_t_encode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int __keep_alive_t_encoded_array_size(const keep_alive_t *p, int elements)
{
    int size = 0, element;
    for (element = 0; element < elements; element++) {

        size += __int32_t_encoded_array_size(&(p[element].source_id), 1);

        size += __int32_t_encoded_array_size(&(p[element].destination_id), 1);

    }
    return size;
}

int keep_alive_t_encoded_size(const keep_alive_t *p)
{
    return 8 + __keep_alive_t_encoded_array_size(p, 1);
}

int __keep_alive_t_decode_array(const void *buf, int offset, int maxlen, keep_alive_t *p, int elements)
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

int __keep_alive_t_decode_array_cleanup(keep_alive_t *p, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_decode_array_cleanup(&(p[element].source_id), 1);

        __int32_t_decode_array_cleanup(&(p[element].destination_id), 1);

    }
    return 0;
}

int keep_alive_t_decode(const void *buf, int offset, int maxlen, keep_alive_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __keep_alive_t_get_hash();

    int64_t this_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (this_hash != hash) return -1;

    thislen = __keep_alive_t_decode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int keep_alive_t_decode_cleanup(keep_alive_t *p)
{
    return __keep_alive_t_decode_array_cleanup(p, 1);
}

int __keep_alive_t_clone_array(const keep_alive_t *p, keep_alive_t *q, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_clone_array(&(p[element].source_id), &(q[element].source_id), 1);

        __int32_t_clone_array(&(p[element].destination_id), &(q[element].destination_id), 1);

    }
    return 0;
}

keep_alive_t *keep_alive_t_copy(const keep_alive_t *p)
{
    keep_alive_t *q = (keep_alive_t*) malloc(sizeof(keep_alive_t));
    __keep_alive_t_clone_array(p, q, 1);
    return q;
}

void keep_alive_t_destroy(keep_alive_t *p)
{
    __keep_alive_t_decode_array_cleanup(p, 1);
    free(p);
}

