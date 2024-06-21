//来自ffmpeg libutil模块的intreadwrite.h文件
#ifndef _U_INTERNAL_RW_H_
#define _U_INTERNAL_RW_H_

#include <stdint.h>
#include "u_attributes.h"
#include "u_byte_swap.h"

typedef union {
    uint64_t u64;
    uint32_t u32[2];
    uint16_t u16[4];
    uint8_t  u8 [8];
    double   f64;
    float    f32[2];
} u_alias u_alias64;

typedef union {
    uint32_t u32;
    uint16_t u16[2];
    uint8_t  u8 [4];
    float    f32;
} u_alias u_alias32;

typedef union {
    uint16_t u16;
    uint8_t  u8 [2];
} u_alias u_alias16;

#if defined(__GNUC__)
union unaligned_64 { uint64_t l; } __attribute__((packed)) u_alias;
union unaligned_32 { uint32_t l; } __attribute__((packed)) u_alias;
union unaligned_16 { uint16_t l; } __attribute__((packed)) u_alias;

#define U_RN(s, p) (((const union unaligned_##s *) (p))->l)
#define U_WN(s, p, v) ((((union unaligned_##s *) (p))->l) = (v))

#else

#ifndef U_RB16
#   define U_RB16(x)                            \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif
#ifndef U_WB16
#   define U_WB16(p, val) do {                  \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[1] = (d);               \
        ((uint8_t*)(p))[0] = (d)>>8;            \
    } while(0)
#endif

#ifndef U_RL16
#   define U_RL16(x)                            \
    ((((const uint8_t*)(x))[1] << 8) |          \
      ((const uint8_t*)(x))[0])
#endif
#ifndef U_WL16
#   define U_WL16(p, val) do {                  \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
    } while(0)
#endif

#ifndef U_RB32
#   define U_RB32(x)                                 \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])
#endif
#ifndef U_WB32
#   define U_WB32(p, val) do {                  \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

#ifndef U_RL32
#   define U_RL32(x)                                 \
    (((uint32_t)((const uint8_t*)(x))[3] << 24) |    \
               (((const uint8_t*)(x))[2] << 16) |    \
               (((const uint8_t*)(x))[1] <<  8) |    \
                ((const uint8_t*)(x))[0])
#endif
#ifndef U_WL32
#   define U_WL32(p, val) do {                  \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
    } while(0)
#endif

#ifndef U_RB64
#   define U_RB64(x)                                   \
    (((uint64_t)((const uint8_t*)(x))[0] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[1] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[6] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[7])
#endif
#ifndef U_WB64
#   define U_WB64(p, val) do {                 \
        uint64_t d = (val);                     \
        ((uint8_t*)(p))[7] = (d);               \
        ((uint8_t*)(p))[6] = (d)>>8;            \
        ((uint8_t*)(p))[5] = (d)>>16;           \
        ((uint8_t*)(p))[4] = (d)>>24;           \
        ((uint8_t*)(p))[3] = (d)>>32;           \
        ((uint8_t*)(p))[2] = (d)>>40;           \
        ((uint8_t*)(p))[1] = (d)>>48;           \
        ((uint8_t*)(p))[0] = (d)>>56;           \
    } while(0)
#endif

#ifndef U_RL64
#   define U_RL64(x)                                   \
    (((uint64_t)((const uint8_t*)(x))[7] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[6] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[1] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[0])
#endif
#ifndef U_WL64
#   define U_WL64(p, val) do {                 \
        uint64_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
        ((uint8_t*)(p))[4] = (d)>>32;           \
        ((uint8_t*)(p))[5] = (d)>>40;           \
        ((uint8_t*)(p))[6] = (d)>>48;           \
        ((uint8_t*)(p))[7] = (d)>>56;           \
    } while(0)
#endif

#if U_HAVE_BIGENDIAN
#   define U_RN(s, p)    U_RB##s(p)
#   define U_WN(s, p, v) U_WB##s(p, v)
#else
#   define U_RN(s, p)    U_RL##s(p)
#   define U_WN(s, p, v) U_WL##s(p, v)
#endif

#endif // end of #if defined(__GNUC__)

#ifndef U_RN16
#   define U_RN16(p) U_RN(16, p)
#endif

#ifndef U_RN32
#   define U_RN32(p) U_RN(32, p)
#endif

#ifndef U_RN64
#   define U_RN64(p) U_RN(64, p)
#endif

#ifndef U_WN16
#   define U_WN16(p, v) U_WN(16, p, v)
#endif

#ifndef U_WN32
#   define U_WN32(p, v) U_WN(32, p, v)
#endif

#ifndef U_WN64
#   define U_WN64(p, v) U_WN(64, p, v)
#endif

