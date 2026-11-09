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

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "xstep.h"

void XSMenuExpose(XSWidget *wid) {

	int i;
	
	for(i=0;i!=wid->size;i++) {

		if(*wid->pointer == i)

			XSDrawBox(wid,0,i*21,wid->width,21,wid->white,wid->control);
		else
			XSDrawBox(wid,0,i*21,wid->width,21,wid->bgcolor,wid->control);

		XDrawString(wid->display,wid->window,wid->gc,
			8,14+i*21,wid->list[i],strlen(wid->list[i]));
	}
}

void XSMenuButtonrelease(XSWidget *wid) {

	int i,p;
	
	struct timespec a,b;

	p=*wid->pointer;

	a.tv_sec	= 0;
	a.tv_nsec	= 1000000000/20;

	if(wid->on.click) 	wid->on.click(wid);
	else			wid->global->count++;

	for(i=0;i!=6;i++) {
	
		if(i%2)
			XSDrawBox(wid,0,p*21,wid->width,21,wid->white,wid->control);
		else
			XSDrawBox(wid,0,p*21,wid->width,21,wid->bgcolor,wid->control);

		XDrawString(wid->display,wid->window,wid->gc,
			8,14+p*21,wid->list[p],strlen(wid->list[p]));	

		XFlush(wid->display);

		nanosleep(&a,&b);
	}

	XSWindowClose(wid);

	wid->global->count++;

	XFlush(wid->display);
}

void XSMenuMotion(XSWidget *wid) {

	int i;

	if(*wid->pointer == wid->global->event->xbutton.y/21) return;

	if(!XSCheckBox(wid,0,0,wid->width,wid->size*21)) {
	
		wid->status=False;
		return;
	}

	wid->status=True;

	i=*wid->pointer;

	XSDrawBox(wid,0,i*21,wid->width,21,wid->bgcolor,wid->control);

	XDrawString(wid->display,wid->window,wid->gc,
		8,14+i*21,wid->list[i],strlen(wid->list[i]));

	i=wid->global->event->xbutton.y/21;

	XSDrawBox(wid,0,i*21,wid->width,21,wid->white,wid->control);

	XDrawString(wid->display,wid->window,wid->gc,
		8,14+i*21,wid->list[i],strlen(wid->list[i]));

	*wid->pointer=wid->global->event->xbutton.y/21;
}

void XSMenuButtonpress(XSWidget *wid) {

	XSWindowClose(wid);

	wid->global->count++;
}

void XSMenuCreate(XSWidget *wid) {

	XSWidgetCreate(wid);

	XGrabPointer(wid->display,wid->window,False,
		ButtonPressMask|ButtonReleaseMask|ButtonMotionMask,
		GrabModeAsync,GrabModeAsync,
		None,
		None,
		CurrentTime);
}

XSWidget *XSMenu(XSWidget *parent,int x,int y,int w,int h,char **list,int size,int *pointer) {

	XSWidget *wid;

	wid=XSWidgetCopy(parent,x,y,w,h,"noname",(int)XSMenu);

	wid->control=XS_BOX_UP;
	wid->status=False;
	wid->list=list;
	wid->size=size;
	wid->pointer=pointer;

	wid->on.event[Expose]=XSMenuExpose;
	wid->on.event[ButtonRelease]=XSMenuButtonrelease;
	wid->on.event[MotionNotify]=XSMenuMotion;

	wid->on.create=XSMenuCreate;

	wid->global->count++;

        wid->attributemask=CWCursor|CWEventMask|CWBackPixmap|CWSaveUnder|CWOverrideRedirect;

	wid->attributes.save_under=False;
	wid->attributes.override_redirect=True;
	wid->attributes.cursor=wid->global->cursor;

	return(wid);
}
