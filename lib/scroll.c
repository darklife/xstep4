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
#include <xstep.h>

void XSScrollDrawButton(XSWidget *wid,int x,int y,int w,int h,XSPixmap *pix,int status) {

	XSDrawBox(wid,x,y,w,h,
		!status?wid->bgcolor:wid->white,
		!status?XS_BOX_UP:XS_BOX_DOWN);
	
	XSPixmapDraw(wid,x+3,y+3,pix);
}


void XSScrollDrawVB(XSWidget *wid,int idx,int bar,int max) {

	/* vertical bar */
/*
	max=wid->viewport->height;
	bar=wid->viewport->parent->height;
	idx=-wid->viewport->geometry.y;
*/
	if(bar<max) {

		XSetForeground(wid->display,wid->gc,wid->darkgray);
		XSetBackground(wid->display,wid->gc,wid->gray);

		XSetFillStyle(wid->display,wid->gc,FillOpaqueStippled);
		XSetStipple(wid->display,wid->gc,wid->pix.bar_bg->mask);
	
		XFillRectangle(wid->display,wid->window,wid->gc,
			3,3,16,wid->height-40);

		XSetFillStyle(wid->display,wid->gc,FillSolid);

		if((bar*(wid->height-40))/max)

			XSDrawBox(wid,
				3,3+(idx*(wid->height-40))/max,
				16,(bar*(wid->height-40))/max,
				wid->bgcolor,XS_BOX_UP);

		XSPixmapDraw(wid,
			16/2,
			(idx*(wid->height-40))/max+((bar*(wid->height-40))/max)/2,
			wid->pix.bar_point);

	} else {

		XSetForeground(wid->display,wid->gc,wid->darkgray);
		XSetBackground(wid->display,wid->gc,wid->gray);

		XSetFillStyle(wid->display,wid->gc,FillOpaqueStippled);
		XSetStipple(wid->display,wid->gc,wid->pix.bar_bg->mask);
	
		XFillRectangle(wid->display,wid->window,wid->gc,
			3,3,16,wid->height-6);

		XSetFillStyle(wid->display,wid->gc,FillSolid);	
	}
}

void XSScrollDrawHB(XSWidget *wid,int idx,int bar,int max) {

	/* horizontal bar */

	if(bar<max) {

		XSetForeground(wid->display,wid->gc,wid->darkgray);
		XSetBackground(wid->display,wid->gc,wid->gray);

		XSetFillStyle(wid->display,wid->gc,FillOpaqueStippled);
		XSetStipple(wid->display,wid->gc,wid->pix.bar_bg->mask);

		XFillRectangle(wid->display,wid->window,wid->gc,
			56,wid->height-19,wid->width-59,16);

		XSetFillStyle(wid->display,wid->gc,FillSolid);

		if((bar*(wid->width-59))/max)

			XSDrawBox(wid,
				56+(idx*(wid->width-59))/max,wid->height-19,
				(bar*(wid->width-59))/max,16,
				wid->bgcolor,XS_BOX_UP);

		XSPixmapDraw(wid,
			53+(idx*(wid->width-59))/max+((bar*(wid->width-59))/max)/2,
			wid->height-19+16/2-3,
			wid->pix.bar_point);

	} else {

		XSetForeground(wid->display,wid->gc,wid->darkgray);
		XSetBackground(wid->display,wid->gc,wid->gray);

		XSetFillStyle(wid->display,wid->gc,FillOpaqueStippled);
		XSetStipple(wid->display,wid->gc,wid->pix.bar_bg->mask);

		XFillRectangle(wid->display,wid->window,wid->gc,
			22,wid->height-19,wid->width-25,16);

		XSetFillStyle(wid->display,wid->gc,FillSolid);	
	}
}

void XSScrollExpose(XSWidget *wid) {

	XSDrawBox(wid,0,0,wid->width,wid->height,XS_INVISIBLE,XS_BOX_DOWN|XS_BOX_HIDEONBORDER);

	XSScrollDrawButton(wid, 3,wid->height-36,16,16,wid->pix.bar_up,   wid->status==1);
	XSScrollDrawButton(wid, 3,wid->height-19,16,16,wid->pix.bar_down, wid->status==2);
	XSScrollDrawButton(wid,22,wid->height-19,16,16,wid->pix.bar_left, wid->status==3);
	XSScrollDrawButton(wid,39,wid->height-19,16,16,wid->pix.bar_right,wid->status==4);

	XSetForeground(wid->display,wid->gc,wid->black);

	XDrawLine(wid->display,wid->window,wid->gc,
		21,wid->height-21,wid->width-3,wid->height-21);

	XDrawLine(wid->display,wid->window,wid->gc,
		20,2,20,wid->height-3);

	if(wid->type==(int)XSScroll) {

		XSScrollDrawHB(wid,
			-wid->viewport->geometry.x,
			wid->viewport->parent->width,
			wid->viewport->width);

		XSScrollDrawVB(wid,
			-wid->viewport->geometry.y,
			wid->viewport->parent->height,
			wid->viewport->height);

	} else {

		XSScrollDrawHB(wid,
			0,
			wid->viewport->width/200,
			wid->viewport->cols);

		XSScrollDrawVB(wid,
			wid->viewport->page,
			wid->viewport->height/16,
			wid->viewport->rows);
	}
}

