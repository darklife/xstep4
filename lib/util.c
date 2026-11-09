/*************************************************************************

	XSTEP 4.0 - Toolkit for X Window System
 	Copyright (C) 1996-2000 Marcelo Samsoniuk
 
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA  02111-1307  USA.

*************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "xstep.h"

void XSDrawBox(XSWidget *wid,int x0,int y0,int w,int h,int c,int control) {

	int x1,y1;

	x1=x0+w-1;
	y1=y0+h-1;

/*	if(!(control&(XS_BOX_UP|XS_BOX_DOWN|XS_BOX_UPDOWN))) {

		XSetForeground(wid->display,wid->gc,c);
		XFillRectangle(wid->display,wid->window,wid->gc,x0,y0,w,h);
		XSetForeground(wid->display,wid->gc,wid->black);
		return;
	}
*/
	if(control&XS_BOX_HIDEONBORDER) {

		if(!x0&&!wid->geometry.x) { x0-=2; w+=2; }
		if(!y0&&!wid->geometry.y) { y0-=2; h+=2; }

		if(x1==wid->width-1 &&!wid->geometry.w) { x1+=2; w+=2; }
		if(y1==wid->height-1&&!wid->geometry.h) { y1+=2; h+=2; }
	}

	switch(c) {
	
		case XS_INVISIBLE:
		case XS_PIXMAP:
		case XS_COPYFROMPARENT:

//			XSetForeground(wid->display,wid->gc,wid->white);
//			XFillRectangle(wid->display,wid->window,wid->gc,x0+2,y0+2,w-4,h-4);
//			XClearArea(wid->display,wid->window,x0+2,y0+2,w-4,h-4,False);
			break;

		default:
			XSetForeground(wid->display,wid->gc,c);
			XFillRectangle(wid->display,wid->window,wid->gc,x0+2,y0+2,w-4,h-4);
			break;
	}

	switch(control&(XS_BOX_UP|XS_BOX_DOWN|XS_BOX_UPDOWN)) {

		case XS_BOX_UP:

			XSetForeground(wid->display,wid->gc,wid->white);
			XDrawLine(wid->display,wid->window,wid->gc,x0,y0,x1-1,y0);
			XDrawLine(wid->display,wid->window,wid->gc,x0,y0,x0,y1-1);
			XSetForeground(wid->display,wid->gc,wid->gray);
			XDrawLine(wid->display,wid->window,wid->gc,x0+1,y0+1,x1-2,y0+1);
			XDrawLine(wid->display,wid->window,wid->gc,x0+1,y0+1,x0+1,y1-2);
			XSetForeground(wid->display,wid->gc,wid->darkgray);
			XDrawLine(wid->display,wid->window,wid->gc,x1-1,y1-1,x0+1,y1-1);
			XDrawLine(wid->display,wid->window,wid->gc,x1-1,y1-1,x1-1,y0+1);
			XSetForeground(wid->display,wid->gc,wid->black);
			XDrawLine(wid->display,wid->window,wid->gc,x1,y1,x0,y1);
			XDrawLine(wid->display,wid->window,wid->gc,x1,y1,x1,y0);
			break;
			
		case XS_BOX_DOWN:

			XSetForeground(wid->display,wid->gc,wid->white);
			XDrawLine(wid->display,wid->window,wid->gc,x1,y1,x0,y1);
			XDrawLine(wid->display,wid->window,wid->gc,x1,y1,x1,y0);
			XSetForeground(wid->display,wid->gc,wid->gray);
			XDrawLine(wid->display,wid->window,wid->gc,x1-1,y1-1,x0+1,y1-1);
			XDrawLine(wid->display,wid->window,wid->gc,x1-1,y1-1,x1-1,y0+1);		
			XSetForeground(wid->display,wid->gc,wid->darkgray);
			XDrawLine(wid->display,wid->window,wid->gc,x0+1,y0+1,x1-2,y0+1);
			XDrawLine(wid->display,wid->window,wid->gc,x0+1,y0+1,x0+1,y1-2);
			XSetForeground(wid->display,wid->gc,wid->black);
			XDrawLine(wid->display,wid->window,wid->gc,x0,y0,x1-1,y0);
			XDrawLine(wid->display,wid->window,wid->gc,x0,y0,x0,y1-1);
			break;


		case XS_BOX_UPDOWN:
		
			XSetForeground(wid->display,wid->gc,wid->white);
			XDrawLine(wid->display,wid->window,wid->gc,x1,y1,x0,y1);
			XDrawLine(wid->display,wid->window,wid->gc,x1,y1,x1,y0);
			XDrawLine(wid->display,wid->window,wid->gc,x0+1,y0+1,x1-2,y0+1);
			XDrawLine(wid->display,wid->window,wid->gc,x0+1,y0+1,x0+1,y1-2);
			XSetForeground(wid->display,wid->gc,wid->darkgray);
			XDrawLine(wid->display,wid->window,wid->gc,x0,y0,x1-1,y0);
			XDrawLine(wid->display,wid->window,wid->gc,x0,y0,x0,y1-1);
			XDrawLine(wid->display,wid->window,wid->gc,x1-1,y1-1,x0+1,y1-1);
			XDrawLine(wid->display,wid->window,wid->gc,x1-1,y1-1,x1-1,y0+1);
			XSetForeground(wid->display,wid->gc,wid->black);
			break;
		
		default:
		
			XSetForeground(wid->display,wid->gc,wid->black);
			break;
	}
}

int XSCheckBox(XSWidget *wid,int x0,int y0,int w,int h) {

	int x1,y1;

	x1=x0+w-1;
	y1=y0+h-1;

	if(wid->event->xbutton.x<x0) return(0);
	if(wid->event->xbutton.y<y0) return(0);
	if(wid->event->xbutton.x>x1) return(0);
	if(wid->event->xbutton.y>y1) return(0);

	return(1);
}

void XSConfigure(XSWidget *wid) {

	XSWidget *aux;
	int w,h;

	wid->width =wid->event->xconfigure.width;
	wid->height=wid->event->xconfigure.height;

	for(aux=wid->child;aux;aux=aux->next) {

		w=(aux->geometry.w>0)?
			aux->geometry.w:
			wid->width-aux->geometry.x+aux->geometry.w;

		h=(aux->geometry.h>0)?
			aux->geometry.h:
			wid->height-aux->geometry.y+aux->geometry.h;

		if(!w) w=1;
		if(!h) h=1;

		XMoveResizeWindow(wid->display,aux->window,
			(aux->geometry.x>=0)?
				aux->geometry.x:
				wid->width+aux->geometry.x-aux->geometry.w,
			(aux->geometry.y>=0)?
				aux->geometry.y:
				wid->height+aux->geometry.y-aux->geometry.h,
				aux->width=w,aux->height=h);
        }
}

int XSCheckSum(char *p) {

	int i,k;

	k=(int)p;

	for(i=0;p[i];i++)
		k+=(p[i]^i);

#ifdef DEBUG
	printf("checksum of '%s': %x\n",p,k);
#endif

	return(k);
}
