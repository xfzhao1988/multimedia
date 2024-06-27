#include <stdint.h>

#include "u_bit_stream.h"
#include "u_byte_stream.h"
#include "dbg.h"

#include "util_example.h"

void u_util_example_bit_stream(void)
{
    bit_stream_t bs = {0};
    uint8_t buff[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    u_bit_stream_init(&bs, buff, sizeof(buff));

    DBG_INFO("%ld\n", u_bit_stream_get_bits(&bs, 8));
}
