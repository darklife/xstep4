/*
	(c) 2003 by Marcelo Samsoniuk and Gleicon Moraes
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/videodev.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86dga.h>

#include <xstep.h>

#define WIDTH	640
#define HEIGHT	480

int 	capture, fd, channel=1, norm=4;

struct video_capability 	cap;
struct video_buffer 		fbuf;
struct video_channel 		chan;
struct video_picture 		pict;
struct video_window 		win;

XSWidget *desktop,*window;

void TVResize(XSWidget *wid) {

	XSConfigure(wid);

	win.x 		= wid->geometry.x;
	win.y 		= wid->geometry.y;
	win.width 	= WIDTH;
	win.height 	= HEIGHT;
	win.flags 	= 0;
	win.clips 	= 0;
	win.clipcount 	= 0;

	fprintf (stderr, "%dx%d %dx%d\n", win.x, win.y, win.width, win.height);

	if (ioctl (fd, VIDIOCSWIN, &win) < 0)
		perror ("VIDIOCSWIN"), exit (1);

	system("xrefresh");
}

void TVChange(XSWidget *wid) {

	static int capturecheck=1;

	if(chan.channel!=channel) {
	
		chan.channel=channel;

		if (ioctl (fd, VIDIOCSCHAN, &chan) < 0)
			perror ("VIDIOCSCHAN"), exit (1);
	}

	if(chan.norm!=norm) {
	
		chan.norm=norm;

		if (ioctl (fd, VIDIOCSCHAN, &chan) < 0)
			perror ("VIDIOCSCHAN"), exit (1);
	}

	if(capture!=capturecheck) {

		capturecheck = capture;

		if (ioctl (fd, VIDIOCCAPTURE, &capture) < 0) 
			perror ("VIDIOCCAPTURE=0"), exit (1);
	}
}

void TVCreate(XSWidget *wid) {

	int foo, bar, flags=0, width,screen;

	void *base;

	XSWidgetCreate(wid);

	screen=XScreenNumberOfScreen(wid->screen);

	fprintf(stderr,"Setting up capture\n");
	
	if (!XF86DGAQueryExtension (wid->display, &foo, &bar))
		fprintf (stderr, "X has no DGA support\n"), exit (1);

	XF86DGAQueryDirectVideo (wid->display, screen, &flags);

	if (!(flags & XF86DGADirectPresent))
		fprintf (stderr, "No DGA for this display\n"), exit (1);
	
	XF86DGAGetVideoLL (wid->display, screen, (int *) &base, &width, &foo, &bar);

	fprintf (stderr, "DGA: %d %d %d [%p], desktop: %dx%d depth %d\n",
		width, foo, bar, base,
		desktop->width,
		desktop->height,
		desktop->depth);

	if ((fd = open ("/dev/video0", O_RDWR)) < 0)
		perror ("open"), exit (1);
	
	if (ioctl (fd, VIDIOCGCAP, &cap) < 0)
		perror ("VIDIOCGCAP"), exit (1);
	
	if (!(cap.type & VID_TYPE_OVERLAY))
		fprintf (stderr, "no overlay supported\n"), exit (1);
	
	if (ioctl (fd, VIDIOCGFBUF, &fbuf))
		perror ("VIDIOCGFBUF"), exit (1);
	
	fbuf.width 		= desktop->width;
	fbuf.height 		= desktop->height;
	fbuf.depth 		= desktop->depth;
	fbuf.bytesperline 	= desktop->width*desktop->depth/8;
	fbuf.base 		= base;

	if (ioctl (fd, VIDIOCSFBUF, &fbuf) < 0)
		perror ("VIDIOCSFBUF (suid)"), exit (1);

	chan.type 	= VIDEO_TYPE_CAMERA;
	chan.norm 	= norm;
	chan.channel 	= channel;

	if (ioctl (fd, VIDIOCSCHAN, &chan) < 0)
		perror ("VIDIOCSCHAN"), exit (1);
	
	if (ioctl (fd, VIDIOCGPICT, &pict) < 0)
		perror ("VIDIOCGPICT"), exit (1);
	
	pict.depth = desktop->depth;

	if (ioctl (fd, VIDIOCSPICT, &pict) < 0)
		perror ("VIDIOCSPICT"), exit (1);

	TVResize(wid);

	capture = 1;

	if (ioctl (fd, VIDIOCCAPTURE, &capture) < 0)
		perror ("VIDIOCCAPTURE=1"), exit (1);
}

int main (int argc,char **argv)
{
	char *channels[]={
	
		"Channel 1 (TV)",
		"Channel 2 (composite)",
		"Channel 3 (SVideo)",
		"Channel 4",
		NULL
	};

	char *norms[]={
	
		"PAL",
		"NTSC",
		"SECAN",
		"AUTO",
		"PAL-M",
		NULL
	};

	desktop=XSDesktop(":0",argv,argc);

	window = XSWindow(desktop,0,0,WIDTH,HEIGHT+24+16,"desktop:TVwindow");
		 XSButton(window,-8,-8,72,24,"Quit",XSWindowClose);
		 XSPopup(window,8,-8,120,24,channels,&channel);
		 XSPopup(window,8+120+8,-8,72,24,norms,&norm);
		 XSCheck(window,8+120+8+8+80,-8,72,24,"Active",&capture,1);

	window->on.create			=TVCreate;
	window->on.change			=TVChange;
	window->on.event[ConfigureNotify]	=TVResize;

	while(XSCheckEvent(desktop,XS_BLOCK));

	capture = 0;

	if (ioctl (fd, VIDIOCCAPTURE, &capture) < 0) 
		perror ("VIDIOCCAPTURE=0"), exit (1);

	close (fd);

	return 0;
}
