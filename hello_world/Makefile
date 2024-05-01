CC=gcc
CFLAGS=-I/usr/include/x86_64-linux-gnu/libavformat -I/usr/include/x86_64-linux-gnu/libavcodec
LIBS=-lavformat -lavcodec -lavutil -lswscale

my_program: demo.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
