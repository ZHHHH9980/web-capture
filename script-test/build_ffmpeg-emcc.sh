#!/bin/bash

# 获取当前脚本所在路径
NOW_PATH=$(cd $(dirname $0) && pwd)

# 载入 Emscripten 环境变量
source $NOW_PATH/../../emsdk/emsdk_env.sh

# 定义 Web Capture 路径
WEB_CAPTURE_PATH=$(cd $NOW_PATH/../ && pwd)

# 定义 FFmpeg 路径
FFMPEG_PATH=$(cd $WEB_CAPTURE_PATH/../ffmpeg-3.4.8 && pwd)

# 定义 libx264 源码位置和安装位置
X264_PATH=$WEB_CAPTURE_PATH/../x264
X264_INSTALL_PATH=$WEB_CAPTURE_PATH/lib/x264-emcc

# 确保安装目录存在
mkdir -p $X264_INSTALL_PATH

# 检查 libx264 是否已经安装
if [ ! -f "$X264_INSTALL_PATH/lib/libx264.a" ]; then
    echo "libx264 not found, cloning and building..."
    cd $WEB_CAPTURE_PATH/../
    [ ! -d "x264" ] && git clone --depth 1 https://code.videolan.org/videolan/x264.git $X264_PATH

    cd $X264_PATH
    make distclean
    emconfigure ./configure --prefix=$X264_INSTALL_PATH --enable-static --disable-cli --host=i686-gnu --disable-asm || exit 1
    emmake make || exit 1
    emmake make install || exit 1
else
    echo "libx264 already installed."
fi

echo "===== start build ffmpeg-emcc ====="

# 清理并创建 FFmpeg 安装目录
rm -rf $WEB_CAPTURE_PATH/lib/ffmpeg-emcc
mkdir -p $WEB_CAPTURE_PATH/lib/ffmpeg-emcc

cd $FFMPEG_PATH
make distclean

# 配置 FFmpeg
emconfigure ./configure \
    --prefix=$WEB_CAPTURE_PATH/lib/ffmpeg-emcc \
    --cc="emcc" \
    --cxx="em++" \
    --ar="emar" \
    --enable-gpl \
    --enable-libx264 \
    --enable-muxer=mp4 \
    --enable-protocol=file \
    --enable-demuxer=mov \
    --enable-decoder=h264 \
    --extra-cflags="-I$X264_INSTALL_PATH/include" \
    --extra-ldflags="-L$X264_INSTALL_PATH/lib" || exit 1

# 编译并安装 FFmpeg
make || exit 1
make install || exit 1

echo "===== finish build ffmpeg-emcc ====="