#if U_HAVE_BIGENDIAN
#   define U_RB(s, p)    U_RN##s(p)
#   define U_WB(s, p, v) U_WN##s(p, v)
#   define U_RL(s, p)    u_b_swap##s(U_RN##s(p))
#   define U_WL(s, p, v) U_WN##s(p, u_b_swap##s(v))
#else
#   define U_RB(s, p)    u_b_swap##s(U_RN##s(p))
#   define U_WB(s, p, v) U_WN##s(p, u_b_swap##s(v))
#   define U_RL(s, p)    U_RN##s(p)
#   define U_WL(s, p, v) U_WN##s(p, v)
#endif

#define U_RB8(x)     (((const uint8_t*)(x))[0])
#define U_WB8(p, d)  do { ((uint8_t*)(p))[0] = (d); } while(0)

#define U_RL8(x)     U_RB8(x)
#define U_WL8(p, d)  U_WB8(p, d)

#ifndef U_RB16
#   define U_RB16(p)    U_RB(16, p)
#endif
#ifndef U_WB16
#   define U_WB16(p, v) U_WB(16, p, v)
#endif

#ifndef U_RL16
#   define U_RL16(p)    U_RL(16, p)
#endif
#ifndef U_WL16
#   define U_WL16(p, v) U_WL(16, p, v)
#endif

#ifndef U_RB32
#   define U_RB32(p)    U_RB(32, p)
#endif
#ifndef U_WB32
#   define U_WB32(p, v) U_WB(32, p, v)
#endif

#ifndef U_RL32
#   define U_RL32(p)    U_RL(32, p)
#endif
#ifndef U_WL32
#   define U_WL32(p, v) U_WL(32, p, v)
#endif

#ifndef U_RB64
#   define U_RB64(p)    U_RB(64, p)
#endif
#ifndef U_WB64
#   define U_WB64(p, v) U_WB(64, p, v)
#endif

#ifndef U_RL64
#   define U_RL64(p)    U_RL(64, p)
#endif
#ifndef U_WL64
#   define U_WL64(p, v) U_WL(64, p, v)
#endif

#ifndef U_RB24
#   define U_RB24(x)                            \
    ((((const uint8_t*)(x))[0] << 16) |         \
     (((const uint8_t*)(x))[1] <<  8) |         \
      ((const uint8_t*)(x))[2])
#endif
#ifndef U_WB24
#   define U_WB24(p, d) do {                    \
        ((uint8_t*)(p))[2] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[0] = (d)>>16;           \
    } while(0)
#endif

#ifndef U_RL24
#   define U_RL24(x)                            \
    ((((const uint8_t*)(x))[2] << 16) |         \
     (((const uint8_t*)(x))[1] <<  8) |         \
      ((const uint8_t*)(x))[0])
#endif
#ifndef U_WL24
#   define U_WL24(p, d) do {                    \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
    } while(0)
#endif

#ifndef U_RB48
#   define U_RB48(x)                                      \
    (((uint64_t)((const uint8_t*)(x))[0] << 40) |         \
     ((uint64_t)((const uint8_t*)(x))[1] << 32) |         \
     ((uint64_t)((const uint8_t*)(x))[2] << 24) |         \
     ((uint64_t)((const uint8_t*)(x))[3] << 16) |         \
     ((uint64_t)((const uint8_t*)(x))[4] <<  8) |         \
      (uint64_t)((const uint8_t*)(x))[5])
#endif
#ifndef U_WB48
#   define U_WB48(p, darg) do {                 \
        uint64_t d = (darg);                    \
        ((uint8_t*)(p))[5] = (d);               \
        ((uint8_t*)(p))[4] = (d)>>8;            \
        ((uint8_t*)(p))[3] = (d)>>16;           \
        ((uint8_t*)(p))[2] = (d)>>24;           \
        ((uint8_t*)(p))[1] = (d)>>32;           \
        ((uint8_t*)(p))[0] = (d)>>40;           \
    } while(0)
#endif

#ifndef U_RL48
#   define U_RL48(x)                                      \
    (((uint64_t)((const uint8_t*)(x))[5] << 40) |         \
     ((uint64_t)((const uint8_t*)(x))[4] << 32) |         \
     ((uint64_t)((const uint8_t*)(x))[3] << 24) |         \
     ((uint64_t)((const uint8_t*)(x))[2] << 16) |         \
     ((uint64_t)((const uint8_t*)(x))[1] <<  8) |         \
      (uint64_t)((const uint8_t*)(x))[0])
#endif
#ifndef U_WL48
#   define U_WL48(p, darg) do {                 \
        uint64_t d = (darg);                    \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
        ((uint8_t*)(p))[4] = (d)>>32;           \
        ((uint8_t*)(p))[5] = (d)>>40;           \
    } while(0)
