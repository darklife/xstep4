#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <asm/types.h>
#include <linux/videodev.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h> 
#include <quicktime.h>
#include <colormodels.h>

#include "xstep.h"

XSWidget 		*desktop;
XSWidget		*window;
XSWidget		*control;
XSPixmap 		*pixmap;
XSImage			*ximage[2],
			*xalpha;

struct video_buffer	 grab_set;
struct video_capability  grab_cap;
struct video_channel     grab_vid; 
struct video_mmap        grab_buf[2];
struct video_mbuf	 grab_mbuf;
int                      grab_fd,
			 grab_size;
unsigned char           *grab_data;

int 			 width =512,
			 height=384,
			 invert,
			 count,
			 channel=1,
			 frame,
			 colorize,
			 intensify,
			 interlock;

struct timeval		 t0,t1;
struct timezone		 tz;
float 			 gamma=0.0,zoom=1.0;
char 			*display,*alpha,*output,*quicktimef;
char			**channels;
int			tt,cc,**video;

quicktime_t		 *quick;

void updatef(XSWidget *wid) {

	float diff;
	unsigned 	size,x,y;
	char 		*d,*s,*s1,*s2;

	interlock=!interlock;

	grab_buf[interlock].format = VIDEO_PALETTE_RGB24;

	if(grab_vid.channel!=channel) {

		grab_vid.channel=channel;

		if (-1 == ioctl(grab_fd,VIDIOCSCHAN,&grab_vid)) {

			perror ("VIDIOCSCHAN");
			return;
		}
	}

	if(ioctl(grab_fd,VIDIOCMCAPTURE,&grab_buf[interlock]))
		perror("VIDIOCMCAPTURE");

	if(ioctl(grab_fd,VIDIOCSYNC,&grab_buf[!interlock]))
		perror("VIDIOCSYNC");

/*
	if(gamma) 	XSImageGamma	(ximage[!interlock],gamma);
	if(colorize) 	XSImageColorize	(ximage[!interlock]);
	if(invert) 	XSImageInvert	(ximage[!interlock]);
	if(intensify) 	XSImageIntensify(ximage[!interlock]);
	if(alpha) 	XSImageBlend	(xalpha,ximage[!interlock]);
*/
	if(!quicktimef || !cc) {
	
		XSImage2Pixmap(wid,ximage[!interlock],pixmap,False);
		XClearWindow(wid->display,window->window);
//		XFlush(wid->display);
	}

	frame++;

	cc++;

	if(tt!=time(0)) {
	
		printf("rate = %d fps\n",cc);
		
		cc=0;
		tt=time(0);

//		if(output) SaveXSImageToJPEG(ximage[!interlock],output);
	}
	
	if(quicktimef) quicktime_encode_video(quick,video,0);
}

void videocreatef(XSWidget *wid) {

	int i,y;

        if (-1 == (grab_fd = open("/dev/video0",O_RDWR))) {

                perror("OPEN");
                return;
        }

        if (-1 == ioctl(grab_fd,VIDIOCGCAP,&grab_cap)) {

		perror("VIDIOCGCAP");
                return;
        }

	printf( "detected device [%s]:\n"
		"  %d device type\n"
		"  %d video channels\n"
		"  %d audio channels\n"
		"  %dx%d upto %dx%d pixels\n",
		
		grab_cap.name,
		grab_cap.type,
		grab_cap.channels,
		grab_cap.audios,
		grab_cap.minwidth,
		grab_cap.minheight,
		grab_cap.maxwidth,
		grab_cap.maxheight);

	channels=(char **)malloc(sizeof(char *)*grab_cap.channels);
	
	for(i=0;i!=grab_cap.channels;i++) {
	
		channels[i]=(char *)malloc(strlen("Video Channel 0"+1));
		sprintf(channels[i],"Video Channel %d",i);
	}

	grab_vid.channel=channel;		/* entrada */
	grab_vid.type = VIDEO_TYPE_TV; 	/* camera, nao tuner */
	grab_vid.norm = 1;			/* NTSC */

	if (-1 == ioctl(grab_fd,VIDIOCSCHAN,&grab_vid)) {

		perror ("VIDIOCSCHAN");
		return;

	} else printf("VIDIOCSCHAN: ok\n");

	grab_buf[0].format 	= VIDEO_PALETTE_RGB24;
        grab_buf[0].frame  	= 0;
        grab_buf[0].width  	= width;
        grab_buf[0].height 	= height;

	grab_buf[1].format 	= VIDEO_PALETTE_RGB24;
        grab_buf[1].frame  	= 1;
        grab_buf[1].width  	= width;
        grab_buf[1].height 	= height;

        grab_size 		= width*height*3;

	grab_mbuf.size 		= grab_size*2;
	grab_mbuf.frames 	= 2;
	grab_mbuf.offsets[0]	= 0;
	grab_mbuf.offsets[1]	= grab_size;
	
	if (-1 == ioctl (grab_fd,VIDIOCGMBUF,&grab_mbuf)) {

		perror ("VIDIOCGMBUF");
		return;

	} else printf("VIDIOCGMBUF: ok\n");

	grab_data = mmap(0,
			grab_mbuf.size,
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			grab_fd,0);

	if(!grab_data) {
	
		perror ("MMAP");
		return;

	} else printf("MMAP: ok\n");

	wid->on.change=updatef;

	ximage[0] = XSImageCreate(width,height,grab_data+grab_mbuf.offsets[0],XS_IMAGE_BGR24);
	ximage[1] = XSImageCreate(width,height,grab_data+grab_mbuf.offsets[1],XS_IMAGE_BGR24);

	if(quicktimef) {
	
		printf("output quicktime file: %s\n",quicktimef);
		quick = quicktime_open(quicktimef, 0, 1);

		quicktime_set_video(quick,1,width,height,10,QUICKTIME_YUV4);
		quicktime_set_jpeg(quick,30,False);

		if(!quicktime_supported_video(quick,0)) 
			printf("failed! can't encode quicktime!\n");
		
        	video=(int **)malloc(sizeof(int *)*height);
        	
                for(y=0;y!=height;y++) 
                	video[y]=(int *)malloc(sizeof(int)*width);
	}

	XSWidgetCreate(wid);

	pixmap = XSPixmapCreate(wid,
		zoom?width*zoom:width,
		zoom?height*zoom:height,
		NULL,
		XS_PIXMAP_XSHM);

	XSetWindowBackgroundPixmap(wid->display,wid->window,pixmap->pixmap);
}

