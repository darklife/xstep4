#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <xstep.h>

XSWidget	*d1,*w1;
time_t 		reference;
int		seconds=False;

void XSClockExpose(XSWidget *wid) {

	struct tm *stm;
	
	int x,y,d,i;

	x=(wid->width)/2;
	y=(wid->height)/2;

	d=((wid->width>wid->height)?wid->height:wid->width)-8;

	stm=localtime(&reference);

	for(i=0;i!=60;i++) {

		if(i%5==0)
			XDrawLine(wid->display,wid->window,wid->gc,
				x+(int)(sin((i)*2.0*M_PI/60.0)*d/2.3),
				y-(int)(cos((i)*2.0*M_PI/60.0)*d/2.3),
				x+(int)(sin((i)*2.0*M_PI/60.0)*d/2.0),
				y-(int)(cos((i)*2.0*M_PI/60.0)*d/2.0));				
		else
			XDrawLine(wid->display,wid->window,wid->gc,
				x+(int)(sin((i)*2.0*M_PI/60.0)*d/2.1),
				y-(int)(cos((i)*2.0*M_PI/60.0)*d/2.1),
				x+(int)(sin((i)*2.0*M_PI/60.0)*d/2.0),
				y-(int)(cos((i)*2.0*M_PI/60.0)*d/2.0));
	}

	XDrawLine(wid->display,wid->window,wid->gc,
		x,y,
		x+(int)(sin((stm->tm_hour%12+stm->tm_min/60.0)*2.0*M_PI/12.0)*d/4.0),
		y-(int)(cos((stm->tm_hour%12+stm->tm_min/60.0)*2.0*M_PI/12.0)*d/4.0));

	XDrawLine(wid->display,wid->window,wid->gc,
		x,y,
		x+(int)(sin(stm->tm_min*2.0*M_PI/60.0)*d/2.0),
		y-(int)(cos(stm->tm_min*2.0*M_PI/60.0)*d/2.0));

	if(seconds)
		XDrawLine(wid->display,wid->window,wid->gc,
			x,y,
			x+(int)(sin(stm->tm_sec*2.0*M_PI/60.0)*d/2.0),
			y-(int)(cos(stm->tm_sec*2.0*M_PI/60.0)*d/2.0));
}

void XSClockIdle(XSWidget *wid) {

	if(seconds?(time(0)==reference):(time(0)/60==reference/60)) return;
	
	XClearArea(wid->display,wid->window,
		0,0,
		wid->width,wid->height,True);
			
	reference=time(0);
}

int main(int argc,char **argv) {

	int i;
	
	for(i=1;argv[i];i++)
		if(!strcmp(argv[i],"-seconds")) seconds=True;

	d1=XSDesktop(getenv("DISPLAY"),argv,argc);

	w1=XSWindow(d1,0,0,160,160,"xstepclock");

	w1->on.event[Expose]	=XSClockExpose;
	w1->on.change		=XSClockIdle;

	w1->fgcolor		=XSNamedColor(d1,"white");
	
	while(XSCheckEvent(d1,!XS_BLOCK)) usleep(1);

	return 0;
}
