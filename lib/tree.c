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
#include <stdlib.h>

#include "xstep.h"

#ifdef DEBUG

char *XSWidgetName(int f) {

	if(f==(int)XSDesktop) 		return("Desktop");
	if(f==(int)XSWindow) 		return("Window");
	if(f==(int)XSLabel) 		return("Label");
	if(f==(int)XSButton) 		return("Button");
	if(f==(int)XSDialog) 		return("Dialog");
	
	return("Unknown");
}

void XSPath(XSWidget *t) {

	if(!t) return;
	XSPath(t->parent);
	fprintf(stderr,"%s[%s].",
		XSWidgetName(t->type),
		t->name);
}

#endif

void XSTreeUnlink(XSWidget *wid) {

	if(!wid) return;

	XSTreeUnlink(wid->child);
	XSTreeUnlink(wid->next);
	
	if(wid->window) XDestroyWindow(wid->display,wid->window);
	if(wid->gc) XFreeGC(wid->display,wid->gc);

	while(wid->pixmap) XSPixmapDestroy(wid,wid->pixmap);

	free(wid);
}

void XSTreeDelete(XSWidget *wid) {

	if(wid->last||wid->next) {

		if(wid->last&&wid->next) {

			wid->last->next=wid->next;
			wid->next->last=wid->last;

		} else {
		
			if(wid->next) {
			
				wid->next->last=NULL;
				wid->parent->child=wid->next;

			} else 
				wid->last->next=NULL;
		}		

	} else 
		wid->parent->child=NULL;

	wid->next=NULL;

	XSTreeUnlink(wid);
}

int XSTree(XSWidget *t) {

#ifdef DEBUG
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
		{ "Configure Notify",	SubstructureNotifyMask },
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
	};
#endif

	if(!t) return(0);

	if(t->event->xany.window == t->window) {

#ifdef DEBUG
		fprintf(stderr,"%s: ",t->global->name);
		XSPath(t);
		fprintf(stderr,"[%s]: ",table[t->event->type].name);
#endif
	
		if(t->on.event[t->event->type]) {

			if(t->event->type == Expose) {

				t->clipmask[t->exposec].x=t->event->xexpose.x;
				t->clipmask[t->exposec].y=t->event->xexpose.y;
				t->clipmask[t->exposec].width=t->event->xexpose.width;
				t->clipmask[t->exposec].height=t->event->xexpose.height;

				t->exposec++;

				if(!t->event->xexpose.count || t->exposec == XS_EXPOSECACHESIZE) {
#ifdef DEBUG
					if(t->exposec!=1)
						fprintf(stderr,"flushed %d exposes, ",t->exposec);
#endif
					XSetClipRectangles(t->display,t->gc,
						0,0,t->clipmask,t->exposec,Unsorted);
		
					t->on.event[Expose](t);

					t->exposec=0;
	
					t->clipmask[0].x=0;
					t->clipmask[0].y=0;
					t->clipmask[0].width=t->width;
					t->clipmask[0].height=t->height;

					XSetClipRectangles(t->display,t->gc,
						0,0,t->clipmask,1,Unsorted);
#ifdef DEBUG
				} else fprintf(stderr,"cached, ");
#else
				} 
#endif

			} else t->on.event[t->event->type](t);
#ifdef DEBUG
			fprintf(stderr,"OK\n");

		} else fprintf(stderr,"ignored!\n");
#else
		}
#endif
		return(1);
	}

	if(XSTree(t->child)) return(1);
	if(XSTree(t->next))  return(1);
	
	return(0);
}

void XSBroadcast(XSWidget *t) {

	if(!t) return;

	if(t->on.destroy) {
	
		t->on.destroy(t);
		return;
	}

	if(t->on.create) t->on.create(t);
	if(t->on.change) t->on.change(t);

	XSBroadcast(t->child);
	XSBroadcast(t->next);
}

int XSCheckEvent(XSWidget *t,int flag) {

#ifdef DEBUG
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
		{ "Configure Notify",	SubstructureNotifyMask },
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
	};
#endif

	t->global->count+=t->global->animate+flag;

#ifdef DEBUG
	if(t->global->count)
		fprintf(stderr,"%s: broadcast count is %d\n",
			t->global->name,t->global->count);
#endif
	while(t->global->count) {
	
		XSBroadcast(t);
		t->global->count--;
	}

	if(flag+t->global->animate) {

		while(XPending(t->display)) {

			XNextEvent(t->display,t->event);
					
#ifdef DEBUG
			if(t->event->type < LASTEvent) {
			
				if(!XSTree(t)) 
					fprintf(stderr,"%s: no target found for [%s]\n",
						t->global->name,
						table[t->event->type].name);
			} else 
				fprintf(stderr,"%s: unknown event type %d\n",
					t->global->name,t->event->type);
#else
			XSTree(t);
#endif
		}
		
	} else {
	
		XNextEvent(t->display,t->event);

#ifdef DEBUG
		if(t->event->type < LASTEvent) {
		
			if(!XSTree(t)) 
				fprintf(stderr,"%s: no target found for [%s]\n",
					t->global->name,
					table[t->event->type].name);
		} else 
			fprintf(stderr,"%s: unknown event type %d\n",
				t->global->name,t->event->type);
#else
		XSTree(t);
#endif
	}

	return(t->child?1:0);
}
