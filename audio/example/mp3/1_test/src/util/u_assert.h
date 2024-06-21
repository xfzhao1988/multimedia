//来自ffmpeg libutil模块的avassert.h文件

#ifndef _U_ASSERT_H_
#define _U_ASSERT_H_

#include <stdlib.h>

//todo: 需要添加avlog相对应的打印
#define u_assert0(cond) do {                                            \
    if (!(cond)) {                                                      \
        abort();                                                        \
    }                                                                   \
} while (0)

#endif // end of #ifndef _U_ASSERT_H_