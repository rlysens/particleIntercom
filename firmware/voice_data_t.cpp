// THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
// BY HAND!!
//
// Generated by lcm-gen

#include <string.h>
#include "voice_data_t.h"

static int __voice_data_t_hash_computed;
static uint64_t __voice_data_t_hash;

uint64_t __voice_data_t_hash_recursive(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for (fp = p; fp != NULL; fp = fp->parent)
        if (fp->v == __voice_data_t_get_hash)
            return 0;

    __lcm_hash_ptr cp;
    cp.parent =  p;
    cp.v = (void*)__voice_data_t_get_hash;
    (void) cp;

    uint64_t hash = (uint64_t)0x5b0470be243fe4d0LL
         + __int32_t_hash_recursive(&cp)
         + __int32_t_hash_recursive(&cp)
         + __int32_t_hash_recursive(&cp)
         + __int16_t_hash_recursive(&cp)
         + __int8_t_hash_recursive(&cp)
        ;

    return (hash<<1) + ((hash>>63)&1);
}

int64_t __voice_data_t_get_hash(void)
{
    if (!__voice_data_t_hash_computed) {
        __voice_data_t_hash = (int64_t)__voice_data_t_hash_recursive(NULL);
        __voice_data_t_hash_computed = 1;
    }

    return __voice_data_t_hash;
}

int __voice_data_t_encode_array(void *buf, int offset, int maxlen, const voice_data_t *p, int elements)
{
    int pos = 0, element;
    int thislen;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].source_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].destination_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].seq_number), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int16_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].data_size), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int8_t_encode_array(buf, offset + pos, maxlen - pos, p[element].data, 482);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int voice_data_t_encode(void *buf, int offset, int maxlen, const voice_data_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __voice_data_t_get_hash();

    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    thislen = __voice_data_t_encode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int __voice_data_t_encoded_array_size(const voice_data_t *p, int elements)
{
    int size = 0, element;
    for (element = 0; element < elements; element++) {

        size += __int32_t_encoded_array_size(&(p[element].source_id), 1);

        size += __int32_t_encoded_array_size(&(p[element].destination_id), 1);

        size += __int32_t_encoded_array_size(&(p[element].seq_number), 1);

        size += __int16_t_encoded_array_size(&(p[element].data_size), 1);

        size += __int8_t_encoded_array_size(p[element].data, 482);

    }
    return size;
}

int voice_data_t_encoded_size(const voice_data_t *p)
{
    return 8 + __voice_data_t_encoded_array_size(p, 1);
}

int __voice_data_t_decode_array(const void *buf, int offset, int maxlen, voice_data_t *p, int elements)
{
    int pos = 0, thislen, element;

    for (element = 0; element < elements; element++) {

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].source_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].destination_id), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int32_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].seq_number), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int16_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].data_size), 1);
        if (thislen < 0) return thislen; else pos += thislen;

        thislen = __int8_t_decode_array(buf, offset + pos, maxlen - pos, p[element].data, 482);
        if (thislen < 0) return thislen; else pos += thislen;

    }
    return pos;
}

int __voice_data_t_decode_array_cleanup(voice_data_t *p, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_decode_array_cleanup(&(p[element].source_id), 1);

        __int32_t_decode_array_cleanup(&(p[element].destination_id), 1);

        __int32_t_decode_array_cleanup(&(p[element].seq_number), 1);

        __int16_t_decode_array_cleanup(&(p[element].data_size), 1);

        __int8_t_decode_array_cleanup(p[element].data, 482);

    }
    return 0;
}

int voice_data_t_decode(const void *buf, int offset, int maxlen, voice_data_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __voice_data_t_get_hash();

    int64_t this_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (this_hash != hash) return -1;

    thislen = __voice_data_t_decode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;

    return pos;
}

int voice_data_t_decode_cleanup(voice_data_t *p)
{
    return __voice_data_t_decode_array_cleanup(p, 1);
}

int __voice_data_t_clone_array(const voice_data_t *p, voice_data_t *q, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {

        __int32_t_clone_array(&(p[element].source_id), &(q[element].source_id), 1);

        __int32_t_clone_array(&(p[element].destination_id), &(q[element].destination_id), 1);

        __int32_t_clone_array(&(p[element].seq_number), &(q[element].seq_number), 1);

        __int16_t_clone_array(&(p[element].data_size), &(q[element].data_size), 1);

        __int8_t_clone_array(p[element].data, q[element].data, 482);

    }
    return 0;
}

voice_data_t *voice_data_t_copy(const voice_data_t *p)
{
    voice_data_t *q = (voice_data_t*) malloc(sizeof(voice_data_t));
    __voice_data_t_clone_array(p, q, 1);
    return q;
}

void voice_data_t_destroy(voice_data_t *p)
{
    __voice_data_t_decode_array_cleanup(p, 1);
    free(p);
}

