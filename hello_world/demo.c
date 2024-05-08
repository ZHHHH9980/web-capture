#include <avformat.h>
#include <stdio.h>

// print out the steps and errors
static void logging(const char *fmt, ...);

int main(int argc, const char *argv[]) {

  logging("initializing all the containers, codecs and protocols.");

  AVFormatContext *pFormatContext = avformat_alloc_context();

  if (!pFormatContext) {
    logging("ERROR could not allocate memory for Format Context");
    return -1;
  }

  logging("opening the input file (%s) and loading format (container) header", argv[1]);

  // Open the file and read its header. The codecs are not opened.
  if (avformat_open_input(&pFormatContext, argv[1], NULL, NULL) != 0) {
    logging("ERROR could not open the file");
    return -1;
  }
  
  // Now we have access to some information about the file
  // Since we read its header we can say what format (container) it's 
  // and some other information related to the format (duration, bit rate, etc.)
  logging("format %s, duration %lld us, bit_rate %lld",
      pFormatContext->iformat->name,
      pFormatContext->duration,
      pFormatContext->bit_rate
  );

  if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
    logging("ERROR could not find stream information");
    return -1;
  }

  // the component that knows how to enCOde and DECode the stream
  // it's the codec
  AVCodec *pCodec = NULL;

  // descibes the properties of a codec used by the stream i
  AVCodecParameters *pCodecParameters =  NULL;

  int video_stream_index = -1;

  // loop_through all the streams and print its main information
  for (int i = 0; i < pFormatContext->nb_streams; i++) {
    // 存储找到的流的编解码参数
    AVCodecParameters *pLocalCodecParameters =  NULL;
    pLocalCodecParameters = pFormatContext->streams[i]->codecpar;

    logging("AVStream->time_base before open coded %d/%d",
        pFormatContext->streams[i]->time_base.num,
        pFormatContext->streams[i]->time_base.den
    );
    logging("AVStream->r_frame_rate before open coded %d/%d",
        pFormatContext->streams[i]->r_frame_rate.num,
        pFormatContext->streams[i]->r_frame_rate.den
    );
    logging("AVStream->start_time %" PRId64, pFormatContext->streams[i]->start_time);
    logging("AVStream->duration %" PRId64, pFormatContext->streams[i]->duration);


    logging("finding the proper decoder (CODEC)");

    AVCodec *pLocalCodec = NULL;

    pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

    if (pLocalCodec == NULL) {
      logging("ERROR unsupported codec!");
      continue;
    }

    // 如果这个流是视频流
    if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {

      // 存储视频流的索引
      if (video_stream_index == -1) {
        video_stream_index = i;
        pCodec = pLocalCodec;
        pCodecParameters = pLocalCodecParameters;

        logging("Video Codec: resolution %d x %d",
            pLocalCodecParameters->width,
            pLocalCodecParameters->height
        );
      }
      // 如果这个流是音频流
    } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
      logging("Audio Codec: %d channels, sample rate %d",
          pLocalCodecParameters->channels,
          pLocalCodecParameters->sample_rate
      );
    }

    // print its name, id and bitrate
    logging("Codec %s ID %d bit_rate %lld", pLocalCodec->long_name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
  }


  return 0;
}

static void logging(const char *fmt, ...)
{
    va_list args;
    fprintf( stderr, "LOG: " );
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
}
