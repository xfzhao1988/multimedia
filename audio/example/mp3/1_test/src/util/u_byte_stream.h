//来自ffmpeg libutil模块的bytestream.h文件
#ifndef _U_BYTE_STREAM_H_
#define _U_BYTE_STREAM_H_

#include <stdint.h>
#include <string.h>

#include "u_internal_rw.h"
#include "u_assert.h"
#include "u_common.h"
#include "u_error.h"

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

#define DEF(type, name, bytes, read, write)                                             \
static u_always_inline type u_byte_stream_get_dep_ ## name(const uint8_t **b)           \
{                                                                                       \
    (*b) += bytes;                                                                      \
    return read(*b - bytes);                                                            \
}                                                                                       \
static u_always_inline void u_byte_stream_put_dep_ ## name(uint8_t **b,                 \
                                                  const type value)                     \
{                                                                                       \
    write(*b, value);                                                                   \
    (*b) += bytes;                                                                      \
}                                                                                       \
static u_always_inline void u_byte_stream_put_ ## name ## u(put_byte_context_t *p,      \
                                                   const type value)                    \
{                                                                                       \
    u_byte_stream_put_dep ## name(&p->buffer, value);                                   \
}                                                                                       \
static u_always_inline void u_byte_stream_put_ ## name(put_byte_context_t *p,           \
                                              const type value)                         \
{                                                                                       \
    if (!p->eof && (p->buffer_end - p->buffer >= bytes)) {                              \
        write(p->buffer, value);                                                        \
        p->buffer += bytes;                                                             \
    } else                                                                              \
        p->eof = 1;                                                                     \
}                                                                                       \
static u_always_inline type u_byte_stream_get_ ## name ## u(get_byte_context_t *g)      \
{                                                                                       \
    return u_byte_stream_get_dep ## name(&g->buffer);                                   \
}                                                                                       \
static u_always_inline type u_byte_stream_get_ ## name(get_byte_context_t *g)           \
{                                                                                       \
    if (g->buffer_end - g->buffer < bytes) {                                            \
        g->buffer = g->buffer_end;                                                      \
        return 0;                                                                       \
    }                                                                                   \
    return u_byte_stream_get_ ## name ## u(g);                                          \
}                                                                                       \
static u_always_inline type u_byte_stream_peek_ ## name ## u(get_byte_context_t *g)     \
{                                                                                       \
    return read(g->buffer);                                                             \
}                                                                                       \
static u_always_inline type u_byte_stream_peek_ ## name(get_byte_context_t *g)          \
{                                                                                       \
    if (g->buffer_end - g->buffer < bytes)                                              \
        return 0;                                                                       \
    return u_byte_stream_peek_ ## name ## u(g);                                         \
}

DEF(uint64_t,     le64, 8, U_RL64, U_WL64)
DEF(unsigned int, le32, 4, U_RL32, U_WL32)
DEF(unsigned int, le24, 3, U_RL24, U_WL24)
DEF(unsigned int, le16, 2, U_RL16, U_WL16)
DEF(uint64_t,     be64, 8, U_RB64, U_WB64)
DEF(unsigned int, be32, 4, U_RB32, U_WB32)
DEF(unsigned int, be24, 3, U_RB24, U_WB24)
DEF(unsigned int, be16, 2, U_RB16, U_WB16)
DEF(unsigned int, byte, 1, U_RB8 , U_WB8)

