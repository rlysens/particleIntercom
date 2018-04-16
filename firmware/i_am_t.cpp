// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <string.h>
#include "i_am_t.h"

static int __i_am_t_hash_computed;
static uint64_t __i_am_t_hash;

uint64_t __i_am_t_hash_recursive(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for (fp = p; fp != NULL; fp = fp->parent)
        if (fp->v == __i_am_t_get_hash)
            return 0;

    __lcm_hash_ptr cp;
    cp.parent =  p;
    cp.v = (void*)__i_am_t_get_hash;
    (void) cp;

    uint64_t hash = (uint64_t)0x14e4e5049c85f050LL
         + __int32_t_hash_recursive(&cp)
         + __int32_t_hash_recursive(&cp)
        ;

    return (hash<<1) + ((hash>>63)&1);
}

int64_t __i_am_t_get_hash(void)
{
    if (!__i_am_t_hash_computed) {
        __i_am_t_hash = (int64_t)__i_am_t_hash_recursive(NULL);
        __i_am_t_hash_computed = 1;
    }

    return __i_am_t_hash;
}

int __i_am_t_encode_array(void *buf, int offset, int maxlen, const i_am_t *p, int elements)
{
    int pos = 0, element;
    int thislen;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].my_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].srvr_addr), 1);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int i_am_t_encode(void *buf, int offset, int maxlen, const i_am_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __i_am_t_get_hash();

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    thislen = __i_am_t_encode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int __i_am_t_encoded_array_size(const i_am_t *p, int elements)
{
    int size = 0, element;
    for (element = 0; element < elements; element++) {

        size += __int32_t_encoded_array_size(&(p[element].my_id), 1);

        size += __int32_t_encoded_array_size(&(p[element].srvr_addr), 1);

    }
    return size;
}

int i_am_t_encoded_size(const i_am_t *p)
{
    return 8 + __i_am_t_encoded_array_size(p, 1);
}

int __i_am_t_decode_array(const void *buf, int offset, int maxlen, i_am_t *p, int elements)
{
    int pos = 0, thislen, element;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].my_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].srvr_addr), 1);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int __i_am_t_decode_array_cleanup(i_am_t *p, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_decode_array_cleanup(&(p[element].my_id), 1);

        __int32_t_decode_array_cleanup(&(p[element].srvr_addr), 1);

    }
    return 0;
}

int i_am_t_decode(const void *buf, int offset, int maxlen, i_am_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __i_am_t_get_hash();

    int64_t this_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (this_hash != hash) return -1;

    thislen = __i_am_t_decode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int i_am_t_decode_cleanup(i_am_t *p)
{
    return __i_am_t_decode_array_cleanup(p, 1);
}

int __i_am_t_clone_array(const i_am_t *p, i_am_t *q, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_clone_array(&(p[element].my_id), &(q[element].my_id), 1);

        __int32_t_clone_array(&(p[element].srvr_addr), &(q[element].srvr_addr), 1);

    }
    return 0;
}

i_am_t *i_am_t_copy(const i_am_t *p)
{
    i_am_t *q = (i_am_t*) malloc(sizeof(i_am_t));
    __i_am_t_clone_array(p, q, 1);
    return q;
}

void i_am_t_destroy(i_am_t *p)
{
    __i_am_t_decode_array_cleanup(p, 1);
    free(p);
}

