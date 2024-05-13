#include<stdio.h>
#include <stdlib.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include <libavutil/log.h>
#include <libavutil/timestamp.h>

#define ERROR_STR_SIZE 1024

void cut_video(double start_seconds, double end_seconds, const char *inputFileName, const char *outputFileName) {
    av_register_all();
    AVFormatContext * inputfile = NULL; //创建输入上下文
    AVFormatContext * outputfile = NULL;//创建输出上下文
    AVOutputFormat *ofmt = NULL; //创建格式
    AVPacket pkt;//创建包
    int error_code;//创建错误代码
    int ret;
    av_log_set_level(AV_LOG_INFO);

    av_log(NULL, AV_LOG_INFO, "Cutting video from %.2f to %.2f seconds\n", start_seconds, end_seconds);
    av_log(NULL, AV_LOG_INFO, "Input file: %s\n", inputFileName);
    av_log(NULL, AV_LOG_INFO, "Output file: %s\n", outputFileName);

    if (error_code = avformat_open_input(&inputfile, inputFileName, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not open source file %s, error code: %d\n", inputFileName, error_code);
        avformat_close_input(&inputfile);
        return;
    }

    av_log(NULL, AV_LOG_INFO, "Opened input file successfully\n");

    // if (error_code = avformat_find_stream_info(inputfile, NULL) < 0) {
    //     av_log(NULL, AV_LOG_ERROR, "Could not find stream info, error code: %d\n", error_code);
    //     avformat_close_input(&inputfile);
    //     return;
    // }
	 /* allocate the output media context */
    avformat_alloc_output_context2(&outputfile, NULL, NULL, outputFileName);
    if (!outputfile) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&outputfile, NULL, "mpeg", outputFileName);
    }

    if (!outputfile) {

        return ;
	}


    av_log(NULL, AV_LOG_INFO, "Found stream info successfully\n");
    av_log(NULL, AV_LOG_INFO, "Duration: %.2f seconds\n", inputfile->duration / (double)AV_TIME_BASE);

    if (start_seconds < 0 || start_seconds >= inputfile->duration / (double)AV_TIME_BASE) {
        av_log(NULL, AV_LOG_ERROR, "Invalid start time %.2f\n", start_seconds);
        avformat_close_input(&inputfile);
        return;
    }

    if (end_seconds <= start_seconds || end_seconds > inputfile->duration / (double)AV_TIME_BASE) {
        av_log(NULL, AV_LOG_ERROR, "Invalid end time %.2f\n", end_seconds);
        avformat_close_input(&inputfile);
        return;
    }

    if (error_code = avformat_alloc_output_context2(&outputfile, NULL, "mp4", outputFileName) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context, error code: %d\n", error_code);
        avformat_close_input(&inputfile);
        return;
    }
  
    av_log(NULL, AV_LOG_INFO, "Allocated output context successfully\n");
  
	for (int i = 0; i < inputfile->nb_streams; i++) {
		AVStream * inputStream = inputfile->streams[i];//创建输入流(根据序号是音频流还是视频流)。。这里通过循环的方式就可以不用av_find_best_stream（）方法
		AVStream * outputStream = avformat_new_stream(outputfile, NULL);//创建输出流
		if (!outputStream) {
			av_log(NULL, AV_LOG_ERROR, "Could not create The Stream\n");
			//goto end;
			avformat_close_input(&inputfile);
		}
		avcodec_parameters_copy(outputStream->codecpar, inputStream->codecpar);
		outputStream->codecpar->codec_tag = 0;
	}
	
	  avio_open(&outputfile->pb, outputFileName, AVIO_FLAG_WRITE);
    avformat_write_header(outputfile, NULL);

    av_log(NULL, AV_LOG_INFO, "Opened output file for writing\n");

    //跳转到指定帧
    ret = av_seek_frame(inputfile, -1, start_seconds * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error seeking to start time, error code: %d\n", ret);
        avformat_close_input(&inputfile);
        return;
    }

    av_log(NULL, AV_LOG_INFO, "Seeked to start time successfully\n");


	int64_t *dts_start_from = (int64_t*)malloc(sizeof(int64_t) * inputfile->nb_streams);//动态分配内存空间
	memset(dts_start_from, 0, sizeof(int64_t) * inputfile->nb_streams);//内存空间初始化
	int64_t *pts_start_from = (int64_t*)malloc(sizeof(int64_t) * inputfile->nb_streams);
	memset(pts_start_from, 0, sizeof(int64_t) * inputfile->nb_streams);
	while (1) {
		AVStream *in_stream, *out_stream;
		ret = av_read_frame(inputfile, &pkt);
		if (ret < 0)
			break;
		in_stream = inputfile->streams[pkt.stream_index];
		out_stream = outputfile->streams[pkt.stream_index];
		if (av_q2d(in_stream->time_base) * pkt.pts > end_seconds) {
			av_free_packet(&pkt);
			break;
		}
		if (dts_start_from[pkt.stream_index] == 0) {
			dts_start_from[pkt.stream_index] = pkt.dts;//记录裁剪初始位置的DTS  pkt.stream_index 为了区分音频流和视频流等信息
			printf("dts_start_from:\n");
		}
		if (pts_start_from[pkt.stream_index] == 0) {
			pts_start_from[pkt.stream_index] = pkt.pts;//记录裁剪初始位置的PTS
			printf("pts_start_from:\n");
		}

		/* copy packet */ // 时间基转换函数求出新的位置下的PTS\DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

		if (pkt.pts < 0) {
			pkt.pts = 0;
		}
		if (pkt.dts < 0) {
			pkt.dts = 0;
		}
		pkt.duration = (int)av_rescale_q((int64_t)pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		printf("\n");
		ret = av_interleaved_write_frame(outputfile, &pkt);
		if (ret < 0) {
			fprintf(stderr, "Error muxing packet\n");
			break;
		}
		av_free_packet(&pkt);
	}
	//释放资源
	free(dts_start_from);
	free(pts_start_from);

	//写文件尾信息
	av_write_trailer(outputfile);
}
int main(int argc, char const *argv[])
{

  return 0;
}

