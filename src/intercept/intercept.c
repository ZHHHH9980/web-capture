#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/log.h"
#include "libavutil/timestamp.h"

typedef struct {
    unsigned char* data;
    int size;
} VideoData;

VideoData* cut_video(double start_seconds, double end_seconds, const char *inputFileName) {
    av_register_all();
    AVFormatContext *inputfile = NULL;
    AVFormatContext *outputfile = NULL;
    AVPacket pkt;
    int ret;
    av_log_set_level(AV_LOG_INFO);

    av_log(NULL, AV_LOG_INFO, "Cutting video from %.2f to %.2f seconds\n", start_seconds, end_seconds);
    av_log(NULL, AV_LOG_INFO, "Input file: %s\n", inputFileName);

    if ((ret = avformat_open_input(&inputfile, inputFileName, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not open source file %s, error code: %d\n", inputFileName, ret);
        return NULL;
    }

    av_log(NULL, AV_LOG_INFO, "Opened input file successfully\n");

    if ((ret = avformat_find_stream_info(inputfile, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not find stream info, error code: %d\n", ret);
        avformat_close_input(&inputfile);
        return NULL;
    }

    av_log(NULL, AV_LOG_INFO, "Found stream info successfully\n");
    av_log(NULL, AV_LOG_INFO, "Duration: %.2f seconds\n", inputfile->duration / (double)AV_TIME_BASE);

    if (start_seconds < 0 || start_seconds >= inputfile->duration / (double)AV_TIME_BASE) {
        av_log(NULL, AV_LOG_ERROR, "Invalid start time %.2f\n", start_seconds);
        avformat_close_input(&inputfile);
        return NULL;
    }

    if (end_seconds <= start_seconds || end_seconds > inputfile->duration / (double)AV_TIME_BASE) {
        av_log(NULL, AV_LOG_ERROR, "Invalid end time %.2f\n", end_seconds);
        avformat_close_input(&inputfile);
        return NULL;
    }

    // 分配输出文件上下文，并指定输出格式为MP4
	if (avformat_alloc_output_context2(&outputfile, NULL, "mp4", NULL) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Could not create output format context\n");
		avformat_close_input(&inputfile);
		return NULL;
	}

	// 循环复制输入流参数到输出流
	for (int i = 0; i < inputfile->nb_streams; i++) {
		AVStream *in_stream = inputfile->streams[i];
		AVStream *out_stream = avformat_new_stream(outputfile, NULL);
		if (!out_stream) {
			av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
			avformat_free_context(outputfile);
			avformat_close_input(&inputfile);
			return NULL;
		}
		if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed to copy codec parameters\n");
			avformat_free_context(outputfile);
			avformat_close_input(&inputfile);
			return NULL;
		}
		out_stream->codecpar->codec_tag = 0;
	}

    if (!(outputfile->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&outputfile->pb, "output.mp4", AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file, error code: %d\n", ret);
            avformat_free_context(outputfile);
            avformat_close_input(&inputfile);
            return NULL;
        }
    }

    ret = avformat_write_header(outputfile, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file, error code: %d\n", ret);
        avformat_free_context(outputfile);
        avformat_close_input(&inputfile);
        return NULL;
    }

    av_log(NULL, AV_LOG_INFO, "Opened output file for writing\n");

    ret = av_seek_frame(inputfile, -1, start_seconds * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error seeking to start time, error code: %d\n", ret);
        avformat_free_context(outputfile);
        avformat_close_input(&inputfile);
        return NULL;
    }

    av_log(NULL, AV_LOG_INFO, "Seeked to start time successfully\n");

    int64_t *dts_start_from = (int64_t*)malloc(sizeof(int64_t) * inputfile->nb_streams);
    memset(dts_start_from, 0, sizeof(int64_t) * inputfile->nb_streams);
    int64_t *pts_start_from = (int64_t*)malloc(sizeof(int64_t) * inputfile->nb_streams);
    memset(pts_start_from, 0, sizeof(int64_t) * inputfile->nb_streams);

    while (1) {
        AVStream *in_stream, *out_stream;
        ret = av_read_frame(inputfile, &pkt);
        if (ret < 0) break;
        in_stream  = inputfile->streams[pkt.stream_index];
        out_stream = outputfile->streams[pkt.stream_index];

        if (av_q2d(in_stream->time_base) * pkt.pts > end_seconds) {
            av_packet_unref(&pkt);
            break;
        }

        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
        }
        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
        }

        pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        if (pkt.pts < 0) pkt.pts = 0;
        if (pkt.dts < 0) pkt.dts = 0;
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = pkt.stream_index;

        ret = av_interleaved_write_frame(outputfile, &pkt);
		if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error muxing packet, error code: %d\n", ret);
            break;
        }
        av_packet_unref(&pkt);
    }

    free(dts_start_from);
    free(pts_start_from);

    av_write_trailer(outputfile);

    if (!(outputfile->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&outputfile->pb);
    }

    // 重新打开输出文件为内存缓冲区
    AVIOContext* output_buf = NULL;
    ret = avio_open_dyn_buf(&output_buf);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not open output buffer, error code: %d\n", ret);
        avformat_free_context(outputfile);
        avformat_close_input(&inputfile);
        return NULL;
    }

    // 将输出文件的数据写入内存缓冲区
    avio_write(output_buf, outputfile->pb->buffer, outputfile->pb->buf_ptr - outputfile->pb->buffer);
    if (output_buf->error < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not write to output buffer, error code: %d\n", output_buf->error);
        avio_close_dyn_buf(output_buf, NULL);
        avformat_free_context(outputfile);
        avformat_close_input(&inputfile);
        return NULL;
    }

    uint8_t* video_data = NULL;
    int video_size = avio_close_dyn_buf(output_buf, &video_data);

    // 创建返回结构体
    VideoData* result = (VideoData*)malloc(sizeof(VideoData));
    result->data = video_data;
    result->size = video_size;

    // 清理资源
    avformat_free_context(outputfile);
    avformat_close_input(&inputfile);

    return result;
}

int main(int argc, char *argv[]) {

    return 0;
}