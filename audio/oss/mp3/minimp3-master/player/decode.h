/**
#pragma once 是一种用于防止头文件被多次包含的预处理指令，通常用于 C 和 C++ 项目中。它的作用相当于传统的
头文件保护（include guards），但更简洁。

用法
在你的头文件中，只需在文件的顶部添加 #pragma once：

#pragma once
// 头文件内容
class MyClass {
public:
    void myFunction();
};

传统的头文件保护（Include Guards）

在引入 #pragma once 之前，头文件保护通常使用条件编译指令 #ifndef、#define 和 #endif 来实现：
#ifndef MYCLASS_H
#define MYCLASS_H
// 头文件内容
class MyClass {
public:
    void myFunction();
};

#endif // MYCLASS_H

两者的对比

优点
简洁：#pragma once 更简洁，不需要定义唯一的宏名。
避免错误：使用 include guards 需要手动定义宏名，容易出错或冲突，而 #pragma once 不存在这个问题。

缺点
兼容性：#pragma once 不是 C/C++ 标准的一部分，虽然大多数现代编译器都支持它，但在极少数情况下可能会遇到不支持的编译器。
文件系统依赖：#pragma once 依赖于文件系统，如果存在符号链接或硬链接，可能会导致意外行为。

适用场景
在大多数情况下，使用 #pragma once 是一个不错的选择，尤其是在现代开发环境中。它可以简化代码，减少错误。然而，
如果你需要兼容一些非常旧的编译器，或者你的项目有特殊的文件系统要求，可能需要使用传统的 include guards。

总之，#pragma once 提供了一种简洁和可靠的方法来防止头文件被多次包含，大多数情况下推荐使用它。
 */
#pragma once //解释如上
#include <stdint.h>
#include "../minimp3_ex.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef int (*PARSE_GET_FILE_CB)(void *user, char **file_name);
typedef int (*PARSE_INFO_CB)(void *user, char *file_name, int rate, int mp3_channels, float duration);

typedef struct decoder
{
    mp3dec_ex_t mp3d;
    float mp3_duration;
    float spectrum[32][2]; // for visualization 用于可视化
} decoder;

extern decoder _dec;

int open_dec(decoder *dec, const char *file_name);
int close_dec(decoder *dec);
void decode_samples(decoder *dec, uint8_t *buf, int bytes);

#ifdef __cplusplus
}
#endif
