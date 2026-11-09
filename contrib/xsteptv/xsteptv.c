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
#include <colormodels.h>

#include "xstep.h"

XSWidget 		*desktop;
XSWidget		*window;
XSWidget		*control;

XSPixmap 		*pixmap;

struct video_tuner	 video_tuner;
struct video_buffer	 video_set;
struct video_capability  video_cap;
struct video_channel     video_channel; 
struct video_mmap        video_buf[2];
struct video_mbuf	 video_mbuf;
int                      video_fd,
			 video_size;
unsigned char           *video_data;

int 			 width =640,
			 height=480,
			 invert,
			 count,
			 channel=1,
			 frame,
			 depth,
			 scan;

struct timeval		 t0,t1;
struct timezone		 tz;
float 			 gamma=0.0,zoom=1.0;
char 			*display;
char			**channels,sfrequency[64]="-";

unsigned		tablea[256],tableb[256],tablec[256],tabled[256],
			frequency=(unsigned)(32*16);

void updatef(XSWidget *wid) {

	static int frame,reference,interlock=0,lastf;

	unsigned n,*s,*d;

	interlock=!interlock;

	if(video_channel.channel!=channel) {

		video_channel.channel=channel;

		if (-1 == ioctl(video_fd,VIDIOCSCHAN,&video_channel)) {

			perror ("VIDIOCSCHAN");
			return;
		}
	}

	if(ioctl(video_fd,VIDIOCMCAPTURE,&video_buf[interlock]))
		perror("VIDIOCMCAPTURE");

	if(ioctl(video_fd,VIDIOCSYNC,&video_buf[!interlock]))
		perror("VIDIOCSYNC");

	d=(unsigned *)(pixmap->image->data);
	s=(unsigned *)(video_data+video_mbuf.offsets[!interlock]);
	n=video_size/16;
	

	if(depth==1) {

		while(n--) {

			d[0]=tablea[s[0]&0xff]+tableb[(s[0]>>8)&0xff]+tablec[(s[0]>>16)&0xff]+tabled[s[0]>>24];
			d[1]=tablea[s[1]&0xff]+tableb[(s[1]>>8)&0xff]+tablec[(s[1]>>16)&0xff]+tabled[s[1]>>24];
			d[2]=tablea[s[2]&0xff]+tableb[(s[2]>>8)&0xff]+tablec[(s[2]>>16)&0xff]+tabled[s[2]>>24];
			d[3]=tablea[s[3]&0xff]+tableb[(s[3]>>8)&0xff]+tablec[(s[3]>>16)&0xff]+tabled[s[3]>>24];

			s+=4; d+=4;
		}

	} else {
	
		while(n--) {

			d[0]=s[0];
			d[1]=s[1];
			d[2]=s[2];
			d[3]=s[3];
			
			s+=4; d+=4;
		}		
	}
	
	if(interlock) XClearWindow(wid->display,window->window);

//	XFlush(wid->display);

	frame++;

	if(reference!=time(0)) {

		if(scan) frequency++;

		if(lastf!=frequency) {

			video_tuner.flags &= (~VIDEO_TUNER_LOW);

			if (-1 == ioctl (video_fd,VIDIOCGTUNER,&video_tuner)) 
				perror ("VIDIOCGTUNER");

			if(-1 == ioctl(video_fd,VIDIOCSFREQ,&frequency))
				perror("VIDIOCSFREQ");

			if(-1 == ioctl(video_fd,VIDIOCGFREQ,&frequency))
				perror("VIDIOCGFREQ");
	
			sprintf(sfrequency,"%.3f MHz",frequency/16.0);	

			lastf=frequency;
		}

		sprintf(sfrequency,"%.6f MHz",(frequency/16.0));
	
		// printf("frequency=%.6f MHz, rate=%d fps\n",(frequency/16.0),frame);
		frame=0;
		reference=time(0);
	}
}

void scanff(XSWidget *wid) {

	scan=!scan;
}

