#!/bin/bashL

# 定义 libx264 源码位置和安装位置
X264_PATH=$WEB_CAPTURE_PATH/../x264
X264_INSTALL_PATH=$WEB_CAPTURE_PATH/lib/x264-emcc

# 下载并解压 libx264 源码（如果还没有的话）
cd $WEB_CAPTURE_PATH
[ ! -d "x264" ] && git clone --depth 1 https://code.videolan.org/videolan/x264.git

# 编译 libx264
cd $X264_PATH
make distclean
emconfigure ./configure --prefix=$X264_INSTALL_PATH --enable-static --disable-cli --host=i686-gnu --disable-asm
emmake make
emmake make install

echo "===== start build ffmpeg-emcc ====="

NOW_PATH=$(cd $(dirname $0); pwd)

WEB_CAPTURE_PATH=$(cd $NOW_PATH/../; pwd)

FFMPEG_PATH=$(cd $WEB_CAPTURE_PATH/../ffmpeg-3.4.8; pwd)

source $WEB_CAPTURE_PATH/../emsdk/emsdk_env.sh

rm -rf  $WEB_CAPTURE_PATH/lib/ffmpeg-emcc

mkdir $WEB_CAPTURE_PATH/lib/ffmpeg-emcc

cd $FFMPEG_PATH

make clean

emconfigure ./configure \
    --prefix=$WEB_CAPTURE_PATH/lib/ffmpeg-emcc \
    --cc="emcc" \
    --cxx="em++" \
    --ar="emar" \
    --cpu=generic \
    --target-os=none \
    --arch=x86_32 \
    --enable-gpl \
    --enable-version3 \
    --enable-libx264 \
    --enable-cross-compile \
    --disable-logging \
    --disable-programs \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --disable-doc \
    --disable-swresample \
    --disable-postproc  \
    --disable-avfilter \
    --disable-pthreads \
    --disable-w32threads \
    --disable-os2threads \
    --disable-network \
    --disable-everything \
    --disable-asm \
    --disable-debug \
    --enable-protocol=file \
    --enable-demuxer=mov \
    --enable-demuxer=matroska \
    --enable-demuxer=flv \
    --enable-demuxer=avi \
    --enable-decoder=h264 \
    --enable-decoder=hevc \
    --enable-decoder=mpeg4 \
    --enable-decoder=vp8 \
    --enable-decoder=vp9 \
    --enable-decoder=wmv3 \
    --extra-cflags="-I$X264_INSTALL_PATH/include" \
    --extra-ldflags="-L$X264_INSTALL_PATH/lib"

make

make install

echo "===== finish build ffmpeg-emcc ====="
