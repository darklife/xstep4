           CC = gcc
           AR = ar rcs
           LD = ld
           RM = rm -f
        SHELL = /bin/sh

     CPUFLAGS = -Wall -O6 -mpentium -funroll-loops -fomit-frame-pointer

       INCDIR = -I../include -I/usr/X11R6/include -I/usr/local/include
       LIBDIR = -L/usr/lib -L/usr/X11R6/lib -L/usr/local/lib
         LIBS = -lxstep -lX11 -lm -lXext
       CFLAGS = $(CPUFLAGS) $(DEFINES) $(INCDIR)
      LDFLAGS = $(LIBDIR) $(LIBS)

         OBJS = main.o setup.o about.o
          BIN = xstepicq

all:: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(BIN)

clean:
	$(RM) $(OBJS) $(PRIORITY) *.core *~