/**
 * Functions with the suffix u are unsafe functions and are not recommended.
 * eg: u_byte_stream_get_ne16u、u_byte_stream_get_ne24u and so on
*/
#if U_HAVE_BIGENDIAN
#   define u_byte_stream_get_ne16  u_byte_stream_get_be16
#   define u_byte_stream_get_ne24  u_byte_stream_get_be24
#   define u_byte_stream_get_ne32  u_byte_stream_get_be32
#   define u_byte_stream_get_ne64  u_byte_stream_get_be64
#   define u_byte_stream_get_ne16u u_byte_stream_get_be16u
#   define u_byte_stream_get_ne24u u_byte_stream_get_be24u
#   define u_byte_stream_get_ne32u u_byte_stream_get_be32u
#   define u_byte_stream_get_ne64u u_byte_stream_get_be64u
#   define u_byte_stream_put_ne16  u_byte_stream_put_be16
#   define u_byte_stream_put_ne24  u_byte_stream_put_be24
#   define u_byte_stream_put_ne32  u_byte_stream_put_be32
#   define u_byte_stream_put_ne64  u_byte_stream_put_be64
#   define u_byte_stream_peek_ne16 u_byte_stream_peek_be16
#   define u_byte_stream_peek_ne24 u_byte_stream_peek_be24
#   define u_byte_stream_peek_ne32 u_byte_stream_peek_be32
#   define u_byte_stream_peek_ne64 u_byte_stream_peek_be64
#else
#   define u_byte_stream_get_ne16  u_byte_stream_get_le16
#   define u_byte_stream_get_ne24  u_byte_stream_get_le24
#   define u_byte_stream_get_ne32  u_byte_stream_get_le32
#   define u_byte_stream_get_ne64  u_byte_stream_get_le64
#   define u_byte_stream_get_ne16u u_byte_stream_get_le16u
#   define u_byte_stream_get_ne24u u_byte_stream_get_le24u
#   define u_byte_stream_get_ne32u u_byte_stream_get_le32u
#   define u_byte_stream_get_ne64u u_byte_stream_get_le64u
#   define u_byte_stream_put_ne16  u_byte_stream_put_le16
#   define u_byte_stream_put_ne24  u_byte_stream_put_le24
#   define u_byte_stream_put_ne32  u_byte_stream_put_le32
#   define u_byte_stream_put_ne64  u_byte_stream_put_le64
#   define u_byte_stream_peek_ne16 u_byte_stream_peek_le16
#   define u_byte_stream_peek_ne24 u_byte_stream_peek_le24
#   define u_byte_stream_peek_ne32 u_byte_stream_peek_le32
#   define u_byte_stream_peek_ne64 u_byte_stream_peek_le64
#endif

static u_always_inline void u_byte_stream_init(get_byte_context_t *g,
                                               const uint8_t *buf,
                                               int buf_size)
{
    u_assert0(buf_size >= 0);
    g->buffer       = buf;
    g->buffer_start = buf;
    g->buffer_end   = buf + buf_size;
}

static u_always_inline void u_byte_stream_init_writer(put_byte_context_t *p,
                                             uint8_t *buf,
                                             int buf_size)
{
    u_assert0(buf_size >= 0);
    p->buffer       = buf;
    p->buffer_start = buf;
    p->buffer_end   = buf + buf_size;
    p->eof          = 0;
}

static u_always_inline int u_byte_stream_get_bytes_left(get_byte_context_t *g)
{
    return g->buffer_end - g->buffer;
}

static u_always_inline int u_byte_stream_get_bytes_left_p(put_byte_context_t *p)
{
    return p->buffer_end - p->buffer;
}

static u_always_inline void u_byte_stream_skip(get_byte_context_t *g,
                                      unsigned int size)
{
    g->buffer += UM_MIN(g->buffer_end - g->buffer, size);
}

/* Unsafe function, not recommended */
static u_always_inline void u_byte_stream_skipu(get_byte_context_t *g,
                                       unsigned int size)
{
    g->buffer += size;
}

static u_always_inline void u_byte_stream_skip_p(put_byte_context_t *p,
                                        unsigned int size)
{
    int size2;
    if (p->eof)
        return;
    size2 = UM_MIN(p->buffer_end - p->buffer, size);
    if (size2 != size)
        p->eof = 1;
    p->buffer += size2;
}

static u_always_inline int u_byte_stream_tell(get_byte_context_t *g)
{
    return (int)(g->buffer - g->buffer_start);
}

static u_always_inline int u_byte_stream_tell_p(put_byte_context_t *p)
{
    return (int)(p->buffer - p->buffer_start);
}

static u_always_inline int u_byte_stream_size(get_byte_context_t *g)
{
    return (int)(g->buffer_end - g->buffer_start);
}

static u_always_inline int u_byte_stream_size_p(put_byte_context_t *p)
{
    return (int)(p->buffer_end - p->buffer_start);
}

static u_always_inline int u_byte_stream_seek(get_byte_context_t *g,
                                              int offset,
                                              int whence)
{
    switch (whence) {
    case SEEK_CUR:
        offset     = uc_clip_c(offset, -(g->buffer - g->buffer_start),
                               g->buffer_end - g->buffer);
        g->buffer += offset;
        break;
    case SEEK_END:
        offset    = uc_clip_c(offset, -(g->buffer_end - g->buffer_start), 0);
        g->buffer = g->buffer_end + offset;
        break;
    case SEEK_SET:
        offset    = uc_clip_c(offset, 0, g->buffer_end - g->buffer_start);
        g->buffer = g->buffer_start + offset;
        break;
    default:
        return U_ERROR(EINVAL);
    }
    return u_byte_stream_tell(g);
}