#endif

/*
 * The U_[RW]NA macros access naturally aligned data
 * in a type-safe way.
 * U_[RW]NA 宏以类型安全的方式访问自然对齐的数据。
 */
#define U_RNA(s, p)    (((const av_alias##s*)(p))->u##s)
#define U_WNA(s, p, v) (((av_alias##s*)(p))->u##s = (v))

#ifndef U_RN16A
#   define U_RN16A(p) U_RNA(16, p)
#endif

#ifndef U_RN32A
#   define U_RN32A(p) U_RNA(32, p)
#endif

#ifndef U_RN64A
#   define U_RN64A(p) U_RNA(64, p)
#endif

#ifndef U_WN16A
#   define U_WN16A(p, v) U_WNA(16, p, v)
#endif

#ifndef U_WN32A
#   define U_WN32A(p, v) U_WNA(32, p, v)
#endif

#ifndef U_WN64A
#   define U_WN64A(p, v) U_WNA(64, p, v)
#endif

#if U_HAVE_BIGENDIAN
#   define U_RLA(s, p)    u_b_swap##s(U_RN##s##A(p))
#   define U_WLA(s, p, v) U_WN##s##A(p, u_b_swap##s(v))
#else
#   define U_RLA(s, p)    U_RN##s##A(p)
#   define U_WLA(s, p, v) U_WN##s##A(p, v)
#endif

#ifndef U_RL64A
#   define U_RL64A(p) U_RLA(64, p)
#endif
#ifndef U_WL64A
#   define U_WL64A(p, v) U_WLA(64, p, v)
#endif

/*
 * The U_COPYxxU macros are suitable for copying data to/from unaligned
 * memory locations.
 * U_COPYxxU 宏适用于从未对齐的内存位置复制数据或将数据复制到未对齐的内存位置。
 */
#define U_COPYU(n, d, s) U_WN##n(d, U_RN##n(s));

#ifndef U_COPY16U
#   define U_COPY16U(d, s) U_COPYU(16, d, s)
#endif

#ifndef U_COPY32U
#   define U_COPY32U(d, s) U_COPYU(32, d, s)
#endif

#ifndef U_COPY64U
#   define U_COPY64U(d, s) U_COPYU(64, d, s)
#endif

#ifndef U_COPY128U
#   define U_COPY128U(d, s)                                    \
    do {                                                       \
        U_COPY64U(d, s);                                       \
        U_COPY64U((char *)(d) + 8, (const char *)(s) + 8);     \
    } while(0)
#endif

/* Parameters for U_COPY*, U_SWAP*, U_ZERO* must be
 * naturally aligned. They may be implemented using MMX,
 * so emms_c() must be called before using any float code
 * afterwards.
 * U_COPY*、U_SWAP*、U_ZERO* 的参数必须自然对齐。它们可能使用 MMX 实现，因此在使用任何浮点
 * 代码之前必须调用 emms_c()。
 */

#define U_COPY(n, d, s) \
    (((av_alias##n*)(d))->u##n = ((const av_alias##n*)(s))->u##n)

#ifndef U_COPY16
#   define U_COPY16(d, s) U_COPY(16, d, s)
#endif

#ifndef U_COPY32
#   define U_COPY32(d, s) U_COPY(32, d, s)
#endif

#ifndef U_COPY64
#   define U_COPY64(d, s) U_COPY(64, d, s)
#endif

#ifndef U_COPY128
#   define U_COPY128(d, s)                    \
    do {                                      \
        U_COPY64(d, s);                       \
        U_COPY64((char*)(d)+8, (char*)(s)+8); \
    } while(0)
#endif

#define U_SWAP(n, a, b) UM_SWAP(u_alias##n, *(u_alias##n*)(a), *(u_alias##n*)(b))

#ifndef U_SWAP64
#   define U_SWAP64(a, b) U_SWAP(64, a, b)
#endif

#define U_ZERO(n, d) (((av_alias##n*)(d))->u##n = 0)

#ifndef U_ZERO16
#   define U_ZERO16(d) U_ZERO(16, d)
#endif

#ifndef U_ZERO32
#   define U_ZERO32(d) U_ZERO(32, d)
#endif

#ifndef U_ZERO64
#   define U_ZERO64(d) U_ZERO(64, d)
#endif

#ifndef U_ZERO128
#   define U_ZERO128(d)         \
    do {                         \
        U_ZERO64(d);            \
        U_ZERO64((char*)(d)+8); \
    } while(0)
#endif


#endif // end of #ifndef _U_INTERNAL_RW_H_