void XSScrollCheck(XSWidget *wid) {

	int checksum;

	if(wid->viewport->height	+
	   wid->viewport->geometry.y 	< 
	   wid->child->height)
			
		wid->viewport->geometry.y = wid->child->height-wid->viewport->height;

	if(wid->viewport->geometry.y>0)
		wid->viewport->geometry.y=0;	

	if(wid->viewport->width		+
	   wid->viewport->geometry.x	< 
	   wid->child->width)

		wid->viewport->geometry.x = wid->child->width-wid->viewport->width;

	if(wid->viewport->geometry.x>0) 
		wid->viewport->geometry.x=0;

	if(wid->viewport->width<wid->child->width)
		wid->viewport->geometry.x = (wid->child->width-wid->viewport->width)/2;

	if(wid->viewport->height<wid->child->height) 
		wid->viewport->geometry.y = (wid->child->height-wid->viewport->height)/2;

	checksum=	wid->viewport->geometry.x^
			wid->viewport->geometry.y^
			wid->child->width^
			wid->child->height;

	if(checksum==wid->checksum) return;
	
	wid->checksum=checksum;

	XMoveResizeWindow(wid->display,wid->viewport->window,
		wid->viewport->geometry.x,
		wid->viewport->geometry.y,
		wid->viewport->width,
		wid->viewport->height);

	XSScrollDrawHB(wid,
		-wid->viewport->geometry.x,
		wid->viewport->parent->width,
		wid->viewport->width);

	XSScrollDrawVB(wid,
		-wid->viewport->geometry.y,
		wid->viewport->parent->height,
		wid->viewport->height);
}

void XSScrollConfigure(XSWidget *wid) {

	XSConfigure(wid);
	XSScrollCheck(wid);
}

void XSScrollOnChange(XSWidget *wid) {

	if(!wid->status) return;

	usleep(1);

	switch(wid->status) {

		case 1:
			wid->viewport->geometry.y+=16;
			break;

		case 2:
			wid->viewport->geometry.y-=16;
			break;
			
		case 3:
			wid->viewport->geometry.x+=16;
			break;

		case 4:
			wid->viewport->geometry.x-=16;
			break;
			
		default: return;
	}
	
	XSScrollCheck(wid);	
}

void XSScrollButtonPress(XSWidget *wid) {

	if(wid->status) return;

	if(XSCheckBox(wid, 3,wid->height-36,16,16)) {

		XSScrollDrawButton(wid, 3,wid->height-36,16,16,wid->pix.bar_up,True);
		wid->status=1;
	}
	if(XSCheckBox(wid, 3,wid->height-19,16,16)) {
	
		XSScrollDrawButton(wid, 3,wid->height-19,16,16,wid->pix.bar_down,True);
		wid->status=2;
	}
	if(XSCheckBox(wid,22,wid->height-19,16,16)) {
	
		XSScrollDrawButton(wid,22,wid->height-19,16,16,wid->pix.bar_left,True);
		wid->status=3;
	}
	if(XSCheckBox(wid,39,wid->height-19,16,16)) {
	
		XSScrollDrawButton(wid,39,wid->height-19,16,16,wid->pix.bar_right,True);
		wid->status=4;
	}

	if(!wid->status) return;

	wid->global->animate++;
}

void XSScrollButtonRelease(XSWidget *wid) {

	if(!wid->status) return;

	switch(wid->status) {

		case 1:
			XSScrollDrawButton(wid, 3,wid->height-36,16,16,wid->pix.bar_up,		False);
			break;
		case 2:
			XSScrollDrawButton(wid, 3,wid->height-19,16,16,wid->pix.bar_down,	False);
			break;
		case 3:
			XSScrollDrawButton(wid,22,wid->height-19,16,16,wid->pix.bar_left,	False);
			break;
		case 4:
			XSScrollDrawButton(wid,39,wid->height-19,16,16,wid->pix.bar_right,	False);
			break;

		default: return;
	}

	wid->global->animate--;
	wid->status=0;
}



