//来自ffmpeg libutil模块的common.h文件

/**
 * UC: 表示util common的缩写
*/

/**
 * @file
 * common internal and external API header
 */

#ifndef _U_COMMON_H_
#define _U_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "u_attributes.h"
#include "u_macros.h"

/**
 * Absolute value, Note, INT_MIN / INT64_MIN result in undefined behavior as they
 * are not representable as absolute values of their type. This is the same
 * as with *abs()
 * 绝对值，注意，INT_MIN / INT64_MIN 会导致未定义的行为，因为它们不能表示为其类型的绝对值。
 * 这与 *abs() 相同。
 */
//done
#define UC_ABS(a) ((a) >= 0 ? (a) : (-(a)))
#define UC_SIGN(a) ((a) > 0 ? 1 : -1)

/**
 * Negative Absolute value.
 * this works for all integers of all types.
 * As with many macros, this evaluates its argument twice, it thus must not have
 * a sideeffect, that is FFNABS(x++) has undefined behavior.
 */
#define UC_NABS(a) ((a) <= 0 ? (a) : (-(a)))

/* misc math functions */

#ifndef uc_clip
#   define uc_clip         uc_clip_c
#endif


/**
 * Clip a signed integer value into the amin-amax range.
 * 将有符号整数值剪辑到 amin-amax 范围内。
 * @param a value to clip
 * @param amin minimum value of the clip range
 * @param amax maximum value of the clip range
 * @return clipped value
 */
static u_always_inline u_const int uc_clip_c(int a, int amin, int amax)
{
#if defined(HAVE_U_CONFIG_H) && defined(ASSERT_LEVEL) && ASSERT_LEVEL >= 2
    if (amin > amax) abort();
#endif
    if      (a < amin) return amin;
    else if (a > amax) return amax;
    else               return a;
}

#endif // end of #ifndef _U_COMMON_H_