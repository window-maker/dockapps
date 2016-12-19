CC	= gcc
CFLAGS  = -O2 -Wall -I/usr/X11R6/include -DVIDEO_DEV=\"/dev/video\"
LDFLAGS = 
LIBS	= -L/usr/X11R6/lib -lX11 -lXext -lm -ljpeg

OBJS    = minirgb.o wmwebcam.o

wmwebcam: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) *~ wmwebcam

install:
	cp wmwebcam /usr/bin
	cp wmwebcam.pl /usr/bin
