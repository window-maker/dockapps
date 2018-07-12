#########################################################
# This is included in a Makefile.
#
# e.g.:
# include general.mk
#########################################################

ifeq ($(MODULAR_X),1)
PREFIX      = $(shell pkg-config --variable=prefix xorg-server)
else
PREFIX      = /usr/X11R6
endif
BINDIR      = $(PREFIX)/bin
LIBDIR      = $(PREFIX)/lib
INCDIR      = $(PREFIX)/include
MANDIR      = $(PREFIX)/share/man
# Set some destination locations.

CC          = gcc
CXX         = g++
# Set the compilers.

#CFLAGS      = -O2
#CXXFLAGS     = $(CFLAGS)
# Safe for all systems.

ifeq ($(CFLAGS),)
	CFLAGS      = -march=pentium3 -O2 -fomit-frame-pointer -pipe
	CXXFLAGS    = $(CFLAGS)
endif
# This is what I use on my box (Intel Celeron (Coppermine) 800MHz).

ifeq ($(FASTER_MATH),1)
	CFLAGS     += -mfpmath=sse -ffast-math
endif

CFLAGS     += -std=c99
CFLAGS     += -Wall -W
CXXFLAGS   += -std=c99
CXXFLAGS   += -Wall -W
# Set the compiler flags.

AS          = as
ASFLAGS     =
# Set the assembler and flags.

LD          = ld
LDFLAGS     = -lm
ifeq ($(MODULAR_X),1)
LDFLAGS    += $(shell pkg-config --libs x11 xpm xext)
else
LDFLAGS    += -L/usr/X11R6/lib -lX11 -lXpm -lXext
endif
LDFLAGS    += -Wl,-O1
# Set the linker and flags.

AR          = ar
ARFLAGS     = cruv
RANLIB      = ranlib

CPPFLAGS    = -D_GNU_SOURCE
# Set the preprocessor flags.

ifeq ($(MODULAR_X),1)
INCLUDES    = $(shell pkg-config --cflags x11 xpm xext)
else
INCLUDES    = -I/usr/X11R6/include
endif
INCLUDES   += -I.
# Set the include locations.
