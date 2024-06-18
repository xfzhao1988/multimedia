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
} bs_t;

void bs_init(bs_t *bs, const uint8_t *data, int bytes);

uint8_t get_bits_u8(bs_t* bs);

uint16_t get_bits_u16(bs_t* bs);

uint32_t get_bits_u32(bs_t* bs);

uint64_t get_bits_u64(bs_t* bs);

uint64_t get_bits(bs_t *bs, int n);

void skip_bits(bs_t *bs, int n);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif //_BS_H_