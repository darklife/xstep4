           CC = gcc
           LD = ld
           RM = rm -f
        SHELL = /bin/sh

     CPUFLAGS = -Wall -O6 -mpentium -funroll-loops -fomit-frame-pointer
       PREFIX = /usr/local/
       INCDIR = -I/usr/X11R6/include -I$(PREFIX)/include
       LIBDIR = -L/usr/X11R6/lib -L$(PREFIX)/lib 
         LIBS = -lxstep -lX11 -lXext -lm
       CFLAGS = $(CPUFLAGS) $(DEFINES) $(INCDIR)
      LDFLAGS = $(LIBDIR) $(LIBS)
      TARGETS = xstepwave 

all:: $(TARGETS)

clean:
	$(RM) $(TARGETS) core
