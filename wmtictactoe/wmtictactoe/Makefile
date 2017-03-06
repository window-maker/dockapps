LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -ldockapp
OBJS =  wmtictactoe.o

.c.o:
	cc -c -g -O2 -Wall $< -o $*.o

all: wmtictactoe

wmtictactoe: $(OBJS)
	cc -o wmtictactoe $^ $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -rf $$i;\
	done
	rm -rf wmtictactoe core *~

install:
	/usr/bin/install -c -o 0 -g 0 -s -m 755 wmtictactoe /usr/local/bin/wmtictactoe

