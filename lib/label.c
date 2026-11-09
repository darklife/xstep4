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

void XSLabelExpose(XSWidget *t) {

	int size;

	XSDrawBox(t,0,0,t->width,t->height,t->invisible,t->control);

	size=XSDrawText(t,t->text);

	if(t->pixmap)
		XCopyArea(t->display,t->pixmap->pixmap,t->window,t->gc,0,0,
			t->pixmap->width,t->pixmap->height,
			(t->width-t->pixmap->width)/2,
			(t->height-t->pixmap->height-size)/2);
}

void XSLabelOnChange(XSWidget *t) {

        if(t->text->textchecksum == XSCheckSum(t->text->text)) return;
        t->text->textchecksum=XSCheckSum(t->text->text);

	XClearArea(t->display,t->window,2,2,t->width-4,t->height-4,True);
}

XSWidget *XSLabel(XSWidget *parent,int x,int y,int w,int h,char *name,int s) {

	XSWidget *wid;

	wid=XSWidgetCopy(parent,x,y,w,h,name,(int)XSLabel);

	wid->text=XSCreateText(wid,name,s);
	wid->control=s;

	wid->on.event[Expose]=XSLabelExpose;
	wid->on.change=XSLabelOnChange;
	wid->on.create=XSWidgetCreate;

        wid->global->count++;
        
        return(wid);
}
