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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "xstep.h"

XSText *XSCreateText(XSWidget *wid,char *text,int control) {

	XSText *tmp;

	if(!text) return NULL;
	
	tmp=(XSText *)malloc(sizeof(XSText));
	memset(tmp,0,sizeof(XSText));

	tmp->text=text;
	tmp->control=control;
	tmp->textstart=strlen(text);
	tmp->font=NULL;

	return(tmp);
}

XSText *XSDestroyText(XSText *text) {

	XSText *aux;
	
	aux=text->next;
	free(text);
	
	return aux;
}

void XSWrapText(XSWidget *wid,char *text,int pos) {

	int  i,pi=0,qi=0;
	char *p,*q;
	
	p=q=text;
	
	for(i=0;i!=pos;i++) {
	
		if(text[i]=='\n') { p=text+i; pi=i; }
		if(text[i]<=32)   { q=text+i; qi=i; }
	}

	if(XTextWidth(wid->font,p,pos-pi)>(wid->width-12)) *q='\n';
}

int XSDrawText(XSWidget *wid,XSText *text) {

	int a,linemax,line,x,y,size,dx;
	char *tmp=text->text;

	if(!text) return 0;

	if(text->font == NULL) text->font=wid->font;

	if(text->font!=wid->font) XSetFont(wid->display,wid->gc,text->font->fid);

	size=text->font->ascent+text->font->descent+1;
	XSetBackground(wid->display,wid->gc,wid->bgcolor);
	XSetForeground(wid->display,wid->gc,wid->fgcolor);

	for(a=0,linemax=1;tmp[a];a++) if(tmp[a]=='\n') linemax++;

	switch(text->control&(XS_TEXT_DOWN|XS_TEXT_UP)) {

		case XS_TEXT_UP:
		
			y=0;
			break;
		
		case XS_TEXT_DOWN:
		
			y=wid->height-(8+linemax*size);
			break;
			
		default:
		
			y=(wid->height-(linemax*size))/2-text->font->descent;
			break;
	}

	for(line=0;line!=linemax;line++) {

		y+=size;

		for(a=0;tmp[a]&&tmp[a]!='\n';a++);

		switch(text->control&(XS_TEXT_RIGHT|XS_TEXT_LEFT)) {
		
			case XS_TEXT_RIGHT:
			
				x=wid->width-(4+XTextWidth(text->font,tmp,a));
				break;
				
			case XS_TEXT_LEFT:
			
				for(x=4;*tmp=='\t';tmp++,a--) x+=40;
				break;
			
			default:
				x=(wid->width-(XTextWidth(text->font,tmp,a)))/2;
		}

		XDrawString(wid->display,wid->window,wid->gc,x,y,tmp,a);

		if(text->control&XS_TEXT_CURSOR) {

			dx=tmp-(text->text);
				
			if(text->textstart>=dx&&text->textstart<=dx+a) {

				dx=XTextWidth(text->font,tmp,(text->textstart)-dx);

				/* XSetFunction(wid->display,wid->gc,GXinvert); */

				XDrawLine(wid->display,wid->window,wid->gc,
					x+dx,1+y,x+dx,4+y-size);

				/* XSetFunction(wid->display,wid->gc,GXcopy); */
			}
		}


		tmp+=(a+1);
	}

	if(text->font!=wid->font) XSetFont(wid->display,wid->gc,wid->font->fid);
	
	return(linemax*size);
}
