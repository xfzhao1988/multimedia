# TagLib Installation
TagLib 安装

TagLib uses the CMake build system. As a user, you will most likely want to
build TagLib in release mode and install it into a system-wide location.
This can be done using the following commands:
TagLib 使用 CMake 构建系统。作为用户，您很可能希望在发布模式下构建 TagLib 并将其安装
到系统范围的位置。可以使用以下命令完成此操作：

    cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release .
    make
    sudo make install

In order to build the included examples, use the `BUILD_EXAMPLES` option:
为了构建所包含的示例，请使用“BUILD_EXAMPLES”选项：

    cmake -DBUILD_EXAMPLES=ON [...]

If you want to build TagLib without ZLib, you can use
如果你想构建没有 ZLib 的 TagLib，你可以使用

    cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release -DWITH_ZLIB=OFF .
    make
    sudo make install

See [cmake(1)](https://cmake.org/cmake/help/latest/manual/cmake.1.html) for
generic help on running CMake.
有关运行 CMake 的一般帮助，请参阅 [cmake(1)](https://cmake.org/cmake/help/latest/manual/cmake.1.html)。

## Build Options
编译选项

These are the most important build options. For details, have a look into the
CMakeLists.txt file.
这些是最重要的构建选项。有关详细信息，请查看 CMakeLists.txt 文件。

| Option                  | Description                                        |
|-------------------------|----------------------------------------------------|
| `BUILD_SHARED_LIBS`     | Build shared libraries                             |
| `CMAKE_BUILD_TYPE`      | Debug, Release, RelWithDebInfo, MinSizeRel         |
| `BUILD_EXAMPLES`        | Build examples                                     |
| `BUILD_BINDINGS`        | Build C bindings                                   |
| `BUILD_TESTING`         | Build unit tests                                   |
| `TRACE_IN_RELEASE`      | Enable debug output in release builds              |
| `WITH_ZLIB`             | Whether to build with ZLib (default ON)            |
| `ZLIB_ROOT`             | Where to find ZLib's root directory                |
| `ZLIB_INCLUDE_DIR`      | Where to find ZLib's include directory             |
| `ZLIB_LIBRARY`          | Where to find ZLib's library                       |
| `CMAKE_INSTALL_PREFIX`  | Where to install Taglib                            |
| `TAGLIB_INSTALL_SUFFIX` | Suffix added to installed libraries, includes, ... |
| `ENABLE_STATIC_RUNTIME` | Link with MSVC runtime statically                  |
| `BUILD_FRAMEWORK`       | Build a macOS framework                            |

If you want to install TagLib 2 alongside TagLib 1, you can use
`-DTAGLIB_INSTALL_SUFFIX=-2` and make sure that `BUILD_EXAMPLES` is not `ON`
for both versions. The installed files will then include bin/taglib-2-config,
include/taglib-2, cmake/taglib-2, pkgconfig/taglib-2.pc,
pkgconfig/taglib_c-2.pc and the libraries have a suffix "-2".
如果您想要同时安装 TagLib 2 和 TagLib 1，则可以使用
`-DTAGLIB_INSTALL_SUFFIX=-2`，并确保两个版本的 `BUILD_EXAMPLES` 都不是 `ON`。
安装的文件将包括 bin/taglib-2-config、include/taglib-2、cmake/taglib-2、pkgconfig/taglib-2.pc、
pkgconfig/taglib_c-2.pc，并且库具有后缀“-2”。


## Dependencies
依赖项

A required dependency is [utf8cpp](https://github.com/nemtrif/utfcpp). You can
install the corresponding package (libutfcpp-dev on Ubuntu, utf8cpp in Homebrew,
utfcpp in vcpkg) or fetch the Git submodule with `git submodule update --init`.
必需的依赖项是 [utf8cpp (https://github.com/nemtrif/utfcpp)。您可以安装相应的软件包（Ubuntu
上的 libutfcpp-dev、Homebrew 中的 utf8cpp、vcpkg 中的 utfcpp）或使用 `git submodule update --init`
获取 Git 子模块。

Optional dependencies are
可选依赖项包括
- [zlib](https://www.zlib.net/): You can disable it with `-DWITH_ZLIB=OFF`,
  build and install it from the sources or use a package (zlib1g-dev on Ubuntu,
  zlib in vcpkg). It is needed for compressed ID3v2 frames.
  [zlib](https://www.zlib.net/): 你可以使用 `-DWITH_ZLIB=OFF` 禁用它，从源代码构建并安装它或使用软件包（Ubuntu 上的 zlib1g-dev，vcpkg 中的 zlib）。它是压缩 ID3v2 帧所必需的。

- [CppUnit](https://wiki.documentfoundation.org/Cppunit): Is required for unit
  tests, which are disabled by default. If you enable them with
  `-DBUILD_TESTING=ON`, you can build and install it from the sources or use a
  package (libcppunit-dev on Ubuntu, cppunit in Homebrew, cppunit in vcpkg).
  If the unit tests are enabled, you can run them with your build tool
  (`make check`, `ninja check`) or via CMake
  `cmake --build /path/to/build-dir --target check`.
  [CppUnit](https://wiki.documentfoundation.org/Cppunit)：单元测试需要它，默认情况下禁用。如果您使用 `-DBUILD_TESTING=ON` 启用它，则可以从源代码构建和安装它，也可以使用软件包（Ubuntu 上的 libcppunit-dev、
  Homebrew 中的 cppunit、vcpkg 中的 cppunit）。如果启用了单元测试，您可以使用构建工具（`make check`、
  `ninja check`）或通过 CMake `cmake --build /path/to/build-dir --target check` 运行它们。

## UNIX (Including Linux, BSD and macOS)

#### Building TagLib

On Linux, you can install the dependencies using the package manager of your
distribution. On macOS with Homebrew, you can use `brew install cppunit utf8cpp`
to install the dependencies.
在 Linux 上，您可以使用发行版的包管理器安装依赖项。在装有 Homebrew 的 macOS 上，您可以使用“brew
install cppunit utf8cpp”来安装依赖项。

```
# Adapt these environment variables to your directories
使这些环境变量适应您的目录
TAGLIB_SRC_DIR=$HOME/projects/taglib/src/taglib
TAGLIB_DST_DIR=$HOME/projects/taglib/src/build
cd $TAGLIB_SRC_DIR
cmake -B $TAGLIB_DST_DIR -DBUILD_SHARED_LIBS=ON -DVISIBILITY_HIDDEN=ON \
  -DBUILD_TESTING=ON -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build $TAGLIB_DST_DIR --config Release
cmake --build $TAGLIB_DST_DIR --config Release --target check

# Install to ~/pkg folder
安装到 ~/pkg 文件夹
cmake --install $TAGLIB_DST_DIR --config Release --prefix $HOME/pkg --strip

# Run example from installed package
从已安装的包运行示例
LD_LIBRARY_PATH=$HOME/pkg/lib $HOME/pkg/bin/tagreader /path/to/audio-file
```

#### Building a Project Using TagLib

As an example for an external application using TagLib, we create a folder
taglib-client and copy the file tagreader.cpp from the examples folder
of the TagLib sources into it. We then add this simple CMakeLists.txt.

```
cmake_minimum_required(VERSION 3.5.0)
project(taglib-client)
set(CMAKE_CXX_STANDARD 17)
find_package(ZLIB)
find_package(TagLib 2.0.0 REQUIRED)
add_executable(tagreader tagreader.cpp)
target_link_libraries(tagreader TagLib::tag)
```

To build into a folder `build` inside this directory, just run

```
# Set this to the path where TagLib is installed
TAGLIB_PREFIX=$HOME/pkg
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$TAGLIB_PREFIX
cmake --build build --config Release

build/tagreader /path/to/audio-file
```

If TagLib is installed in a standard location (e.g. /usr, /usr/local), its CMake
configuration is found without setting CMAKE_INSTALL_PREFIX (or alternatives
like CMAKE_PREFIX_PATH, CMAKE_SYSTEM_PREFIX_PATH).

To use the C-bindings with tagreader_c.c, you can change the last two lines in
CMakeLists.txt to

```
add_executable(tagreader_c tagreader_c.c)
target_link_libraries(tagreader_c TagLib::tag_c)
```

#### Building a Project Using pkg-config

Before version 2.0, TagLib did not export CMake configuration, therefore
`pkg-config` could be used. This is still possible.

Note, however, that `pkg-config` makes it more difficult to use packages which
are not installed in a standard location. You can point `pkg-config` to search
in non-standard locations for .pc-files, but they still contain the install
locations defined when building TagLib. Since we did not give a
`CMAKE_INSTALL_PREFIX` in the example above, the default `/usr/local/` is used.

```
PKG_CONFIG_PATH=$HOME/pkg/lib/pkgconfig pkg-config --libs --cflags taglib
-I/usr/local/include -I/usr/local/include/taglib -L/usr/local/lib -ltag -lz
```

The following examples use the same build example with additional CMake
parameters affecting the installation location.

- Using the default prefix `-DCMAKE_INSTALL_PREFIX=/usr/local`:
  ```
  -I/usr/local/include -I/usr/local/include/taglib -L/usr/local/lib -ltag -lz
  ```
- Using an absolute prefix `-DCMAKE_INSTALL_PREFIX=/usr`:
  ```
  -I/usr/include/taglib -ltag -lz
  ```
- Using absolute lib and include directories
  `-DCMAKE_INSTALL_LIBDIR=/abs-lib -DCMAKE_INSTALL_INCLUDEDIR=/abs-include -DCMAKE_INSTALL_PREFIX=/usr`:
  ```
  -I/abs-include -I/abs-include/taglib -L/abs-lib -ltag -lz
  ```
- Using relative lib and include directories
  `-DCMAKE_INSTALL_LIBDIR=rel-lib -DCMAKE_INSTALL_INCLUDEDIR=rel-include -DCMAKE_INSTALL_PREFIX=/usr`:
  ```
  -I/usr/rel-include -I/usr/rel-include/taglib -L/usr/rel-lib -ltag -lz
  ```
  This is the output of
  ```
  PKG_CONFIG_PATH=$HOME/pkg/rel-lib/pkgconfig pkg-config --libs --cflags taglib
  ```
  You could add `--define-prefix` to the `pkg-config` arguments to have the
  effective location `$HOME/pkg/` instead of `/usr/` in the output.

Therefore, the correct paths for our example package would be given when using
the `--define-prefix` feature.

```
PKG_CONFIG_PATH=$HOME/pkg/lib/pkgconfig pkg-config --define-prefix --libs --cflags taglib
```

You can use pkg-config from CMake, however, relocation with `--define-prefix`
is not supported.

```
cmake_minimum_required(VERSION 3.6.0)
project(taglib-client)
find_package(PkgConfig)
pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)
add_executable(tagreader tagreader.cpp)
target_link_libraries(tagreader PkgConfig::TAGLIB)
```

#### Framework on macOS

On macOS, you might want to build a framework that can be easily integrated
into your application. If you set the BUILD_FRAMEWORK option on, it will compile
TagLib as a framework. For example, the following command can be used to build
a framework with macOS 10.10 as the deployment target:

    mkdir build; cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=OFF \
      -DBUILD_FRAMEWORK=ON \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 \
      -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
    make

For a 10.10 static library, use:

    mkdir build; cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 \
      -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
    make

After `make`, and `make install`, add `libtag.` to your XCode project, and add
the include folder to the project's User Header Search Paths.

## Windows

### Using Visual Studio Build Tools

For this example, we assume that you have the Visual Studio Build Tools 2022
installed. Additionally, you need [cmake](https://cmake.org/), which can be
installed for example using [scoop](https://scoop.sh/) with
`scoop install cmake`.

#### Building TagLib (MSVC)

You can build and install the dependencies
[utf8cpp](https://github.com/nemtrif/utfcpp) and optionally
[zlib](https://www.zlib.net/) and
[CppUnit](https://wiki.documentfoundation.org/Cppunit) yourself, but it is
probably easier using a package manager such as [vcpkg](https://vcpkg.io/),
which can be installed using `scoop install vcpkg` and then install the
dependencies using `vcpkg install utfcpp zlib cppunit`.

Now you can build TagLib from PowerShell.

```
# Adapt these environment variables to your directories
$env:TAGLIB_SRC_DIR = "${env:HOMEDRIVE}${env:HOMEPATH}/projects/taglib/src/taglib"
$env:TAGLIB_DST_DIR = "${env:HOMEDRIVE}${env:HOMEPATH}/projects/taglib/src/msvs_vcpkg_build"
cd $env:TAGLIB_SRC_DIR
cmake -B $env:TAGLIB_DST_DIR -DBUILD_SHARED_LIBS=ON -DVISIBILITY_HIDDEN=ON `
  -DBUILD_TESTING=ON -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -G "Visual Studio 17 2022"
cmake --build $env:TAGLIB_DST_DIR --config Release

# Add directories containing DLL dependencies to path
$env:Path += -join(";$env:TAGLIB_DST_DIR\taglib\Release;",
"$env:TAGLIB_DST_DIR\bindings\c\Release;",
"$env:VCPKG_ROOT\packages\cppunit_x64-windows\bin;",
"$env:VCPKG_ROOT\packages\utfcpp_x64-windows\bin;",
"$env:VCPKG_ROOT\packages\zlib_x64-windows\bin")
cmake --build $env:TAGLIB_DST_DIR --config Release --target check

# Install to \pkg folder on current drive
cmake --install $env:TAGLIB_DST_DIR --config Release --prefix /pkg --strip

# Static library
$env:TAGLIB_DST_DIR = "C:/Users/fle/projects/taglib/src/msvs_vcpkg_static_build"
cmake -B $env:TAGLIB_DST_DIR -DBUILD_SHARED_LIBS=OFF -DVISIBILITY_HIDDEN=ON `
  -DBUILD_TESTING=ON -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -G "Visual Studio 17 2022"
cmake --build $env:TAGLIB_DST_DIR --config Release

cmake --build $env:TAGLIB_DST_DIR --config Release --target check

# Install to \pkg_static folder on current drive
cmake --install $env:TAGLIB_DST_DIR --config Release --prefix /pkg_static --strip
```

Including `ENABLE_STATIC_RUNTIME=ON` indicates you want TagLib built using the
static runtime library, rather than the DLL form of the runtime.

#### Building a Project Using TagLib (MSVC)

As an example for an external application using TagLib, we create a folder
taglib-client and copy the file tagreader.cpp from the examples folder
of the TagLib sources into it. We then add this simple CMakeLists.txt.

```
cmake_minimum_required(VERSION 3.5.0)
project(taglib-client)
set(CMAKE_CXX_STANDARD 17)
find_package(ZLIB)
find_package(TagLib 2.0.0 REQUIRED)
add_executable(tagreader tagreader.cpp)
target_link_libraries(tagreader TagLib::tag)
```

To build into a folder build inside this directory, just run

```
# Set this to the path where TagLib is installed
$env:TAGLIB_PREFIX = "/pkg"
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$env:TAGLIB_PREFIX" `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -G "Visual Studio 17 2022"
cmake --build build --config Release

# Add directories containing DLL dependencies to path
$env:Path += ";$env:TAGLIB_PREFIX\bin;$env:VCPKG_ROOT\packages\zlib_x64-windows\bin"
build\Release\tagreader /path/to/audio-file
```

To use the C-bindings with tagreader_c.c, you can change the last two lines in
CMakeLists.txt to

```
add_executable(tagreader_c tagreader_c.c)
target_link_libraries(tagreader_c TagLib::tag_c)
```

If you link against a static TagLib, you have to adapt the installation path
(e.g. "/pkg_static") and add the following line to CMakeLists.txt

```
target_compile_definitions(tagreader_c PRIVATE TAGLIB_STATIC)
```

### Using MSYS2

#### Building TagLib (MSYS2)

To build TagLib using Clang from MSYS, install [msys2](https://www.msys2.org/),
then start the MSYS CLANG64 shell and install the dependencies as they are
specified in the [MSYS recipe for TagLib](
https://packages.msys2.org/package/mingw-w64-clang-x86_64-taglib?repo=clang64).

```
pacman -Suy
pacman -S mingw-w64-clang-x86_64-gcc-libs mingw-w64-clang-x86_64-zlib \
  mingw-w64-clang-x86_64-cc mingw-w64-clang-x86_64-cmake \
  mingw-w64-clang-x86_64-cppunit mingw-w64-clang-x86_64-ninja
```

Then you can build TagLib from the MSYS CLANG64 Bash.

```
# Adapt these environment variables to your directories
TAGLIB_SRC_DIR=$HOME/projects/taglib/src/taglib
TAGLIB_DST_DIR=$HOME/projects/taglib/src/msys_build
cd $TAGLIB_SRC_DIR
cmake -B $TAGLIB_DST_DIR -DBUILD_SHARED_LIBS=ON -DVISIBILITY_HIDDEN=ON \
  -DBUILD_TESTING=ON -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON \
  -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build $TAGLIB_DST_DIR --config Release

PATH=$PATH:/clang64/bin:$TAGLIB_DST_DIR/taglib:$TAGLIB_DST_DIR/bindings/c \
  cmake --build $TAGLIB_DST_DIR --config Release --target check

# Install to /pkg_msys folder inside MSYS root (e.g. C:/msys64/pkg_msys/)
cmake --install $TAGLIB_DST_DIR --config Release --prefix /pkg_msys --strip
```

#### Building a Project Using TagLib (MSYS2)

As an example for an external application using TagLib, we create a folder
taglib-client and copy the file tagreader.cpp from the examples folder
of the TagLib sources into it. We then add this simple CMakeLists.txt.

```
cmake_minimum_required(VERSION 3.5.0)
project(taglib-client)
set(CMAKE_CXX_STANDARD 17)
find_package(ZLIB)
find_package(TagLib 2.0.0 REQUIRED)
add_executable(tagreader tagreader.cpp)
target_link_libraries(tagreader TagLib::tag)
```

To build into a folder build_msys inside this directory, just run

```
# Set this to the path where TagLib is installed
TAGLIB_PREFIX=/pkg_msys
cmake -B build_msys -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$TAGLIB_PREFIX -G Ninja
cmake --build build_msys --config Release

PATH=$PATH:$TAGLIB_PREFIX/bin
  build_msys/tagreader /path/to/audio-file
```

To use the C-bindings with tagreader_c.c, you can change the last two lines in
CMakeLists.txt to

```
add_executable(tagreader_c tagreader_c.c)
target_link_libraries(tagreader_c TagLib::tag_c)
```

### Using MinGW

The instructions for MSYS2 can also be used to build with MinGW. You could use
Git Bash together with the MinGW provided by Qt.

```
# Adapt these environment variables to your directories
PATH=$PATH:/c/Qt/Tools/mingw1120_64/bin
TAGLIB_SRC_DIR=$HOME/projects/taglib/src/taglib
TAGLIB_DST_DIR=$HOME/projects/taglib/src/mingw_build
cd $TAGLIB_SRC_DIR
cmake -B $TAGLIB_DST_DIR -DBUILD_SHARED_LIBS=ON -DVISIBILITY_HIDDEN=ON \
  -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON -DWITH_ZLIB=OFF \
  -DCMAKE_BUILD_TYPE=Release -G 'MinGW Makefiles'
cmake --build $TAGLIB_DST_DIR --config Release

PATH=$PATH:$TAGLIB_DST_DIR/taglib \
  $TAGLIB_DST_DIR/examples/tagreader /path/to/audio-file

# Install to C:\pkg_mingw
cmake --install $TAGLIB_DST_DIR --config Release --prefix /c/pkg_mingw --strip
```

The installed package can then be used by other projects.

```
TAGLIB_PREFIX=/c/pkg_mingw
cmake -B build_mingw -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$TAGLIB_PREFIX -G 'MinGW Makefiles'
cmake --build build_mingw --config Release

PATH=$PATH:$TAGLIB_PREFIX/bin
  build_mingw/tagreader /path/to/audio-file
```
