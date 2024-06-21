//来自ffmpeg libutil模块的error.h文件

#ifndef _U_ERROR_H_
#define _U_ERROR_H_
#include <errno.h>
#include <stddef.h>

/* error handling 错误处理 */
//done
#if EDOM > 0
#define U_ERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
                            ///< 从 POSIX 错误代码返回负错误代码，以从库函数返回。
#define U_UNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
                            ///< 从库函数错误返回值返回 POSIX 错误代码。
#else
/* Some platforms have E* and errno already negated.
 * 有些平台的 E* 和 errno 已经被否定。
 */
#define U_ERROR(e) (e)
#define U_UNERROR(e) (e)
#endif

#endif // end of #ifndef _U_ERROR_H_