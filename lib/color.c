/*************************************************************************

	XSTEP 4.0 - Toolkit for X Window System
 	Copyright (C) 1996-2001 Marcelo Samsoniuk
 
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

int XSColor(XSWidget *wid,int red,int green,int blue) {

	XColor  color;

	switch(wid->depth) {

		case 24:
		        return(red<<16|green<<8|blue);
		case 16:
			return((red&0xf8)<<8|(green&0xfc)<<3|blue>>3);
		case 15:
		        return((red&0xf8)<<7|(green&0xf8)<<2|blue>>3);
		default:
			color.red	=red  <<8;
			color.green	=green<<8;
			color.blue	=blue <<8;

			if(!XAllocColor(wid->display,DefaultColormapOfScreen(wid->screen),&color)) {
#ifdef DEBUG
				fprintf(stderr,"warning: can't allocate color %u,%u,%u\n",
					red,green,blue);
#endif
				return(XS_INVISIBLE);
			}
			return(color.pixel);
	}
}

int XSNamedColor(XSWidget *wid,char *name) {

	XColor  color;
	int	number,red,green,blue;

	if(*name=='#') {

		number	=strtol(name+1,NULL,16);

		red	=(number&0xff0000)>>16;
		green	=(number&0x00ff00)>>8;
		blue	=(number&0x0000ff);
	
		return(XSColor(wid,red,green,blue));
	}

	if(strcmp(name,"None")) {

		if(!XAllocNamedColor(wid->display,
			DefaultColormapOfScreen(wid->screen),
			name,
			&color,
			&color)) {
#ifdef DEBUG
			fprintf(stderr,"warning: can't allocate color %s\n",
				name);
#endif
			return(XS_INVISIBLE);
		}
		return(color.pixel);

	} else return(XS_INVISIBLE);
}
