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

void XSButtonExpose(XSWidget *t) {

	int size=0;

	if(!t->status)

		XSDrawBox(t,0,0,t->width,t->height,t->invisible,t->control);
	else
		XSDrawBox(t,0,0,t->width,t->height,t->white,XS_BOX_DOWN);

	if(t->pixmap) {

		if(t->text) {

			t->text->control |= XS_TEXT_DOWN;
			size=XSDrawText(t,t->text);
		}

		XSetClipMask(t->display,t->gc,t->pixmap->mask);

		XSetClipOrigin(t->display,t->gc,
			(t->width-t->pixmap->width)/2,
			(t->height-t->pixmap->height-size)/2);

		XCopyArea(t->display,t->pixmap->pixmap,t->window,t->gc,0,0,
			t->pixmap->width,
			t->pixmap->height,
			(t->width-t->pixmap->width)/2,
			(t->height-t->pixmap->height-size)/2);
		
		XSetClipMask(t->display,t->gc,None);

	} else

		XSDrawText(t,t->text);
}

void XSButtonButtonRelease(XSWidget *t) {

	t->status=False;
	
	if(XSCheckBox(t,0,0,t->width,t->height)&&t->on.click) {
		
		XDefineCursor(t->display,
			t->parent->window,
			XCreateFontCursor(t->display,XC_watch));

		XFlush(t->display);
		
		t->on.click(t);

		XDefineCursor(t->display,
			t->parent->window,
			XCreateFontCursor(t->display,XC_left_ptr));
	}
	XClearWindow(t->display,t->window);
	XSButtonExpose(t);
	XFlush(t->display);
}

void XSButtonButtonPress(XSWidget *t) {

	t->status=True;

	XSButtonExpose(t);
	XFlush(t->display);
}

XSWidget *XSButton(XSWidget *parent,int x,int y,int w,int h,char *name,void (*action)(XSWidget *)) {

	XSWidget *wid;

	wid=XSWidgetCopy(parent,x,y,w,h,name,(int)XSButton);

	wid->on.click=action;
	wid->control=XS_BOX_UP;
	wid->status=False;

	wid->text=XSCreateText(wid,name,XS_NONE);

	wid->on.event[Expose]		=XSButtonExpose;
	wid->on.event[ButtonRelease]	=XSButtonButtonRelease;
	wid->on.event[ButtonPress]	=XSButtonButtonPress;
	wid->on.create			=XSWidgetCreate;

	wid->global->count++;
	
	return(wid);
}