void savef(XSWidget *wid) {

	char buffer[256];
	
	(void)wid;
	
	sprintf(buffer,"xvideo-%lx.%d.jpg",time(0),frame);
//	SaveXSImageToJPEG(ximage[!interlock],buffer);
}

int main(int argc,char **argv) {

	int i,y;

	/* initialize application */

	display=getenv("DISPLAY");

	for(i=0;i!=argc;i++) {
	
		if(!strcmp(argv[i],"-gamma")&&i+1!=argc)
			gamma=atof(argv[i+1]);

		if(!strcmp(argv[i],"-channel")&&i+1!=argc)
			channel=atoi(argv[i+1]);

		if(!strcmp(argv[i],"-display")&&i+1!=argc)
			display=argv[i+1];

		if(!strcmp(argv[i],"-output")&&i+1!=argc)
			output=argv[i+1];

		if(!strcmp(argv[i],"-quicktime")&&i+1!=argc)
			quicktimef=argv[i+1];

		if(!strcmp(argv[i],"-invert"))
			invert=True;

		if(!strcmp(argv[i],"-alpha"))
			alpha=argv[i+1];

		if(!strcmp(argv[i],"-zoom"))
			zoom=atof(argv[i+1]);

		if(!strcmp(argv[i],"-colorize"))
			colorize=True;

		if(!strcmp(argv[i],"-intensify"))
			intensify=True;

		if(!strcmp(argv[i],"-size")&&i+1!=argc) {

			width=atoi(argv[i+1]);
			
			if(strchr(argv[i+1],'x')) 
				height=atoi(strchr(argv[i+1],'x')+1);
			else
				height=(width*3)/4;
		}
	} 

	if(output) printf("output file: %s\n",output);

	/* create xstep objects: desktop, pixmap, window, buttons */

	desktop = XSDesktop(display,argv,argc);
		
	if(!desktop) return -1;

	fprintf(stderr,"%s: desktop %x, size %dx%d, depth %d\n",
		argv[0],
		(int)desktop,
		desktop->width,
		desktop->height,
		desktop->depth);

	control = XSWindow(desktop,
		0,0,
		width*zoom+28+152,
		height*zoom+20,
		"xvideo");

//	XSPopup(control,-8,8+21*0,152,21,channels,&channel);

	XSCheck(control,-8,8+21*2,152,21,"Intensify (IR)",&intensify,1);
	XSCheck(control,-8,8+21*3,152,21,"Colorize (IR)",&colorize,1);
	XSCheck(control,-8,8+21*4,152,21,"Invert",&invert,1);

	XSButton(control,-88,-8,72,24,"Grab",savef);
	XSButton(control, -8,-8,72,24,"Quit",XSWindowClose);

	window = XSWindow(control,
		10,10,
		width*zoom,
		height*zoom,
		"xvideo.image");

	window->on.create=videocreatef;

	XSLabel(control,
		8,8,
		width*zoom+4,
		height*zoom+4,
		"xvideo.label",
		XS_BOX_DOWN);

//	if(alpha) 
//		xalpha = XSCreateImageFromFile(alpha);

	/* main loop */

	while(XSCheckEvent(desktop,XS_NOBLOCK));

	if(quick) quicktime_close(quick);

	return 0;
}
