#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <stdio.h>

int main() {
    av_register_all();
    
    return 0;
}

AVFormatContext *cut_video(double duration, const char *input_video, const char *output_video) {
    AVFormatContext *input_ctx = NULL;
    AVFormatContext *output_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int video_index = -1;
    int64_t start_time, end_time;

    // 打开输入视频文件
    ret = avformat_open_input(&input_ctx, input_video, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "无法打开输入视频文件: %s\n", av_err2str(ret));
        return NULL;
    }

    // 查找视频流的索引
    for (i = 0; i < input_ctx->nb_streams; i++) {
        if (input_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
            break;
        }
    }
    if (video_index == -1) {
        fprintf(stderr, "未找到视频流\n");
        goto end;
    }

    // 创建输出上下文
    avformat_alloc_output_context2(&output_ctx, NULL, "mp4", output_video);
    if (!output_ctx) {
        fprintf(stderr, "无法创建输出上下文\n");
        goto end;
    }

    // 复制视频流的参数
    AVStream *in_stream = input_ctx->streams[video_index];
    AVStream *out_stream = avformat_new_stream(output_ctx, NULL);
    ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
    if (ret < 0) {
        fprintf(stderr, "无法复制编解码器参数: %s\n", av_err2str(ret));
        goto end;
    }

    // 打开输出文件
    ret = avio_open(&output_ctx->pb, output_video, AVIO_FLAG_WRITE);
    if (ret < 0) {
        fprintf(stderr, "无法打开输出文件: %s\n", av_err2str(ret));
        goto end;
    }

    // 写入输出文件头
    ret = avformat_write_header(output_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "无法写入输出文件头: %s\n", av_err2str(ret));
        goto end;
    }

    // 计算截取的起始和结束时间
    start_time = 0;
    end_time = (int64_t)(duration * AV_TIME_BASE);

    // 读取和写入数据包
    while (av_read_frame(input_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_index) {
            // 检查数据包的时间戳是否在截取范围内
            if (pkt.pts >= start_time && pkt.pts <= end_time) {
                // 调整数据包的时间戳
                pkt.pts -= start_time;
                pkt.dts -= start_time;
                
                // 写入数据包到输出文件
                av_interleaved_write_frame(output_ctx, &pkt);
            }
        }
        av_packet_unref(&pkt);
    }

    // 写入输出文件尾
    av_write_trailer(output_ctx);

end:
    // 关闭输入上下文
    avformat_close_input(&input_ctx);
    
    // 如果输出上下文打开失败,关闭输出文件并释放上下文
    if (!output_ctx || (output_ctx->oformat->flags & AVFMT_NOFILE)) {
        if (output_ctx && !(output_ctx->oformat->flags & AVFMT_NOFILE))
            avio_closep(&output_ctx->pb);
        avformat_free_context(output_ctx);
        output_ctx = NULL;
    }
    
    return output_ctx;
}

