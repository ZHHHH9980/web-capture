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

  if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
    logging("ERROR could not find stream information");
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
