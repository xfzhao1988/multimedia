#include <stdint.h>

#include "u_bit_stream.h"
#include "u_byte_stream.h"
#include "dbg.h"

#include "util_example.h"

void u_util_example_bit_stream(void)
{
    uint32_t val = 0x55; //0101 0101
    bit_stream_t bs = {0};
    uint8_t* buff = (uint8_t*)&val;

    u_bit_stream_init(&bs, buff, sizeof(val));

    DBG_INFO("%ld\n", u_bit_stream_get_bits(&bs, 1));
}
