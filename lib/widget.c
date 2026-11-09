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

#include "xstep.h"

#include <xstep_check_on.xpm>
#include <xstep_check_off.xpm>
#include <xstep_radio_on.xpm>
#include <xstep_radio_off.xpm>
#include <xstep_bar_point.xpm>
#include <xstep_bar_bg.xpm>
#include <xstep_bar_up.xpm>
#include <xstep_bar_down.xpm>
#include <xstep_bar_left.xpm>
#include <xstep_bar_right.xpm>

#include <stdio.h>
#include <stdlib.h>

// will be integrated later into any struct to avoid variable colisions!

static int xlast=0, ylast=0, placement=XS_ABSOLUT;

void XSMode(int mode) {
	placement = mode;
}

XSWidget *XSWidgetCopy(XSWidget *parent,int x,int y,int w,int h,char *name,int type) {

	XSWidget *wid;

        wid=(XSWidget *)malloc(sizeof(XSWidget));
        memcpy(wid,parent,sizeof(XSWidget));
	
 	if(placement == (int) XS_DELTA) {
                 x = xlast + x;
                 y = ylast + y;
		 if(x<0) x = 0;   // only non-negative, as negative means placement relative to parent
		 if(y<0) y = 0;
        }

	wid->visible=True;

	xlast = (x>=0)?x:wid->parent->width+x-w;
	ylast = (y>=0)?y:wid->parent->height+y-h;

	wid->geometry.x=x;
	wid->geometry.y=y;
	wid->geometry.w=w;
	wid->geometry.h=h;

        wid->name=name;
        wid->type=type;                        

        wid->last       =NULL;
        wid->child      =NULL;
        wid->parent     =parent;
        wid->next       =parent->child;

        if(wid->next) wid->next->last=wid;

        return(parent->child=wid);
}

