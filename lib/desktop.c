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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <xstep.h>

XSWidget *XSDesktop(char *target,char **argv,int argc) {

	Display		*display;
	Screen 		*screen;
	XSWidget	*aux,
			*dsk=NULL,
			*def=NULL;
	XSGlobal	*global;
	int		i,transparent=False;
/*
	unsigned	r,g,b;
*/
	for(i=1;argv[i];i++) {
	
		if(!strcmp(argv[i],"-display")) 	target=argv[i+1];
		if(!strcmp(argv[i],"-transparent")) 	transparent=True;
	}

	display=XOpenDisplay(target);
	if(!display) return(def);

	global=(XSGlobal *)malloc(sizeof(XSGlobal));
	if(!global) return(def);
	
	memset(global,0,sizeof(XSGlobal));

	global->display		=display;
	global->helvetica12m	=XLoadQueryFont(display,"-*-Helvetica-Medium-R-Normal--12-120-75-75-P-*-*");
	global->helvetica12b	=XLoadQueryFont(display,"-*-Helvetica-Bold-R-Normal--12-120-75-75-P-*-*");
	global->helvetica24b	=XLoadQueryFont(display,"-*-Helvetica-Bold-R-Normal--24-240-75-75-P-*-*");
	global->courier12m	=XLoadQueryFont(display,"-*-Courier-Medium-R-Normal--12-120-75-75-P-*-*");
	global->cursor		=XCreateFontCursor(display,XC_left_ptr);
	global->atom		=XInternAtom(display,"WM_DELETE_WINDOW",False);
	global->hints		=XAllocWMHints();
	global->hints->flags	=WindowGroupHint;
	global->name		=argv[0];
	global->argv		=argv;
	global->argc		=argc;

	global->event=(XEvent *)malloc(sizeof(XEvent));
	if(!global->event) return(def);

	for(i=0;i!=ScreenCount(display);i++) {

		screen=ScreenOfDisplay(display,i);

		aux=(XSWidget *)malloc(sizeof(XSWidget));
		if(!aux) return(def);

		memset(aux,0,sizeof(XSWidget));

		aux->name			=target;
		aux->type			=(int)XSDesktop;
		aux->display			=display;
		aux->screen			=screen;
		aux->window			=RootWindowOfScreen(screen);
		aux->gc				=DefaultGCOfScreen(screen);
		aux->depth			=DefaultDepthOfScreen(screen);
		aux->width			=WidthOfScreen(screen);
		aux->height			=HeightOfScreen(screen);
		
		aux->font			=global->helvetica12m;
		aux->helvetica12m		=global->helvetica12m;
		aux->helvetica12b		=global->helvetica12b;
		aux->helvetica24b		=global->helvetica24b;
		aux->courier12m			=global->courier12m;
		
		aux->white			=XSColor(aux,0xff,0xff,0xff);
		aux->black			=XSColor(aux,0x00,0x00,0x00);
		aux->gray			=XSColor(aux,0xae,0xaa,0xae);
		aux->darkgray			=XSColor(aux,0x51,0x51,0x51);
		aux->bgcolor			=aux->gray;
		aux->fgcolor			=aux->black;
		aux->invisible			=XS_INVISIBLE;

		aux->global			=global;
		aux->event			=global->event;
		aux->attributemask		=CWEventMask|CWCursor;
		aux->attributes.cursor		=global->cursor;

		aux->valuemask			=GCForeground|
						 GCBackground|
						 GCFont|
						 GCLineWidth|
						 GCLineStyle|
						 GCCapStyle|
						 GCJoinStyle|
						 GCGraphicsExposures;

		aux->values.line_width          =0;
		aux->values.line_style          =LineSolid;
		aux->values.cap_style           =CapRound;
		aux->values.join_style          =JoinRound;
		aux->values.font                =aux->font->fid;
		aux->values.graphics_exposures  =False;
		aux->values.foreground		=aux->fgcolor;
		aux->values.background		=aux->bgcolor;
/*                                                				
		for(r=0;r!=4&&aux->depth==8;r++)
			for(g=0;g!=4;g++)
				for(b=0;b!=4;b++)
					if(!(aux->colormap[r<<4|g<<2|b]=XSColor(aux,r<<6,g<<6,b<<6)))
						fprintf(stderr,"warning: XSColor(%s,%d,%d,%d) failed!\n",
							aux->name,r<<6,g<<6,b<<6);
*/
		if(i==DefaultScreen(display)) def=aux;
		if(transparent) {
		
			aux->attributes.background_pixmap = ParentRelative;
			aux->fgcolor=aux->white;
		}
		if((aux->last=dsk)) dsk->next=aux;

		aux->desktop=dsk=aux;
	}

	return(def);
}
