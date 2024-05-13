# 使用 Ubuntu 23.04 LTS 作为基础镜像
FROM ubuntu:23.04

# 使用北京外国语大学的 Ubuntu 镜像源



# 安装 gcc 编译器、make 工具及 FFmpeg 开发库
RUN apt-get update && apt-get install -y \
    build-essential \
    libavformat-dev \
    libavcodec-dev \
    && rm -rf /var/lib/apt/lists/* 

# 设置工作目录
WORKDIR /app

# 将整个项目目录复制到容器的 /app 目录
COPY . /app/

# 切换到含有 Makefile 的子目录
WORKDIR /app/hello_world

# 使用 Makefile 编译 demo 程序
RUN make demo

# 运行时配置，当 Docker 容器启动时运行 demo 程序
CMD ["./demo"]
