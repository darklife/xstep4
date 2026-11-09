#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <xstep.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

unsigned 	black,green,orange,
		wavein,waveout,trace,dots,block,
		rate,format,stereo,word,swapin,swapout,
		freq[2],vpp[2],t,fps,amp[2],
		freqcount[2],fpscount,ampcount[2],
		fpsrate,last[2],freqcount[2],size,
		lock[2],lockcount[2],channels,
		ch1,ch2,split;

XPoint          points[2][4000];
char            message[2][128];

void XSUpdate(XSWidget *wid) {

	int 		i,start[2],min[2],max[2],ch;
	char 		buffer[4000];

	XSetForeground(wid->display,wid->gc,black);

	for(i=0;i!=channels;i++) {

		if(((i==0&&ch1)||(i==1&&ch2))&&
		    size&&!block) {

			if(!trace) {

		
				if(!dots)
					XDrawLines(wid->display,wid->window,wid->gc,
						points[i],
						size/channels,
						CoordModeOrigin);
				else	XDrawPoints(wid->display,wid->window,wid->gc,
						points[i],
						size/channels,
						CoordModeOrigin);
			}

			XDrawString(wid->display,wid->window,wid->gc,8,i*32+32,message[i],strlen(message[i]));
		}

		start[i]=last[i]=max[i]=min[i]=0;
	}

	if(t!=time(0)) {
	
		t=time(0);

		fps =fpscount;  	fpscount =0;

		for(i=0;i!=channels;i++) {

			freq[i]=freqcount[i]; 	freqcount[i]=0;

			amp[i] =ampcount[i];  	ampcount[i] =0;
		
			if(lockcount[i]>(fps/2)) lock[i]=1; else lock[i]=0;
			lockcount[i]=0;
			last[i]=0;
		}

	} else fpscount ++;
	
	if(!(size=read(wavein,buffer,channels*rate/fpsrate))) XSWindowClose(wid);

	if((channels*rate/fpsrate)!=size) return;

	for(i=0;i<=size;i++) {

		ch=i%channels;

		if(swapin) buffer[i]-=127;

		if(last[ch]!=-1&&buffer[i]<(-1)) {
		
			last[ch]=-1;
		}

		if(last[ch]==-1&&buffer[i]>1) {
		
			if(start[ch]==0&&freqcount[ch]) {

				start[ch]=i/channels;
				lockcount[ch]++;
			}

			freqcount[ch]++;
			last[ch]=1;
		}

		points[ch][i/channels].x=(((i-start[ch])*wid->width/size));
		
		if(!split)		
			points[ch][i/channels].y=-buffer[i]*(wid->height/2)/255+wid->height/2;
		else
			points[ch][i/channels].y=-buffer[i]*(wid->height/2)/255+(ch*wid->height)/3+(wid->height)/3;
		
		if(buffer[i]<min[ch]) min[ch]=buffer[i];
		if(buffer[i]>max[ch]) max[ch]=buffer[i];
		
		if(swapout) buffer[i]+=127;
	}

	for(i=0;i!=channels;i++) {

		ampcount[i]=max[i]-min[i];

		XSetForeground(wid->display,wid->gc,i?orange:green);

		if(!block&&((i==0&&ch1)||(i==1&&ch2))) {

			if(!dots) 	
				XDrawLines(wid->display,wid->window,wid->gc,
					&points[i][start[i]],
					size/channels-start[i],
					CoordModeOrigin);
			else
				XDrawPoints(wid->display,wid->window,wid->gc,
					&points[i][start[i]],
					size/channels-start[i],
					CoordModeOrigin);

			sprintf(message[i],"F=%04dHz A=%3.2fV (%02dfps %s)",
				freq[i],
				amp[i]/255.0,
				fps,
				lock[i]?"locked":"unlocked");

			XDrawString(wid->display,wid->window,wid->gc,8,i*32+32,message[i],strlen(message[i]));
		}
	}

	XFlush(wid->display);

	if(waveout) write(waveout,buffer,size);

	usleep(1);
}

int setdsp(unsigned channel) {

	if(!channel) return 0;

	if(!ioctl(channel, SNDCTL_DSP_SETFMT,&format)) {
	
		fprintf(stderr,"%d: format accept.\n",channel);
	
		if(!ioctl(channel, SNDCTL_DSP_SPEED, &rate)) {
			
			fprintf(stderr,"%d: speed accept.\n",channel);	
				
			if(!ioctl(channel, SNDCTL_DSP_STEREO,&stereo)) {
						
				fprintf(stderr,"%d: multiplex accept.\n",channel);
				fprintf(stderr,"%d: fully configurable, working in format unsigned, rate %d and %s mode.\n",
					channel,rate,stereo?"stereo":"mono");
							
				fprintf(stderr,"%d: display rate is %d Hz, block will be of %d bytes.\n",channel,fpsrate,rate/fpsrate);
		
				if(stereo) channels=2;
				else	   channels=1;
		
				return 1;
			
			} else perror("SNDCTL_DSP_STEREO");
			
		} else perror("SNDCTL_DSP_SPEED");

	} else perror("SNDCTL_DSP_SETFMT");

	return 0;
}

