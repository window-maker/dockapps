AM_CPPFLAGS =
AM_CFLAGS   = $(dockapp_CFLAGS) $(X11_CFLAGS)

if ENABLE_DEBUG
  AM_CPPFLAGS += -DDEBUG -D_XOPEN_SOURCE=600
  AM_CFLAGS   += -std=c99 -pedantic -Wall -Wextra
endif

if ENABLE_DELT_XPMS
  AM_CPPFLAGS += -DUSE_DELT_XPMS
endif

LIBS += $(dockapp_LIBS) $(X11_LIBS)

bin_PROGRAMS = wmail

wmail_SOURCES = common.c common.h \
                config.c config.h \
                wmail.c \
                xpm/*.xpm \
                xpm_delt/*.xpm
