#include <avformat.h>
#include <stdio.h>

// print out the steps and errors
static void logging(const char *fmt, ...);

int main() {

  logging("initializing all the containers, codecs and protocols.");

  AvFormatContext *pFormatContext = avformat_alloc_context();

  if (!pFormatContext) {
    logging("ERROR could not allocate memory for Format Context");
    return -1;
  }

  logging("opening the input file (%s) and loading format (container) header", argv[1]);

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
