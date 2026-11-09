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

void XSScrollDrawButton(XSWidget *,int,int,int,int,XSPixmap *,int);

void XSTuneExpose(XSWidget *wid) {

	XSScrollDrawButton(wid, 0,0,16,16,wid->pix.bar_down	,wid->status==2);
	XSScrollDrawButton(wid,17,0,16,16,wid->pix.bar_up	,wid->status==1);
}

void XSTuneButtonrelease(XSWidget *wid) {

	wid->status=0;
	wid->global->animate--;

	XSTuneExpose(wid);
}

void XSTuneButtonpress(XSWidget *wid) {

	if(XSCheckBox(wid, 0,0,16,16)) 	wid->status=2;
	if(XSCheckBox(wid,17,0,16,16)) 	wid->status=1;

	wid->global->animate++;
	XSTuneExpose(wid);
}

void XSTuneOnChange(XSWidget *wid) {

	if(!wid->status) return;

	if(wid->status==1&&*wid->pointer<wid->max) (*wid->pointer)++;
	if(wid->status==2&&*wid->pointer>wid->min) (*wid->pointer)--;

	usleep(1);
}

XSWidget *XSTune(XSWidget *parent,int x,int y,int min,int max,int *pointer) {

	XSWidget *wid;
	
	int w=33,h=16;

	wid=XSWidgetCopy(parent,x,y,w,h,"noname",(int)XSTune);

	wid->min=min;
	wid->max=max;
	wid->pointer=pointer;

	wid->status=False;

	wid->on.event[Expose]=XSTuneExpose;
	wid->on.event[ButtonRelease]=XSTuneButtonrelease;
	wid->on.event[ButtonPress]=XSTuneButtonpress;
	wid->on.change=XSTuneOnChange;
	wid->on.create=XSWidgetCreate;

	wid->global->count++;

	return(wid);
}
