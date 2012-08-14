CC     = gcc
CFLAGS = -O2 -Wall 
INCDIR = 
DESTDIR= /usr
LIBDIR = 
LIBS   = -lXpm -lX11 -lXext

OBJS   = wmMatrix.o matrix.o xutils.o yarandom.o

.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)

all:	wmMatrix

wmMatrix.o:	wmMatrix_master.xpm wmMatrix_mask.xbm

wmMatrix:	$(OBJS) 
	$(CC) $(LDFLAGS) -o wmMatrix $^ $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmMatrix

install:	wmMatrix
	install -m 0755 wmMatrix $(DESTDIR)/bin
	install -m 0644 wmMatrix.1 $(DESTDIR)/share/man/man1
