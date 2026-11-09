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
#include <xstep.h>

void XSRadioExpose(XSWidget *wid) {

	if((wid->status==*wid->idata))

		XSPixmapDraw(wid,3,3,wid->pix.radio_on);
	else
		XSPixmapDraw(wid,3,3,wid->pix.radio_off);

	XSetForeground(wid->display,wid->gc,wid->fgcolor);
	XDrawString(wid->display,wid->window,wid->gc,
		24,14,wid->name,strlen(wid->name));
}

void XSRadioButtonpress(XSWidget *wid) {

	*wid->idata = wid->status;

	XSRadioExpose(wid);

	wid->global->count++;
}

void XSRadioOnChange(XSWidget *wid) {

	if(*wid->idata==wid->checksum) return;
	
	wid->checksum=*wid->idata;
	XSRadioExpose(wid);
}

XSWidget *XSRadio(XSWidget *parent,int x,int y,int w,int h,char *name,int *idata,int status) {

	XSWidget *wid;

	wid=XSWidgetCopy(parent,x,y,w,h,name,(int)XSRadio);

	wid->idata=idata;
	wid->control=XS_NONE;
	wid->status=status;

	wid->on.event[Expose]=XSRadioExpose;
	wid->on.event[ButtonPress]=XSRadioButtonpress;
	wid->on.create=XSWidgetCreate;
	wid->on.change=XSRadioOnChange;

	wid->global->count++;

	return(wid);
}