void XSTextScrollExpose(XSWidget *wid) {

	int 	x,y,xmax,ymax;
	char 	**line;

	if(!wid->rows||!wid->cols) return;

	XSetForeground(wid->display,wid->gc,wid->white);

	XFillRectangle(wid->display,wid->window,wid->gc,
		0,(wid->line-wid->page)*16,wid->width,16);

        XSetForeground(wid->display,wid->gc,wid->black);

	ymax=wid->height/16+wid->page; 

	if(ymax>=wid->rows) ymax=wid->rows;
	
	xmax=(wid->cols>(wid->width/200+1))?wid->width/200+1:wid->cols;

	for(x=0;x!=xmax;x++) {

		if(x)
			XDrawLine(wid->display,wid->window,wid->gc,
				x*200,0,x*200,wid->height);

		line=wid->matrix[x];

		for(y=wid->page;y!=ymax;y++) {
		
			XDrawString(wid->display,wid->window,wid->gc,
				4+x*200,12+(y-wid->page)*16,line[y],strlen(line[y]));
		}
	}
}

void XSTextScrollOnChange(XSWidget *wid) {

	char 	**line;
	int 	x,newline,newpage,xmax;

	if(!wid->rows||!wid->cols) return;

	if(!wid->parent->status) return;

	usleep(1);

	newline=wid->line;
	newpage=wid->page;

	xmax=(wid->cols>(wid->width/200+1))?wid->width/200+1:wid->cols;

	XSetForeground(wid->display,wid->gc,wid->gray);

	XFillRectangle(wid->display,wid->window,wid->gc,
		0,(wid->line-wid->page)*16,wid->width,16);

	XSetForeground(wid->display,wid->gc,wid->black);

	for(x=0;x!=xmax;x++) {

		line=wid->matrix[x];

		if(x)
			XDrawLine(wid->display,wid->window,wid->gc,
				x*200,0,x*200,wid->height);

		XDrawString(wid->display,wid->window,wid->gc,
			4+x*200,12+(wid->line-wid->page)*16,line[wid->line],strlen(line[wid->line]));
	}

	if(wid->parent->status==1) {
	
		if(wid->line) wid->line--;
		if(wid->page>=wid->line) wid->page=wid->line;

		if(newpage!=wid->page) {

			XCopyArea(wid->display,wid->window,wid->window,wid->gc,
				0,0,
				wid->width,
				(wid->height/16-1)*16,
				0,16);

			XClearArea(wid->display,wid->window,
				0,0,wid->width,16,True);

	                XSScrollDrawVB(wid->parent,
	                        wid->page,
	                        wid->height/16,
	                        wid->rows);
			
			return;
		}
	}

	if(wid->parent->status==2) {
	
		if(wid->line!=wid->rows-1) wid->line++;
		if(wid->page<wid->line-wid->height/16+1) wid->page=wid->line-wid->height/16+1;

		if(newpage!=wid->page) {

			XCopyArea(wid->display,wid->window,wid->window,wid->gc,
				0,16,
				wid->width,
				(wid->height/16-1)*16,
				0,0);

			XClearArea(wid->display,wid->window,
				0,(wid->height/16-1)*16,wid->width,16,True);

	                XSScrollDrawVB(wid->parent,
	                        wid->page,
	                        wid->height/16,
	                        wid->rows);				

			return;
		}
	}

	xmax=(wid->cols>(wid->width/200+1))?wid->width/200+1:wid->cols;

	XSetForeground(wid->display,wid->gc,wid->white);

	XFillRectangle(wid->display,wid->window,wid->gc,
		0,(wid->line-wid->page)*16,wid->width,16);

	XSetForeground(wid->display,wid->gc,wid->black);

	for(x=0;x!=xmax;x++) {

		line=wid->matrix[x];

		if(x)
			XDrawLine(wid->display,wid->window,wid->gc,
				x*200,0,x*200,wid->height);

		XDrawString(wid->display,wid->window,wid->gc,
			4+x*200,12+(wid->line-wid->page)*16,line[wid->line],strlen(line[wid->line]));
	}
}