void videocreatef(XSWidget *wid) {

	int i,r,g,b;

        if (-1 == (video_fd = open("/dev/video0",O_RDWR))) {

                perror("OPEN");
                return;
        }

        if (-1 == ioctl(video_fd,VIDIOCGCAP,&video_cap)) {

		perror("VIDIOCGCAP");
                return;
        }

	printf( "detected device [%s]:\n"
		"  %d device type\n"
		"  %d video channels\n"
		"  %d audio channels\n"
		"  %dx%d upto %dx%d pixels\n",
		
		video_cap.name,
		video_cap.type,
		video_cap.channels,
		video_cap.audios,
		video_cap.minwidth,
		video_cap.minheight,
		video_cap.maxwidth,
		video_cap.maxheight);

	channels=(char **)malloc(sizeof(char *)*(video_cap.channels+1));
	
	for(i=0;i!=video_cap.channels;i++) {
	
		channels[i]=(char *)malloc(strlen("Input xx"+1));
		sprintf(channels[i],"Input %d",i);
	}

	channels[i]=NULL;

	video_channel.channel=channel;
	video_channel.type = VIDEO_TYPE_TV;
	video_channel.norm = VIDEO_MODE_NTSC;
	
	if (-1 == ioctl(video_fd,VIDIOCSCHAN,&video_channel)) {

		perror ("VIDIOCSCHAN");
		return;

	} else printf("VIDIOCSCHAN: ok\n");

	depth = desktop->depth>16?4:(desktop->depth>8?2:1);

	video_buf[0].format 	= depth==4?VIDEO_PALETTE_RGB32:(depth==2?VIDEO_PALETTE_RGB565:VIDEO_PALETTE_HI240);
        video_buf[0].frame  	= 0;
        video_buf[0].width  	= width;
        video_buf[0].height 	= height;

	video_buf[1].format 	= depth==4?VIDEO_PALETTE_RGB32:(depth==2?VIDEO_PALETTE_RGB565:VIDEO_PALETTE_HI240);
        video_buf[1].frame  	= 1;
        video_buf[1].width  	= width;
        video_buf[1].height 	= height;

        video_size 		= width*height*depth;

	video_mbuf.size 		= video_size*2;
	video_mbuf.frames 	= 2;
	video_mbuf.offsets[0]	= 0;
	video_mbuf.offsets[1]	= video_size;
	
	if (-1 == ioctl (video_fd,VIDIOCGMBUF,&video_mbuf)) {

		perror ("VIDIOCGMBUF");
		return;

	} else printf("VIDIOCGMBUF: ok\n");

	video_data = mmap(0,
			video_mbuf.size,
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			video_fd,0);

	if(!video_data) {
	
		perror ("MMAP");
		return;

	} else printf("MMAP: ok\n");

	pixmap = XSPixmapCreate(wid,
			width,
			height,
			NULL,
			XS_PIXMAP_XSHM);

	wid->on.change=updatef;
	wid->attributes.background_pixmap = pixmap->pixmap;

	for(i=16,r=0;r!=9;r++)
		for(g=0;g!=5;g++)
			for(b=0;b!=5;b++,i++) {

				tablea[i]=XSColor(desktop,(r*31.8),(g*63.75),(b*63.75))&0xff;
				tableb[i]=tablea[i]<<8;
				tablec[i]=tablea[i]<<16;
				tabled[i]=tablea[i]<<24;
			}

	XSWidgetCreate(wid);

	XSPopup(control,8,-8,100,21,channels,&channel);	

	video_tuner.tuner=0;

	if (-1 == ioctl (video_fd,VIDIOCGTUNER,&video_tuner)) {

		perror ("VIDIOCGTUNER");
		return;

	} else {
	
	
		printf("detected tuner in channel %d:\n",video_channel.channel);
		printf("  number=%d\n",video_tuner.tuner);
		printf("  name=%s\n",video_tuner.name);

		printf("  frequency range from %u up to %u\n",
			video_tuner.rangelow,
			video_tuner.rangehigh);

		printf("  flags=%d (PAL=%d,NTSC=%d,SECAM=%d,LOW=%d,mode=%d)\n",
			video_tuner.flags,
			video_tuner.flags&VIDEO_TUNER_PAL,
			video_tuner.flags&VIDEO_TUNER_NTSC,
			video_tuner.flags&VIDEO_TUNER_SECAM,
			video_tuner.flags&VIDEO_TUNER_LOW,
			video_tuner.mode);

		video_tuner.mode=VIDEO_MODE_NTSC;

		printf("  signal=%d\n",video_tuner.signal);

		//XSTune(control,116+100,-(8+24),32*16,500*16,&frequency);
		//XSLabel(control,116,-(8+24),100,21,sfrequency,XS_BOX_DOWN);
		//XSButton(control,-88,-8,72,24,"Scan",scanff);
	}
}

int main(int argc,char **argv) {

	int i;

	display=getenv("DISPLAY");

	for(i=0;i!=argc;i++) {
	
		if(!strcmp(argv[i],"-channel")&&i+1!=argc)
			channel=atoi(argv[i+1]);

		if(!strcmp(argv[i],"-display")&&i+1!=argc)
			display=argv[i+1];

		if(!strcmp(argv[i],"-zoom"))
			zoom=atof(argv[i+1]);

		if(!strcmp(argv[i],"-freq"))
			frequency=(unsigned)(atof(argv[i+1])*16);

		if(!strcmp(argv[i],"-size")&&i+1!=argc) {

			width=atoi(argv[i+1]);
			
			if(strchr(argv[i+1],'x')) 
				height=atoi(strchr(argv[i+1],'x')+1);
			else
				height=(width*3)/4;
		}
	} 

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
		width,
		height+16+24,
		"xstepTV");

//	control->bgcolor = window->black;
//	control->fgcolor = window->white;

	XSButton(control, -8,-8,72,24,"Quit",XSWindowClose);

	window = XSWindow(control,
		0,0,
		width,
		height,
		"xstepTV.image");

	window->on.create=videocreatef;

	while(desktop->child) XSCheckEvent(desktop,XS_NOBLOCK);

	return 0;
}