static u_always_inline int u_byte_stream_seek_p(put_byte_context_t *p,
                                                int offset,
                                                int whence)
{
    p->eof = 0;
    switch (whence) {
    case SEEK_CUR:
        if (p->buffer_end - p->buffer < offset)
            p->eof = 1;
        offset     = uc_clip_c(offset, -(p->buffer - p->buffer_start),
                               p->buffer_end - p->buffer);
        p->buffer += offset;
        break;
    case SEEK_END:
        if (offset > 0)
            p->eof = 1;
        offset    = uc_clip_c(offset, -(p->buffer_end - p->buffer_start), 0);
        p->buffer = p->buffer_end + offset;
        break;
    case SEEK_SET:
        if (p->buffer_end - p->buffer_start < offset)
            p->eof = 1;
        offset    = uc_clip_c(offset, 0, p->buffer_end - p->buffer_start);
        p->buffer = p->buffer_start + offset;
        break;
    default:
        return U_ERROR(EINVAL);
    }
    return u_byte_stream_tell_p(p);
}

static u_always_inline unsigned int u_byte_stream_get_buffer(get_byte_context_t *g,
                                                             uint8_t *dst,
                                                             unsigned int size)
{
    int size2 = UM_MIN(g->buffer_end - g->buffer, size);
    memcpy(dst, g->buffer, size2);
    g->buffer += size2;
    return size2;
}

/* Unsafe function, not recommended. Please use u_byte_stream_get_buffer instead */
static u_always_inline unsigned int u_byte_stream_get_bufferu(get_byte_context_t *g,
                                                              uint8_t *dst,
                                                              unsigned int size)
{
    memcpy(dst, g->buffer, size);
    g->buffer += size;
    return size;
}

static u_always_inline unsigned int u_byte_stream_put_buffer(put_byte_context_t *p,
                                                             const uint8_t *src,
                                                             unsigned int size)
{
    int size2;
    if (p->eof)
        return 0;
    size2 = UM_MIN(p->buffer_end - p->buffer, size);
    if (size2 != size)
        p->eof = 1;
    memcpy(p->buffer, src, size2);
    p->buffer += size2;
    return size2;
}

/* Unsafe function, not recommended. Please use u_byte_stream_put_buffer instead */
static u_always_inline unsigned int u_byte_stream_put_bufferu(put_byte_context_t *p,
                                                              const uint8_t *src,
                                                              unsigned int size)
{
    memcpy(p->buffer, src, size);
    p->buffer += size;
    return size;
}

static u_always_inline void u_byte_stream_set_buffer(put_byte_context_t *p,
                                                     const uint8_t c,
                                                     unsigned int size)
{
    int size2;
    if (p->eof)
        return;
    size2 = UM_MIN(p->buffer_end - p->buffer, size);
    if (size2 != size)
        p->eof = 1;
    memset(p->buffer, c, size2);
    p->buffer += size2;
}

/* Unsafe function, not recommended. Please use u_byte_stream_set_buffer instead */
static u_always_inline void u_byte_stream_set_bufferu(put_byte_context_t *p,
                                                      const uint8_t c,
                                                      unsigned int size)
{
    memset(p->buffer, c, size);
    p->buffer += size;
}

static u_always_inline unsigned int u_byte_stream_get_eof(put_byte_context_t *p)
{
    return p->eof;
}

/* Unsafe function, not recommended. Please use u_byte_stream_copy_buffer instead */
static u_always_inline unsigned int u_byte_stream_copy_bufferu(put_byte_context_t *p,
                                                               get_byte_context_t *g,
                                                               unsigned int size)
{
    memcpy(p->buffer, g->buffer, size);
    p->buffer += size;
    g->buffer += size;
    return size;
}

static u_always_inline unsigned int u_byte_stream_copy_buffer(put_byte_context_t *p,
                                                              get_byte_context_t *g,
                                                              unsigned int size)
{
    int size2;

    if (p->eof)
        return 0;
    size  = UM_MIN(g->buffer_end - g->buffer, size);
    size2 = UM_MIN(p->buffer_end - p->buffer, size);
    if (size2 != size)
        p->eof = 1;

    return u_byte_stream_copy_bufferu(p, g, size2);
}

/* Deprecated API, not recommended */
static u_always_inline unsigned int u_byte_stream_get_buffer_dep(const uint8_t **b,
                                                                 uint8_t *dst,
                                                                 unsigned int size)
{
    memcpy(dst, *b, size);
    (*b) += size;
    return size;
}

/* Deprecated API, not recommended */
static u_always_inline void u_byte_stream_put_buffer_dep(uint8_t **b,
                                                         const uint8_t *src,
                                                         unsigned int size)
{
    memcpy(*b, src, size);
    (*b) += size;
}

#endif //end of #ifndef _U_BYTE_STREAM_H_