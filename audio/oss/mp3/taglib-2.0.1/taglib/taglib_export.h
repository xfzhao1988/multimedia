/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_EXPORT_H
#define TAGLIB_EXPORT_H

#if defined(TAGLIB_STATIC)
#define TAGLIB_EXPORT
#elif (defined(_WIN32) || defined(_WIN64))
#ifdef MAKE_TAGLIB_LIB
/**
 * 这段代码定义了一个宏 TAGLIB_EXPORT，使用了 Microsoft Visual C++ 编译器的 __declspec(dllexport) 扩展，
 * 用于指定符号的导出。在 Windows 平台上，这个扩展用于指定共享库中的符号可以被外部程序引用。
 *
 * 具体来说，__declspec(dllexport) 告诉编译器将相应的符号标记为导出符号，这样它们就可以被外部程序引用。
 *
 * 例如，假设你有一个名为 MyClass 的类，并且你想要在共享库中导出它，你可以使用 TAGLIB_EXPORT 宏来标记这个类的导出：
 *
 * class TAGLIB_EXPORT MyClass {
        // 类定义
   };
 *
 * 在这个例子中，MyClass 类被标记为 TAGLIB_EXPORT，这意味着它的符号会被标记为导出符号，以便可以在共享库之外被外部程序引用。
 *
*/
#define TAGLIB_EXPORT __declspec(dllexport)
#else
/**
 * 这段代码定义了一个宏 TAGLIB_EXPORT，使用了 Microsoft Visual C++ 编译器的 __declspec(dllimport) 扩展，
 * 用于指定符号的导入。在 Windows 平台上，这个扩展用于指定共享库中的符号可以被外部程序引用。
 *
 * 具体来说，__declspec(dllimport) 告诉编译器将相应的符号标记为导入符号，这样它们就可以在程序中被引用。
 *
 * 例如，当你在一个程序中引用另一个共享库中导出的符号时，你可以使用 TAGLIB_EXPORT 宏来标记这些符号的导入：
 *
 * TAGLIB_EXPORT int someFunction(); // 声明共享库中导出的函数
 *
 * 在这个例子中，TAGLIB_EXPORT 宏用于声明共享库中导出的 someFunction 函数。在编译时，编译器将查找共享库中的符号并进行相应的链接。
*/
#define TAGLIB_EXPORT __declspec(dllimport)
#endif
#elif defined(__GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 1)
/**
 * 这段代码使用了 GCC/Clang 的 __attribute__ 扩展，用于指定符号的可见性。在这里，TAGLIB_EXPORT 被定义为一个宏，
 * 它使用 visibility("default") 属性，将相应的符号标记为默认可见性。
 *
 * 具体来说，__attribute__((visibility("default"))) 告诉编译器将相应的符号设置为默认可见性，这意味着它们可以
 * 被动态链接到其他目标文件中。这在创建共享库时特别有用，因为它确保导出的符号可以在库之外被其他程序使用。
 *
 * 例如，假设你有一个名为 MyClass 的类，并且你想要在共享库中导出它，你可以使用 TAGLIB_EXPORT 宏来标记这个类的导出：
 *
 * class TAGLIB_EXPORT MyClass {
        // 类定义
   };
 *
 * 在这个例子中，MyClass 类被标记为 TAGLIB_EXPORT，这意味着它的符号会被设置为默认可见性，以便可以在共享库之外使用。
 *
*/
#define TAGLIB_EXPORT __attribute__ ((visibility("default")))
#else
#define TAGLIB_EXPORT
#endif

#if defined _MSC_VER && !defined TAGLIB_STATIC
/*!
 * Suppress MSVC C4251 warning for next statement.
 * Unfortunately, MSVC exports everything (not only public members) when
 * __declspec(dllexport) is set at the class level via TAGLIB_EXPORT, which
 * leads to many "needs to have dll-interface to be used by clients" C4251
 * warnings issued by MSVC, because the std::unique_ptr pimpls are
 * exported too. This macro can be used before private STL fields to suppress
 * such warnings.
 * 抑制下一个语句的 MSVC C4251 警告。
 * 不幸的是，当通过 TAGLIB_EXPORT 在类级别设置 __declspec(dllexport) 时，MSVC 会导出所有
 * 内容（不仅仅是公共成员），这会导致 MSVC 发出许多“需要有 dll 接口供客户端使用”C4251 警告，
 * 因为 std::unique_ptr pimpls 也被导出。可以在私有 STL 字段之前使用此宏来抑制此类警告。
*/
/**
 * 这段代码定义了一个宏 TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE，
 * 它使用了 _Pragma 操作符，具体指令是 "warning(suppress: 4251)"，这是 Microsoft Visual C++
 * 编译器的一个特殊指令，用于抑制特定的编译器警告。
 *
 * 在这里，4251 是一个警告码，通常表示“需要导出的类成员模板”或“需要导出的成员变量”。这种情况经常
 * 出现在使用 __declspec(dllexport) 或 __declspec(dllimport) 标记导出类或结构时，如果类或结构中
 * 包含模板成员或成员变量，则会触发警告。
 *
 * 因此，_Pragma("warning(suppress: 4251)") 指令被用来抑制这个特定的警告，以避免在编译过程中产生这个警告。
*/
#define TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE _Pragma("warning(suppress: 4251)")
#else
#define TAGLIB_MSVC_SUPPRESS_WARNING_NEEDS_TO_HAVE_DLL_INTERFACE
#endif

#endif
