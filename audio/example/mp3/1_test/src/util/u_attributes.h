//来自ffmpeg libutil模块的attributes.h文件
#ifndef _U_ATTRIBUTES_H_
#define _U_ATTRIBUTES_H_

#ifdef __GNUC__
/**
 * @brief 这段代码定义了一个宏 U_GCC_VERSION_AT_LEAST(x,y)，用于检查编译器是否至少是指定版本的 GCC。
 *
 * 具体来说：
 * __GNUC__ 和 __GNUC_MINOR__ 是 GCC 编译器预定义的宏，分别表示当前编译器的主版本号和次版本号。
 *
 * __GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y)：这个表达式用于判断当前
 * GCC 编译器版本是否大于等于指定的版本 (x.y)。如果当前 GCC 编译器的主版本号大于 x，则直接返回真，
 * 如果主版本号等于 x，则比较次版本号是否大于等于 y。
 *
 * 例如，如果你希望检查编译器是否至少是 GCC 4.8 版本，你可以这样使用这个宏：
 * #if U_GCC_VERSION_AT_LEAST(4,8)
 *   // 当前 GCC 版本至少是 4.8
 * #else
 *   // 当前 GCC 版本不满足要求
 * #endif
 *
 * 这个宏在编写跨平台代码时特别有用，可以根据 GCC 编译器的版本来选择不同的实现方式或者处理一些特定的问题。
 *
 */
#    define U_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
/**
 * @brief 这段代码定义了一个宏 U_GCC_VERSION_AT_MOST(x,y)，用于检查编译器是否最多是指定版本的 GCC。
 *
 * 具体来说：
 *
 * __GNUC__ 和 __GNUC_MINOR__ 是 GCC 编译器预定义的宏，分别表示当前编译器的主版本号和次版本号。
 *
 * __GNUC__ < (x) || __GNUC__ == (x) && __GNUC_MINOR__ <= (y)：这个表达式用于判断当前 GCC 编译器版本
 * 是否小于等于指定的版本 (x.y)。如果当前 GCC 编译器的主版本号小于 x，则直接返回真，如果主版本号等于 x，
 * 则比较次版本号是否小于等于 y。
 *
 * 例如，如果你希望检查编译器是否最多是 GCC 4.8 版本，你可以这样使用这个宏：
 * #if U_GCC_VERSION_AT_MOST(4,8)
 *   // 当前 GCC 版本最多是 4.8
 * #else
 *   // 当前 GCC 版本不满足要求
 * #endif
 *
 * 这个宏在编写跨平台代码时特别有用，可以根据 GCC 编译器的版本来选择不同的实现方式或者处理一些特定的问题。
 *
 */
#    define U_GCC_VERSION_AT_MOST(x,y)  (__GNUC__ < (x) || __GNUC__ == (x) && __GNUC_MINOR__ <= (y))
#else
#    define U_GCC_VERSION_AT_LEAST(x,y) 0
#    define U_GCC_VERSION_AT_MOST(x,y)  0
#endif

#ifndef u_always_inline
#if U_GCC_VERSION_AT_LEAST(3,1)
/**
 * @brief 这是一个在C/C++中常用的宏定义，特别是在对性能要求较高的系统中使用。
 *
 * 这里是各部分的中文解释：
 *
 * #define：这是C和C++中的预处理指令，用于定义宏。
 *
 * av_always_inline：这是被定义的宏的名称。这似乎是一个自定义的名称，可能用于特定的项目或库
 * （例如FFmpeg经常使用前缀“av”来表示与音视频相关的函数）。
 *
 * __attribute__((always_inline))：这是GCC编译器的一个属性。它向编译器建议，无论编译器的
 * 优化设置如何，都应该总是将函数内联。内联函数可以通过消除函数调用的开销来提高执行速度。
 *
 * inline：这是C/C++中的一个关键字。它提示编译器应该尝试将函数的代码嵌入到每个调用函数的点，
 * 而不是管理函数调用和返回。这是另一种优化性能的方式，但这只是一个建议，编译器可能选择不内联函数。
 *
 * 通过组合这些，av_always_inline 被定义为任何用它声明的函数将被建议编译器内联，且由于 always_inline
 * 属性的强烈推荐，编译器被强烈建议这样做。这在性能关键的代码中非常有用，其中必须最小化函数调用的开销。
 *
 */
#    define u_always_inline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#    define u_always_inline __forceinline
#else
#    define u_always_inline inline
#endif
#endif

/**
 * @brief 这里是这个宏定义的各个部分的解释：
 *
 * #define：这是 C 和 C++ 中的一个预处理指令，用于定义宏。
 *
 * av_const：这是被定义的宏的名称。看起来这是一个自定义的名称，可能是用在特定的项目或库中，
 * 比如像 FFmpeg 这样的音视频处理库。
 *
 * __attribute__((const))：这是 GCC 编译器的一个属性，它告诉编译器这个函数或变量具有“const”属性。
 * 这意味着函数满足以下条件：
 * 函数不会修改任何不通过指针参数访问的内存区域。
 * 函数对于相同的输入总是返回相同的结果。
 *
 * 这个属性非常有用，因为它允许编译器进行更积极的优化，例如删除重复的函数调用（如果函数多次使用
 * 相同的参数进行调用），并将结果替换为第一次调用的结果。这样可以显著提高程序的执行效率。
 *
 */
#if U_GCC_VERSION_AT_LEAST(2,6) || defined(__clang__)
#    define u_const __attribute__((const))
#else
#    define u_const
#endif

#if U_GCC_VERSION_AT_LEAST(3,3) || defined(__clang__)
#   define u_alias __attribute__((may_alias))
#else
#   define u_alias
#endif

#endif // end of #ifndef _U_ATTRIBUTES_H_