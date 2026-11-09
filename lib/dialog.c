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
#include "xstep.h"

void XSDialogExpose(XSWidget *wid) {

	XSetForeground(wid->display,wid->gc,wid->white);

	XDrawLine(wid->display,wid->window,wid->gc,
		0,wid->height/2,
		wid->width,wid->height/2);

	XSetForeground(wid->display,wid->gc,wid->black);

	XDrawLine(wid->display,wid->window,wid->gc,
		0,wid->height/2-1,
		wid->width,wid->height/2-1);

	XSetForeground(wid->display,wid->gc,wid->fgcolor);

	if(wid->pixmap) {

		XDrawString(wid->display,wid->window,wid->gc,
			32+wid->pixmap->width,
			50,
			wid->name,strlen(wid->name));

		XSetClipMask(wid->display,wid->gc,wid->pixmap->mask);

		XSetClipOrigin(wid->display,wid->gc,16,16);

		XCopyArea(wid->display,wid->pixmap->pixmap,wid->window,wid->gc,0,0,
			wid->pixmap->width,
			wid->pixmap->height,
			16,16);

		XSetClipMask(wid->display,wid->gc,None);

	} else
		XDrawString(wid->display,wid->window,wid->gc,
			(wid->width-XTextWidth(wid->global->helvetica24b,wid->name,strlen(wid->name)))/2,
			50,
			wid->name,strlen(wid->name));
}

void XSDialogCreate(XSWidget *wid) {

	int x,y,w,h;
	
	x=wid->geometry.x;
	y=wid->geometry.y;
	w=wid->geometry.w;
	h=wid->geometry.h;

	XSWidgetCreate(wid);
 
	XSetWMNormalHints(wid->display,wid->window,wid->sizehints);
	XSetClassHint(wid->display,wid->window,&wid->classhint);
	XSetWMProtocols(wid->display,wid->window,&wid->global->atom,False);
	XSetFont(wid->display,wid->gc,wid->global->helvetica24b->fid);
	XStoreName(wid->display,wid->window,"");
	
	if(!wid->group) {

		XSetTransientForHint(wid->display,wid->window,wid->parent->window);
		wid->global->hints->window_group = wid->window;
		XSetCommand(wid->display,wid->window,wid->global->argv,wid->global->argc);
		XSetWMHints(wid->display,wid->window,wid->global->hints);

	} else 
		XSetTransientForHint(wid->display,wid->window,wid->group->window);
}

XSWidget *XSDialog(XSWidget *parent,XSWidget *group,int w,int h,char *name) {

	XSWidget 	*wid;
	int 		x,y;

	wid=XSWidgetCopy(parent,x=(parent->width-w)/2,y=(parent->height-h)/2,w,h,name,(int)XSDialog);

	if(group)
		while(group->parent!=group->desktop) 
			group=group->parent;
	
	wid->group=group;

	wid->on.event[ClientMessage]=XSWindowClose;
	wid->on.event[Expose]=XSDialogExpose;
	wid->on.create=XSDialogCreate;

	wid->classhint.res_name=wid->name;
	wid->classhint.res_class="xstep.dialog";

	wid->sizehints=XAllocSizeHints();
	wid->sizehints->x=x;
	wid->sizehints->y=y;
	wid->sizehints->min_width=w;	
	wid->sizehints->max_width=w;
	wid->sizehints->min_height=h;	
	wid->sizehints->max_height=h;
	wid->sizehints->flags=PMinSize|PMaxSize|PPosition;

	wid->global->count++;
	
	return(wid);
}
