#include <avformat.h>
#include <stdio.h>

// print out the steps and errors
static void logging(const char *fmt, ...);

int main() {
  logging("initializing all the containers, codecs and protocols.");

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
