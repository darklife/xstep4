#
#	XSTEP 4.0 - Toolkit for X Window System
# 	Copyright (C) 1996-2001 Marcelo Samsoniuk
#
#	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Library General Public
#	License as published by the Free Software Foundation; either
#	version 2 of the License, or (at your option) any later version.
#	
#	This library is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	Library General Public License for more details.
#	
#	You should have received a copy of the GNU Library General Public
#	License along with this library; if not, write to the
#	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#	Boston, MA  02111-1307  USA.
#
# XSTEP will compile in many unix systems with X11R5 or later. preliminary
# versions was sucessful tested in aix, irix, linux and freebsd machines,
# with gcc and other compilers; bsd make and gnu make will work.
# 
# compiler and binaries section:
# 
# CC is your C compiler. GNU C compiler is recomended and will be found in
# most unix systems, like linux, freebsd, netbsd, openbsd, etc. AR is for
# static linked library and LD is for dinamic linked library, most unix
# systems support dinamic linked libraries. SHELL is prefered to be any
# bourne shell variation, like sh or bash. 

           CC=gcc
           AR=ar rcs
           LD=ld -shared
           RM=rm -f
        SHELL=/bin/sh

# instalation section:
#
# PREFIX is the package prefix, preferred options are /usr/local (default
# location, is XSTEP isn't a native package in your system), usr/X11R6 or
# /usr. LDCONFIG option will run ldconfig in your machine. in freebsd
# machines you must set this option to "ldconfig -m $(PREFIX)/lib", in
# linux systems just run ldconfig (but before you must check if
# {$(PREFIX)/lib} is listed in your /etc/ld.so.conf :)

         USER=root
       PREFIX=/usr/local
      INSTALL=install -c -o $(USER) -m 644 
   LIBINSTALL=install -c -o $(USER) -m 755 -s
     LDCONFIG=ldconfig

# compiler options section:
#
# valid options in DEFINES section:
# 
#	-DDEBUG		to print debug informations!
#	-DXSHM		to enable XSHM support!
# 
# in the option CPUFLAGS you must set the best configuration for your target
# machine. in most unix systems running in intel machines with gcc compiler,
# the default options will produce a expensive and fast result. the
# -mpentium isn't valid for any non-intel machine. all options will be
# invalid for other compilers! check in the cc man page.

       DEFINES=-DXSHM
      CPUFLAGS=-O6 -funroll-loops -fomit-frame-pointer -mpentium -pipe
#     CPUFLAGS=-O6 -funroll-loops -fomit-frame-pointer -m68040   -pipe

# misc section:
# 
# you must define directory paths for include and libraries, in most unix
# systems these default options will be right. 

        INCDIR=-I../include -I/usr/X11R6/include -I$(PREFIX)/include
        LIBDIR=-L/usr/lib -L/usr/X11R6/lib -L$(PREFIX)/lib
          LIBS=-lX11 -lXext -lm
        CFLAGS=-Wall $(CPUFLAGS) $(DEFINES) $(INCDIR)
       LDFLAGS=$(LIBDIR) $(LIBS)
       LIBNAME=libxstep
       VERSION=4
       RELEASE=0
       TARGETS=$(LIBNAME).so.$(VERSION).$(RELEASE) $(LIBNAME).a
       OBJECTS=desktop.o util.o window.o tree.o button.o dialog.o edit.o \
      		label.o check.o radio.o scroll.o widget.o popup.o menu.o \
      		tune.o pixmap.o text.o color.o image.o

all:: $(TARGETS)

$(LIBNAME).so.$(VERSION).$(RELEASE):: $(OBJECTS)
	$(LD) -soname $(LIBNAME).so.$(VERSION) -o $(LIBNAME).so.$(VERSION).$(RELEASE) $(OBJECTS)

$(LIBNAME).a:: $(OBJECTS)
	$(AR) $(LIBNAME).a $(OBJECTS)
	ranlib $(LIBNAME).a

install:: $(TARGETS)
	test "$$USER" = "$(USER)"
	test -d $(PREFIX)/include
	test -d $(PREFIX)/lib
	$(INSTALL) ../include/xstep.h $(PREFIX)/include/
	$(LIBINSTALL) $(LIBNAME).a $(PREFIX)/lib/
	$(LIBINSTALL) $(LIBNAME).so.$(VERSION).$(RELEASE) $(PREFIX)/lib/
	cd $(PREFIX)/lib && ln -sf $(LIBNAME).so.$(VERSION).$(RELEASE) $(LIBNAME).so.$(VERSION)
	cd $(PREFIX)/lib && ln -sf $(LIBNAME).so.$(VERSION).$(RELEASE) $(LIBNAME).so
	test "$$USER" = "root"
	uname -s | awk '/Linux/   { system("grep $(PREFIX)/lib /etc/ld.so.conf || echo $(PREFIX)/lib >> /etc/ld.so.conf; $(LDCONFIG)") }'
	uname -s | awk '/FreeBSD/ { system("$(LDCONFIG) -m $(PREFIX)/lib/")}'

uninstall:
	$(RM) $(PREFIX)/include/xstep.h
	$(RM) $(PREFIX)/lib/$(LIBNAME).a $(PREFIX)/lib/$(LIBNAME).so.$(VERSION).*

clean:
	$(RM) $(OBJECTS) $(LIBNAME).* core
