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
# systems support dinamic liked libraries. SHELL is prefered to be any
# bourne shell variation, like sh or bash. 

           CC = gcc
           LD = ld
           RM = rm -f
        SHELL = /bin/sh

# compiler options section:
#
# in the option CPUFLAGS you must set the best configuration for your target
# machine. in most unix systems running in intel machines with gcc compiler,
# the default options will produce a expensive and fast result. the
# -mpentium isn't valid for any non-intel machine. all options will be
# invalid for other compilers! check in the cc man page.

     CPUFLAGS = -Wall -O6 -mpentium -funroll-loops -fomit-frame-pointer

# misc section:
# 
# you must define directory paths for include and libraries, in most unix
# systems these default options will be right. 

       PREFIX = /usr/local
       INCDIR = -I/usr/X11R6/include -I$(PREFIX)/include
       LIBDIR = -L/usr/lib -L/usr/X11R6/lib -L$(PREFIX)/lib 
         LIBS = -lxstep -lX11 -lXext -lGL -lGLU -lm
       CFLAGS = $(CPUFLAGS) $(DEFINES) $(INCDIR)
      LDFLAGS = $(LIBDIR) $(LIBS)
      TARGETS = xstepglxworld

all:: $(TARGETS)

clean:
	$(RM) $(TARGETS) core
