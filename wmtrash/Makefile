

# this makefile is part of the "wmtrash" project. 
# Copyright (C) 2003 by Jean Philippe GUILLEMIN <jp.guillemin@free.fr>
# license: This software is under GPL version 2 of license


HEADERS = 

OBJECTS = 

# customize install path as you need
SRC = wmtrash.c
BIN = wmtrash
PREFIX = /usr
CONFDIR = /etc/wmtrash
DESTDIR = 
SHAREDIR = /share/wmtrash

SRCPATH = .
BINPATH = $(DESTDIR)$(PREFIX)/bin
CONFIGPATH = $(DESTDIR)$(CONFDIR)
ICONPATH = $(DESTDIR)$(PREFIX)$(SHAREDIR)
CONFIGFILE = default.cf
CFLAGS = -g -Wall
DEFINE = -D__CONFPATH='"$(CONFDIR)"' -D__CONFFILE='"$(CONFIGFILE)"' -D__ICONPATH='"$(PREFIX)$(SHAREDIR)"'

LIBCONFIG = `gtk-config --cflags --libs`


$(BIN) : $(SRC)
	gcc $(CFLAGS) $(DEFINE) $(LIBCONFIG) -o $(BIN) $(SRC)


install : $(BIN) path
	chmod 755 $(SRCPATH)/$(BIN)
	chmod 666 $(SRCPATH)/conf/*.xpm
	chmod 666 $(SRCPATH)/conf/*.cf
	strip $(SRCPATH)/$(BIN)
	cp -pf $(SRCPATH)/$(BIN) $(BINPATH)/
	cp -pf $(SRCPATH)/conf/*.xpm $(ICONPATH)/
	cp -pf $(SRCPATH)/conf/*.cf $(CONFIGPATH)/


uninstall : $(BINPATH)/$(BIN)
	rm -f $(BINPATH)/$(BIN)
	rm -rf $(CONFIGPATH)
	rm -rf $(ICONPATH)


clean :
	rm -f $(SRCPATH)/*.o
	rm -f $(SRCPATH)/$(BIN)


path : $(PREFIX)
	mkdir -p $(CONFIGPATH)
	mkdir -p $(ICONPATH)

