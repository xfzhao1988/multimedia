#!/bin/bash

# 1. 安装依赖子模块
# 必须安装的子模块：sudo apt-get install libutfcpp-dev

# 2. build和install
# 将该build.sh脚本copy到taglib-2.0.1目录下，加上可执行权限，然后./build.sh即可
# 在~/.bashrc中添加如下两行：
# export PATH=$PATH:/home/zxf/study/3_multimedia/audio/oss/mp3/taglib-2.0.1/install/bin
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/zxf/study/3_multimedia/audio/oss/mp3/taglib-2.0.1/install/lib
#

set -e

mkdir build
mkdir install

TAGLIB_SRC_DIR=/home/zxf/study/3_multimedia/audio/oss/mp3/taglib-2.0.1
TAGLIB_DST_DIR=/home/zxf/study/3_multimedia/audio/oss/mp3/taglib-2.0.1/build

cd $TAGLIB_SRC_DIR

cmake -B $TAGLIB_DST_DIR -DBUILD_SHARED_LIBS=ON \
  -DBUILD_TESTING=ON -DBUILD_EXAMPLES=ON -DBUILD_BINDINGS=ON \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build $TAGLIB_DST_DIR --config Debug

echo "build taglib-2.0.1 success"

TAGLIB_INSTALL_DIR=/home/zxf/study/3_multimedia/audio/oss/mp3/taglib-2.0.1/install
cmake --install $TAGLIB_DST_DIR --config Debug --prefix $TAGLIB_INSTALL_DIR

echo "install taglib-2.0.1 success"