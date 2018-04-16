// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <string.h>
#include "retire_t.h"

static int __retire_t_hash_computed;
static uint64_t __retire_t_hash;

uint64_t __retire_t_hash_recursive(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for (fp = p; fp != NULL; fp = fp->parent)
        if (fp->v == __retire_t_get_hash)
            return 0;

    __lcm_hash_ptr cp;
    cp.parent =  p;
    cp.v = (void*)__retire_t_get_hash;
    (void) cp;

    uint64_t hash = (uint64_t)0x225f30e922788c5eLL
         + __int32_t_hash_recursive(&cp)
         + __int8_t_hash_recursive(&cp)
        ;

    return (hash<<1) + ((hash>>63)&1);
}

int64_t __retire_t_get_hash(void)
{
    if (!__retire_t_hash_computed) {
        __retire_t_hash = (int64_t)__retire_t_hash_recursive(NULL);
        __retire_t_hash_computed = 1;
    }

    return __retire_t_hash;
}

int __retire_t_encode_array(void *buf, int offset, int maxlen, const retire_t *p, int elements)
{
    int pos = 0, element;
    int thislen;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].my_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int8_t_encode_array(buf, offset + pos, maxlen - pos, p[element].dummy, 4);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int retire_t_encode(void *buf, int offset, int maxlen, const retire_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __retire_t_get_hash();

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    thislen = __retire_t_encode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int __retire_t_encoded_array_size(const retire_t *p, int elements)
{
    int size = 0, element;
    for (element = 0; element < elements; element++) {

        size += __int32_t_encoded_array_size(&(p[element].my_id), 1);

        size += __int8_t_encoded_array_size(p[element].dummy, 4);

    }
    return size;
}

int retire_t_encoded_size(const retire_t *p)
{
    return 8 + __retire_t_encoded_array_size(p, 1);
}

int __retire_t_decode_array(const void *buf, int offset, int maxlen, retire_t *p, int elements)
{
    int pos = 0, thislen, element;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].my_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int8_t_decode_array(buf, offset + pos, maxlen - pos, p[element].dummy, 4);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int __retire_t_decode_array_cleanup(retire_t *p, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_decode_array_cleanup(&(p[element].my_id), 1);

        __int8_t_decode_array_cleanup(p[element].dummy, 4);

    }
    return 0;
}

int retire_t_decode(const void *buf, int offset, int maxlen, retire_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __retire_t_get_hash();

    int64_t this_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (this_hash != hash) return -1;

    thislen = __retire_t_decode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int retire_t_decode_cleanup(retire_t *p)
{
    return __retire_t_decode_array_cleanup(p, 1);
}

int __retire_t_clone_array(const retire_t *p, retire_t *q, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_clone_array(&(p[element].my_id), &(q[element].my_id), 1);

        __int8_t_clone_array(p[element].dummy, q[element].dummy, 4);

    }
    return 0;
}

retire_t *retire_t_copy(const retire_t *p)
{
    retire_t *q = (retire_t*) malloc(sizeof(retire_t));
    __retire_t_clone_array(p, q, 1);
    return q;
}

void retire_t_destroy(retire_t *p)
{
    __retire_t_decode_array_cleanup(p, 1);
    free(p);
}

