#include <stdlib.h>
#include <math.h>
#include <xstep.h>

XSWidget *d1,*w1;

struct point { 

	double 		x,y;
	int		status; 
	struct point 	*next;

} *points;

unsigned white,black;

void w1expose(XSWidget *wid) {

	int 	x=0,y=0;
	struct point 	*aux;

	for(aux=points;aux;aux=aux->next) {

		if(points!=aux && aux->status==1)
			XDrawLine(wid->display,wid->window,wid->gc,
				x,y,aux->x+wid->width/2,aux->y+wid->height/2);

		x=aux->x+wid->width/2;
		y=aux->y+wid->height/2;
	}
}

void w1buttonpress(XSWidget *wid) {

	struct point *aux;
	
	aux=(struct point *)malloc(sizeof(struct point));

	aux->x=wid->event->xbutton.x-wid->width/2;
	aux->y=wid->event->xbutton.y-wid->height/2;
	aux->status=wid->event->xbutton.button;
	aux->next=points;

	points=aux;
}

void w1keypress(XSWidget *wid) {

	struct point *aux;
	float x,y,zsin,zcos;

	XSetForeground(wid->display,wid->gc,black);
	w1expose(wid);

	switch(XLookupKeysym(&wid->event->xkey,0)) {
	
		case XK_Page_Up:

			zsin=sin(1.74532925199e-2);
			zcos=cos(1.74532925199e-2);
		
			for(aux=points;aux;aux=aux->next) {

				x=aux->x;
				y=aux->y;
			
				aux->x=(x*zcos-(float)y*zsin);
				aux->y=(y*zcos+(float)x*zsin);
			}
			break;
		
		case XK_Page_Down:

			zsin=sin(-1.74532925199e-2);
			zcos=cos(-1.74532925199e-2);
		
			for(aux=points;aux;aux=aux->next) {
			
				x=aux->x;
				y=aux->y;
			
				aux->x=(x*zcos-(float)y*zsin);
				aux->y=(y*zcos+(float)x*zsin);
			}
			break;	

		case XK_Home:
			for(aux=points;aux;aux=aux->next)			
				aux->x=aux->x*0.9,aux->y=aux->y*0.9;
			break;

		case XK_End:
			for(aux=points;aux;aux=aux->next) 
				aux->x=aux->x*1.1,aux->y=aux->y*1.1;
			break;

		case XK_Left:
			for(aux=points;aux;aux=aux->next) 
				aux->x-=8;	
			break;
		
		case XK_Right:
			for(aux=points;aux;aux=aux->next) 
				aux->x+=8;	
			break;

		case XK_Down:
			for(aux=points;aux;aux=aux->next) 
				aux->y+=8;	
			break;
			
		case XK_Up:
			for(aux=points;aux;aux=aux->next) 
				aux->y-=8;	
			break;
		
		case XK_Escape:
			XSWindowClose(wid);
			break;
	}

	XSetForeground(wid->display,wid->gc,white);
	w1expose(wid);
}

void w1clear() {

	struct point *aux;
	
	while(points) {

		aux=points;
		points=points->next;
		free(aux);
	}

	XClearArea(w1->display,w1->window,0,0,0,0,True);
}

int main(int argc,char **argv) {

	d1=XSDesktop(getenv("DISPLAY"),argv,argc);

	white=XSNamedColor(d1,"white");
	black=XSNamedColor(d1,"black");

	w1=XSWindow(d1,0,0,800,600,"xstepdraw!");

	XSButton(w1,-4, 4,72,24,"Clear",w1clear);
	XSButton(w1,-4,32,72,24,"Quit",XSWindowClose);

	w1->on.event[Expose]		=w1expose;
	w1->on.event[ButtonRelease]	=w1expose;
	w1->on.event[ButtonPress]	=w1buttonpress;
	w1->on.event[KeyPress]		=w1keypress;

	w1->bgcolor=black;
	w1->fgcolor=white;
	
	while(XSCheckEvent(d1,XS_BLOCK));

	return 0;
}
