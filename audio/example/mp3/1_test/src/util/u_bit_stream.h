//来自minimp3中minimp3.h

#ifndef _BS_H_
#define _BS_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
    const uint8_t *buf;
    uint64_t pos;
    uint64_t limit;
} bit_stream_t;

static inline void u_bit_stream_bs_init(bit_stream_t* bs, const uint8_t* data, int bytes)
{
    bs->buf   = data;
    bs->pos   = 0;
    bs->limit = bytes * 8;
}

static inline uint8_t u_bit_stream_get_bits_u8(bit_stream_t* bs)
{
    /**
     * s = bs->pos & 7: 表示pos中不足一个byte的位数
    */
    uint8_t next, cache = 0, s = bs->pos & 7;
    int shl = 8 + s;
    const uint8_t *p = bs->buf + (bs->pos >> 3);

    if ((bs->pos += 8) > bs->limit)
    {
        return 0;
    }

    next = *p++ & (255 >> s); //从指针 p 指向的当前字节开始读取数据，根据 s 的值掩码处理，获取当前字节中未被读取的部分。

    while ((shl -= 8) > 0)
    {
        cache |= next << shl;
        next = *p++;
    }

    return cache | (next >> -shl);
}

static inline uint16_t u_bit_stream_get_bits_u16(bit_stream_t* bs)
{
    /**
     * s = bs->pos & 7: 表示pos中不足一个byte的位数
    */
    uint16_t next, cache = 0, s = bs->pos & 7;
    int shl = 16 + s;
    const uint8_t *p = bs->buf + (bs->pos >> 3);

    if ((bs->pos += 16) > bs->limit)
    {
        return 0;
    }

    next = *p++ & (255 >> s); //从指针 p 指向的当前字节开始读取数据，根据 s 的值掩码处理，获取当前字节中未被读取的部分。

    while ((shl -= 8) > 0)
    {
        cache |= next << shl;
        next = *p++;
    }

    return cache | (next >> -shl);
}

static inline uint32_t u_bit_stream_get_bits_u32(bit_stream_t* bs)
{
    /**
     * s = bs->pos & 7: 表示pos中不足一个byte的位数
    */
    uint32_t next, cache = 0, s = bs->pos & 7;
    int shl = 32 + s;
    const uint8_t *p = bs->buf + (bs->pos >> 3);

    if ((bs->pos += 32) > bs->limit)
    {
        return 0;
    }

    next = *p++ & (255 >> s); //从指针 p 指向的当前字节开始读取数据，根据 s 的值掩码处理，获取当前字节中未被读取的部分。

    while ((shl -= 8) > 0)
    {
        cache |= next << shl;
        next = *p++;
    }

    return cache | (next >> -shl);
}

static inline uint64_t u_bit_stream_get_bits_u64(bit_stream_t* bs)
{
    /**
     * s = bs->pos & 7: 表示pos中不足一个byte的位数
    */
    uint64_t next, cache = 0, s = bs->pos & 7;
    int shl = 64 + s;
    const uint8_t *p = bs->buf + (bs->pos >> 3);

    if ((bs->pos += 64) > bs->limit)
    {
        return 0;
    }

    next = *p++ & (255 >> s); //从指针 p 指向的当前字节开始读取数据，根据 s 的值掩码处理，获取当前字节中未被读取的部分。

    while ((shl -= 8) > 0)
    {
        cache |= next << shl;
        next = *p++;
    }

    return cache | (next >> -shl);
}

static inline uint64_t u_bit_stream_get_bits(bit_stream_t* bs, int n)
{
    /**
     * s = bs->pos & 7: 表示pos中不足一个byte的位数
    */
    uint64_t next, cache = 0, s = bs->pos & 7;
    int shl = n + s;
    const uint8_t *p = bs->buf + (bs->pos >> 3);

    if ((bs->pos += n) > bs->limit)
    {
        return 0;
    }

    next = *p++ & (255 >> s); //从指针 p 指向的当前字节开始读取数据，根据 s 的值掩码处理，获取当前字节中未被读取的部分。

    while ((shl -= 8) > 0)
    {
        cache |= next << shl;
        next = *p++;
    }

    return cache | (next >> -shl);
}

static inline void u_bit_stream_skip_bits(bit_stream_t* bs, int n)
{
    bs->pos += n;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif //_BS_H_