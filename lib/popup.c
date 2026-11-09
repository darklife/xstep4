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

#include "xstep.h"

void XSPopupExpose(XSWidget *wid) {

	XSDrawBox(wid,0,0,wid->width,wid->height,wid->bgcolor,wid->control);
	XSDrawBox(wid,wid->width-20,wid->height/2-3,12,6,wid->invisible,XS_BOX_UP);

	XDrawString(wid->display,wid->window,wid->gc,
		8,14,wid->list[*wid->pointer],strlen(wid->list[*wid->pointer]));
}

void XSPopupOnChange(XSWidget *wid) {

	if(wid->checksum == XSCheckSum(wid->list[*wid->pointer])) return;
	wid->checksum=XSCheckSum(wid->list[*wid->pointer]);

	XSPopupExpose(wid);
}


void XSPopupButtonpress(XSWidget *wid) {

	XSWidget 	*menu;
	Window  	foo;
	int 		x,y,pointer;

	pointer=*wid->pointer;

	XTranslateCoordinates(wid->display,
		wid->window,
		wid->desktop->window,
		0,
		0,
		&x,
		&y,
		&foo);

	menu=XSMenu(wid->desktop,x,y-(pointer*21),wid->width,wid->height*wid->size,wid->list,wid->size,wid->pointer);
}

XSWidget *XSPopup(XSWidget *parent,int x,int y,int w,int h,char **list,int *pointer) {

	XSWidget *wid;
	int	 size;

	for(size=0;list[size];size++);

	wid=XSWidgetCopy(parent,x,y,w,h,"noname",(int)XSPopup);

	wid->control=XS_BOX_UP;
	wid->status=False;
	wid->list=list;
	wid->size=size;
	wid->pointer=pointer;

	wid->on.event[Expose]=XSPopupExpose;
	wid->on.event[ButtonPress]=XSPopupButtonpress;
	wid->on.create=XSWidgetCreate;
	wid->on.change=XSPopupOnChange;

	wid->global->count++;

	return(wid);
}
