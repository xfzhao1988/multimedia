#include <stdint.h>

#include "bs.h"


void bs_init(bs_t* bs, const uint8_t* data, int bytes)
{
    bs->buf   = data;
    bs->pos   = 0;
    bs->limit = bytes * 8;
}

uint64_t get_bits(bs_t* bs, int n)
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

void skip_bits(bs_t* bs, int n)
{
    bs->pos += n;
}