void XSTextScrollButtonPress(XSWidget *wid) {

	char 	**line;
	int 	x,newline,xmax;

	if(!wid->rows||!wid->cols) return;

	if(!wid->on.event[MotionNotify]) {

		wid->on.event[MotionNotify]=XSTextScrollButtonPress;
		wid->global->animate++;
	}

	if(wid->event->xbutton.y<0) {
	
		wid->parent->status=1;
		return;
	}

	if(wid->event->xbutton.y/16>=wid->height/16) {

		wid->parent->status=2;
		return;
	}
	
	wid->parent->status=0;
			
	newline=wid->event->xbutton.y/16+wid->page;
	
	if(newline<0) newline=0;
	if(newline>=wid->rows) newline=wid->rows-1;

	if(wid->line==newline) return;

	xmax=(wid->cols>(wid->width/200+1))?wid->width/200+1:wid->cols;

	XSetForeground(wid->display,wid->gc,wid->gray);

	XFillRectangle(wid->display,wid->window,wid->gc,
		0,(wid->line-wid->page)*16,wid->width,16);

        XSetForeground(wid->display,wid->gc,wid->black);

	for(x=0;x!=xmax;x++) {

		line=wid->matrix[x];

		XDrawString(wid->display,wid->window,wid->gc,
			4+x*200,12+(wid->line-wid->page)*16,line[wid->line],strlen(line[wid->line]));
	}

	wid->line=newline;

	XSetForeground(wid->display,wid->gc,wid->white);

	XFillRectangle(wid->display,wid->window,wid->gc,
		0,(wid->line-wid->page)*16,wid->width,16);

        XSetForeground(wid->display,wid->gc,wid->black);

	for(x=0;x!=xmax;x++) {

		if(x)
			XDrawLine(wid->display,wid->window,wid->gc,
				x*200,0,x*200,wid->height);

		line=wid->matrix[x];

		XDrawString(wid->display,wid->window,wid->gc,
			4+x*200,12+(wid->line-wid->page)*16,line[wid->line],strlen(line[wid->line]));
	}
}

void XSTextScrollButtonRelease(XSWidget *wid) {

	if(!wid->rows||!wid->cols) return;

	if(wid->on.event[MotionNotify]) {

		wid->on.event[MotionNotify]=NULL;
		wid->global->animate--;
	}
	
	wid->parent->status=0;
}

XSWidget *XSScroll(XSWidget *parent,int x,int y,int w,int h,int width,int height) {

	XSWidget *wid,*viewport;

	wid=XSWidgetCopy(parent,x,y,w,h,"noname",(int)XSScroll);

	viewport=wid;

	viewport=XSWindow(viewport,21,2,-2,-21,"scroll.viewport.box");

	viewport->attributes.event_mask=0;
	viewport->on.event[Expose]=NULL;
	viewport->type=(int)XSScroll;
	viewport->bgcolor=wid->darkgray;

	viewport=XSWindow(viewport,0,0,width,height,"scroll.viewport.window");

	viewport->type=(int)XSScroll;
	viewport->bgcolor=wid->gray;

	wid->on.event[ConfigureNotify]=XSScrollConfigure;

	wid->viewport=viewport;

	wid->on.event[Expose]=XSScrollExpose;
	wid->on.event[ButtonPress]=XSScrollButtonPress;
	wid->on.event[ButtonRelease]=XSScrollButtonRelease;

	wid->on.create=XSWidgetCreate;
	wid->on.change=XSScrollOnChange;

	wid->global->count++;

        return(wid);
}

XSWidget *XSTextScroll(XSWidget *parent,int x,int y,int w,int h,int cols,int rows,char ***matrix) {

	XSWidget *wid,*viewport;

	wid=XSWidgetCopy(parent,x,y,w,h,"noname",(int)XSTextScroll);

	wid->values.graphics_exposures=True;

	viewport=XSWindow(wid,21,2,-2,-2,"scroll.textbox");

	viewport->attributes.event_mask=PointerMotionMask;
	viewport->attributes.background_pixmap=ParentRelative;

	viewport->on.event[Expose]=XSTextScrollExpose;
	viewport->on.event[GraphicsExpose]=XSTextScrollExpose;	
	viewport->on.event[ButtonPress]=XSTextScrollButtonPress;
	viewport->on.event[ButtonRelease]=XSTextScrollButtonRelease;
	viewport->on.change=XSTextScrollOnChange;

	viewport->cols=cols;
	viewport->rows=rows;
	viewport->matrix=matrix;

	viewport->page=0;
	viewport->line=0;

	wid->viewport=viewport;

	wid->on.event[Expose]=XSScrollExpose;
	wid->on.event[ButtonPress]=XSScrollButtonPress;
	wid->on.event[ButtonRelease]=XSScrollButtonRelease;
	wid->on.change=NULL;
	wid->on.create=XSWidgetCreate;

	wid->global->count++;

        return(wid);
}