void XSWindowTrace(XSWidget *wid) {

	XSDestroyText(wid->text);
	wid->text=XSCreateText(wid,trace?"Trace":"Clear",XS_NONE);

	if(!(trace=!trace)) XClearWindow(wid->display,wid->parent->window);
}

void XSWindowPause(XSWidget *wid) {

	XSDestroyText(wid->text);
	wid->text=XSCreateText(wid,block?"Pause":"Go",XS_NONE);

	if(!(block=!block)) XClearWindow(wid->display,wid->parent->window);
}

void XSWindowDots(XSWidget *wid) {

	XSDestroyText(wid->text);
	wid->text=XSCreateText(wid,dots?"Dots":"Lines",XS_NONE);

	dots=!dots;
	XClearWindow(wid->display,wid->parent->window);
}

void XSWindowSplit(XSWidget *wid) {

	XSDestroyText(wid->text);
	wid->text=XSCreateText(wid,split?"Split":"Join",XS_NONE);

	split=!split;
	XClearWindow(wid->display,wid->parent->window);
}

void XSCH1(XSWidget *wid) {

	XSDestroyText(wid->text);
	wid->text=XSCreateText(wid,ch1?"CH1 On":"CH1 Off",XS_NONE);

	ch1=!ch1;
	XClearWindow(wid->display,wid->parent->window);
}

void XSCH2(XSWidget *wid) {

	XSDestroyText(wid->text);
	wid->text=XSCreateText(wid,ch2?"CH2 On":"CH2 Off",XS_NONE);

	ch2=!ch2;
	XClearWindow(wid->display,wid->parent->window);
}

int main(int argc,char **argv) {

	XSWidget *desk;
	XSWidget *wid;

	waveout =0;
	wavein  =0;
	stereo	=1;
	format  =AFMT_U8;
	rate	=44100;
	fpsrate	=50;
	fps	=fpsrate;
	swapin	=0;
	swapout	=0;
	trace	=0;
	dots	=0;
	block	=0;
	ch1	=1;
	ch2	=1;

	for(argc=1;argv[argc];argc++) {

		if(!strcmp("--input",	argv[argc])) wavein =open(argv[argc+1],O_RDONLY);
		if(!strcmp("--output",	argv[argc])) waveout=open(argv[argc+1],O_WRONLY);
		if(!strcmp("--mono",	argv[argc])) stereo=0;
		if(!strcmp("--rate",	argv[argc])) rate=atoi(argv[argc+1]);
		if(!strcmp("--swapin",	argv[argc])) swapin=1;
		if(!strcmp("--swapout",	argv[argc])) swapout=1;
	}

	if(argc==1) {
	
		printf("options:\n\n");
		printf("  --input device 	(for ex: /dev/dsp, none by default)\n");
		printf("  --output device 	(for ex: /dev/dsp, none by default)\n");
		printf("  --mono 		(mono switch, stereo by default)\n");
		printf("  --rate 		(data rate, 44.1KHz by default)\n");
		printf("  --swapin 		(swap 16 bit data in)\n");
		printf("  --swapout		(swap 16 bit data out)\n\n");
		printf("if you set input to /dev/dsp, please set your mixer to record the correct input channel!\n");
	}

	if(setdsp(wavein))  swapin =1;
	if(setdsp(waveout)) swapout=1;

	desk=XSDesktop(getenv("DISPLAY"),argv,argc);
	wid =XSWindow(desk,0,0,640,480,"xwave!");
	
	black=XSNamedColor(desk,"black");
	green=XSNamedColor(desk,"green");
	orange=XSNamedColor(desk,"orange");

	wid->bgcolor	=black;
	wid->fgcolor	=green;

	XSButton(wid,-8-80*0,-8,72,24,"Quit",XSWindowClose);
	XSButton(wid,-8-80*1,-8,72,24,"Trace",XSWindowTrace);
	XSButton(wid,-8-80*2,-8,72,24,"Pause",XSWindowPause);	
	XSButton(wid,-8-80*3,-8,72,24,"Dots",XSWindowDots);

	XSButton(wid,-8-80*4,-8,72,24,"CH1 Off",XSCH1);	

	if(channels==2) {
	
		XSButton(wid,-8-80*5,-8,72,24,"CH2 Off",XSCH2);
		XSButton(wid,-8-80*6,-8,72,24,"Split",XSWindowSplit);	
	}

	wid->on.change	=XSUpdate;
	wid->font	=desk->helvetica24b;

	while(XSCheckEvent(desk,!XS_BLOCK));

	return 0;
}
