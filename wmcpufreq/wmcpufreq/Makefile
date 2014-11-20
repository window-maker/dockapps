CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include
DESTDIR= /usr/local/
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lX11 -lXext -lcpufreq
OBJS   = wmcpufreq.o \
         ../wmgeneral/wmgeneral.o \
	 ../wmgeneral/misc.o \
	 ../wmgeneral/list.o


.c.o:
	$(CC) $(CFLAGS)  -c $< -o $*.o $(INCDIR)

all:	wmcpufreq.o wmcpufreq

wmcpufreq.o: wmcpufreq_master_1.xpm wmcpufreq_mask_1.xbm wmcpufreq_master_2.xpm wmcpufreq_mask_2.xbm wmcpufreq_master_3.xpm wmcpufreq_mask_3.xbm
wmcpufreq: $(OBJS)
	$(CC) 	$(SYSTEM) -o wmcpufreq $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmcpufreq

install:: wmcpufreq
	install -s -m 0755 wmcpufreq $(DESTDIR)/bin
	chmod +x $(DESTDIR)/bin/wmcpufreq

