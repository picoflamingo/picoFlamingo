
CFLAGS1 = -O3
LFLAGS1 = -ljpeg

OFILES = v4l2.o jpeg.o

all:		v4l2

v4l2:		$(OFILES)
		$(CC) -o v4l2 $(OFILES) $(LFLAGS1)

clean:
		rm -f $(TARGETS) *.o *~
