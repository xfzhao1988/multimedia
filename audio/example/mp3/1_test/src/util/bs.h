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

extern void bs_init(bs_t *bs, const uint8_t *data, int bytes);

extern uint64_t get_bits(bs_t *bs, int n);

extern void skip_bits(bs_t *bs, int n);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif //_BS_H_