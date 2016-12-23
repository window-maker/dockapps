# Copyright (c) 2001-2003 Aaron Trickey <aaron@amtrickey.net>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Makefile for the dockapp library packaged in the ``wmget'' application.


DAOBJS=		da_run.o da_x.o da_mouse.o

CFLAGS=		-ansi -Wall -W -Werror -g
LDFLAGS=	-L/usr/X11R6/lib -lXpm -lXext -lX11

dockapp.a:	$(DAOBJS)
		ar rcsv dockapp.a $(DAOBJS)

unittest:	unittest.o $(DAOBJS)

clean:
		rm -f unittest.o unittest dockapp.a $(DAOBJS)

