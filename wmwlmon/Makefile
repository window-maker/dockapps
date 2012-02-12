# $Id: Makefile,v 1.2 2005/10/20 15:25:28 hacki Exp $

PROG=		wmwlmon
SRCS=		wmwlmon.c wl.c xutils.c
MAN=		wmwlmon.1

CFLAGS+=	-Wall -Werror -Wstrict-prototypes -ansi -I/usr/X11R6/include
LDADD+=		-L/usr/X11R6/lib -lXpm -lX11 -lXext -lm
PIPE=		-pipe

BINDIR=		/usr/local/bin

.include <bsd.prog.mk>
