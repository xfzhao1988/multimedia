//来自ffmpeg libutil模块的bytestream.h文件
#ifndef _U_BYTE_STREAM_H_
#define _U_BYTE_STREAM_H_

#include <stdint.h>

typedef struct
{
    const uint8_t *buffer;
    const uint8_t *buffer_end;
    const uint8_t *buffer_start;
} get_byte_context_t;

typedef struct{
    uint8_t *buffer;
    uint8_t *buffer_end;
    uint8_t *buffer_start;
    int eof;
} put_byte_context_t;

#define DEF(type, name, bytes, read, write)                                    \
static inline type u_byte_stream_get_ ## name(const uint8_t **b)        \
{                                                                              \
    (*b) += bytes;                                                             \
    return read(*b - bytes);                                                   \
}                                                                              \
static av_always_inline void bytestream_put_ ## name(uint8_t **b,              \
                                                     const type value)         \
{                                                                              \
    write(*b, value);                                                          \
    (*b) += bytes;                                                             \
}                                                                              \
static av_always_inline void bytestream2_put_ ## name ## u(PutByteContext *p,  \
                                                           const type value)   \
{                                                                              \
    bytestream_put_ ## name(&p->buffer, value);                                \
}                                                                              \
static av_always_inline void bytestream2_put_ ## name(PutByteContext *p,       \
                                                      const type value)        \
{                                                                              \
    if (!p->eof && (p->buffer_end - p->buffer >= bytes)) {                     \
        write(p->buffer, value);                                               \
        p->buffer += bytes;                                                    \
    } else                                                                     \
        p->eof = 1;                                                            \
}                                                                              \
static av_always_inline type bytestream2_get_ ## name ## u(GetByteContext *g)  \
{                                                                              \
    return bytestream_get_ ## name(&g->buffer);                                \
}                                                                              \
static av_always_inline type bytestream2_get_ ## name(GetByteContext *g)       \
{                                                                              \
    if (g->buffer_end - g->buffer < bytes) {                                   \
        g->buffer = g->buffer_end;                                             \
        return 0;                                                              \
    }                                                                          \
    return bytestream2_get_ ## name ## u(g);                                   \
}                                                                              \
static av_always_inline type bytestream2_peek_ ## name ## u(GetByteContext *g) \
{                                                                              \
    return read(g->buffer);                                                    \
}                                                                              \
static av_always_inline type bytestream2_peek_ ## name(GetByteContext *g)      \
{                                                                              \
    if (g->buffer_end - g->buffer < bytes)                                     \
        return 0;                                                              \
    return bytestream2_peek_ ## name ## u(g);                                  \
}

#endif //_U_BYTE_STREAM_H_