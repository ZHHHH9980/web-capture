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
    [ ! -d "x264" ] && git clone --depth 1 https://code.videolan.org/videolan/x264.git

    # 编译 libx264
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
mkdir $WEB_CAPTURE_PATH/lib/ffmpeg-emcc


# 配置 FFmpeg
emconfigure ./configure \
    --prefix=$WEB_CAPTURE_PATH/lib/ffmpeg-emcc \
    --cc="emcc" \
    --cxx="em++" \
    --ar="emar" \
    --list-decoders\
     --list-encoders\
     --list-hwaccels \
     --list-demuxers \
     --list-muxers\
    --cpu=generic \
    --target-os=none \
    --arch=x86_32 \
    --disable-avformat \
    --enable-gpl \
    --enable-version3 \
    --enable-muxer=mp4 \
    --enable-libx264 \
    --enable-cross-compile \
    --disable-logging \
    --disable-program \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --disable-doc \
    --disable-swresample \
    --disable-postproc \
    --disable-avfilter \
    --disable-pthreads \
    --disable-w32threads \
    --disable-os2threads \
    --disable-network \
    --disable-asm \
    --disable-debug \
    --enable-protocol=file \
    --enable-demuxer=mov \
    --enable-demuxer=matroska \
    --enable-demuxer=flv \
    --enable-demuxer=avi \
    --enable-decoder=h264,aac,pcm_s16le \
    --enable-decoder=hevc \
    --enable-decoder=mpeg4 \
    --enable-decoder=vp8 \
    --enable-decoder=vp9 \
    --enable-decoder=wmv3 \
    --extra-cflags="-I$X264_INSTALL_PATH/include" \
    --extra-ldflags="-L$X264_INSTALL_PATH/lib" || exit 1

# 编译并安装 FFmpeg
make || exit 1
make install || exit 1

echo "===== finish build ffmpeg-emcc ====="