void XSWidgetCreate(XSWidget *wid) {

	int x,y,w,h,i;

	static struct {

		char	*name;
		int	value;

	} table[]={

		{ "X Protocol Error",	NoEventMask },
		{ "X Protocol Reply",	NoEventMask },
		{ "Key Press",		KeyPressMask },
		{ "Key Release",	KeyReleaseMask },
		{ "Button Press",	ButtonPressMask },
		{ "Button Release",	ButtonReleaseMask },
		{ "Motion Notify",	PointerMotionMask },
		{ "Enter Notify",	EnterWindowMask },
		{ "Leave Notify",	LeaveWindowMask },
		{ "Focus In",		FocusChangeMask },
		{ "Focus Out",		FocusChangeMask },
		{ "Keymap Notify",	KeymapStateMask },
		{ "Expose",		ExposureMask },
		{ "Graphics Expose",	NoEventMask },
		{ "No Expose",		NoEventMask },
		{ "Visibility Notify",	VisibilityChangeMask },
		{ "Create Notify",	NoEventMask },
		{ "Destroy Notify",	NoEventMask },
		{ "Unmap Notify",	StructureNotifyMask },
		{ "Map Notify",		StructureNotifyMask },
		{ "Map Request",	NoEventMask },
		{ "Reparent Notify",	NoEventMask },
		{ "Configure Notify",	StructureNotifyMask },
		{ "Configure Request",	SubstructureRedirectMask },
		{ "Gravity Notify",	StructureNotifyMask },
		{ "Resize Request",	ResizeRedirectMask },
		{ "Circulate Notify",	StructureNotifyMask },
		{ "Circulate Request",	SubstructureRedirectMask },
		{ "Property Notify",	PropertyChangeMask },
		{ "Selection Clear",	NoEventMask },
		{ "Selection Request",	NoEventMask },
		{ "Selection Notify",	NoEventMask },
		{ "Colormap Notify",	ColormapChangeMask },
		{ "Client Message",	NoEventMask },
		{ "Mapping Notify",	NoEventMask },
		{ NULL,			0 },
	};

	x=wid->geometry.x;
	y=wid->geometry.y;
	w=wid->geometry.w;
	h=wid->geometry.h;

	wid->attributemask |= (wid->attributes.background_pixmap?CWBackPixmap:CWBackPixel);

	wid->attributes.background_pixel=wid->bgcolor;

	wid->values.foreground=wid->fgcolor;
	wid->values.background=wid->bgcolor;

#ifdef DEBUG
	for(i=0;table[i].name;i++)
		if(wid->on.event[i]) {
		
			wid->attributes.event_mask |= table[i].value;
			
			fprintf(stderr,
				"XSWidgetCreate(%s): added %s\n",
				wid->name,table[i].name);
		} 
#else
	for(i=0;table[i].name;i++)
		if(wid->on.event[i])
			wid->attributes.event_mask |= table[i].value;
#endif

	if(wid->attributes.event_mask) wid->attributemask |= CWEventMask;

	if(wid->type != (int)XSMenu) {

		wid->window=XCreateWindow(wid->display,wid->parent->window,
			(x>=0)?x:wid->parent->width+x-w,
			(y>=0)?y:wid->parent->height+y-h,
			wid->width =(w>0)?w:wid->parent->width -x+w,
			wid->height=(h>0)?h:wid->parent->height-y+h,
			0,CopyFromParent,CopyFromParent,CopyFromParent,
			wid->attributemask,
			&wid->attributes);
	} else {

		wid->window=XCreateWindow(wid->display,wid->parent->window,
			x,
			y,
			wid->width =(w>0)?w:wid->parent->width -x+w,
			wid->height=(h>0)?h:wid->parent->height-y+h,
			0,CopyFromParent,CopyFromParent,CopyFromParent,
			wid->attributemask,
			&wid->attributes);
	}

	if(wid->type == (int)XSWindow) {

	        XSetClassHint(wid->display,wid->window,&wid->classhint);
	        XSetWMProtocols(wid->display,wid->window,&wid->global->atom,True);
	        XSetIconName(wid->display,wid->window,wid->name);
	        XStoreName(wid->display,wid->window,wid->name);
	
	        if(!wid->parent->parent) {
	
	                wid->global->hints->window_group = wid->window;
	                XSetCommand(wid->display,wid->window,wid->global->argv,wid->global->argc);
	                XSetWMHints(wid->display,wid->window,wid->global->hints);
	        }
	}

	if(wid->type==(int)XSCheck) {

		wid->pix.check_on               =XSPixmapCreateXPM(wid,xstep_check_on_xpm);
		wid->pix.check_off              =XSPixmapCreateXPM(wid,xstep_check_off_xpm);
	}
	
	if(wid->type==(int)XSRadio) {
	
		wid->pix.radio_on               =XSPixmapCreateXPM(wid,xstep_radio_on_xpm);
		wid->pix.radio_off              =XSPixmapCreateXPM(wid,xstep_radio_off_xpm);
	}
	
	if(wid->type==(int)XSTextScroll||wid->type==(int)XSScroll) {
	
		wid->pix.bar_point              =XSPixmapCreateXPM(wid,xstep_bar_point_xpm);
		wid->pix.bar_bg                 =XSPixmapCreateXPM(wid,xstep_bar_bg_xpm);
		wid->pix.bar_left               =XSPixmapCreateXPM(wid,xstep_bar_left_xpm);
		wid->pix.bar_right              =XSPixmapCreateXPM(wid,xstep_bar_right_xpm);
		wid->pix.bar_up                 =XSPixmapCreateXPM(wid,xstep_bar_up_xpm);
		wid->pix.bar_down               =XSPixmapCreateXPM(wid,xstep_bar_down_xpm);
	}
	
	if(wid->type==(int)XSTune) {

		wid->pix.bar_up                 =XSPixmapCreateXPM(wid,xstep_bar_up_xpm);
		wid->pix.bar_down               =XSPixmapCreateXPM(wid,xstep_bar_down_xpm);
	}
	
	wid->values.font=wid->font->fid;
	wid->values.background=wid->bgcolor;
	wid->values.foreground=wid->fgcolor;
	
	wid->valuemask |= GCForeground|GCBackground|GCFont;
	
	wid->gc=XCreateGC(wid->display,wid->window,wid->valuemask,&wid->values);

	if(wid->visible) XMapWindow(wid->display,wid->window);
	
	wid->on.create=NULL;
}
