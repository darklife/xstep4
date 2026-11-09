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
#include <unistd.h>
#include <string.h>

#include "xstep.h"

void XSEditExpose(XSWidget *t) {

	XSDrawBox(t,0,0,t->width,t->height,t->bgcolor,t->control);
	XSDrawText(t,t->text);
}

void XSEditOnChange(XSWidget *t) {

	if(t->text->textchecksum == XSCheckSum(t->text->text)+t->text->textstart) return;
	t->text->textchecksum=XSCheckSum(t->text->text)+t->text->textstart;

	if((unsigned)t->text->textstart>strlen(t->text->text)) t->text->textstart=strlen(t->text->text);

	XSDrawBox(t,0,0,t->width,t->height,t->bgcolor,t->control);
	XSDrawText(t,t->text);
}

void XSEditButtonpress(XSWidget *t) {

	XSetInputFocus(t->display,t->window,None,RevertToNone);
}

void XSEditKeypress(XSWidget *t) {

	unsigned key;
	
	key=XLookupKeysym(&t->event->xkey,0);

	switch(key) {

		case XK_Home:

			t->text->textstart=0;
			t->global->count++;
			break;

		case XK_End:

			t->text->textstart=strlen(t->text->text);
			t->global->count++;
			break;

		case XK_Left:

			if(t->text->textstart) {

				t->text->textstart--;
				t->global->count++;
			}
			break;
		
		case XK_Right:

			if((unsigned)t->text->textstart<strlen(t->text->text)) {
			
				t->text->textstart++;
				t->global->count++;
			}
			break;
/*
		case XK_Down:

			for(a=b=c=0;t->text->text[a];a++) {

				if(t->text->textstart==a)  c=b;
				if(t->text->text[a]=='\n') b++;
			}
			
			t->text->textstart=(t->text->textstart/c)*(c+1);
			t->global->count++;			
			break;

		case XK_Up:

			for(a=b=c=0;t->text->text[a];a++) {

				if(t->text->textstart==a)  c=b;
				if(t->text->text[a]=='\n') b++;
			}
			
			t->text->textstart=(t->text->textstart/c)*(c-1);
			t->global->count++;			
			break;
*/	
		case XK_BackSpace:

			if(!t->text->text[0]) break;
			t->text->textstart--;
			t->text->textcurrent--;
			memmove(t->text->text+t->text->textstart,
				t->text->text+t->text->textstart+1,
				strlen(t->text->text+t->text->textstart));
				
			t->global->count++;
			break;

		case XK_Caps_Lock:
		
			t->global->capslock=t->global->capslock?False:True;
			break;

		case XK_Shift_L:
		case XK_Shift_R:

			t->global->shift=True;
			break;
	
		default:

			if(t->text->textcurrent >= t->text->textsize) return;

			switch(key) {

				case XK_Return:
				case XK_KP_Enter:
		
					if(t->text->singleline)	return;

					t->text->textcurrent++;
					t->global->count++;
					key='\n';
					break;
			
				case XK_Tab:

					t->text->textcurrent++;
					key='\t';
					break;
			}

			if(key>256) break;

			memmove(t->text->text+t->text->textstart+1,
				t->text->text+t->text->textstart,
				strlen(t->text->text+t->text->textstart));
		
			/* t->text->text[strlen(t->text->text)+1]=0; */

			t->text->textcurrent++;
		
			if(t->global->shift||t->global->capslock) switch(key) {
		
				case '`': key='~'; break;
				case '1': key='!'; break;
				case '2': key='@'; break;
				case '3': key='#'; break;
				case '4': key='$'; break;
				case '5': key='%'; break;
				case '6': key='^'; break;
				case '7': key='&'; break; 
				case '8': key='*'; break;
				case '9': key='('; break;	
				case '0': key=')'; break;
				case '-': key='_'; break;
				case '=': key='+'; break;
				case '[': key='{'; break;
				case ']': key='}'; break;
				case ';': key=':'; break;
				case '\'': key='"'; break;
				case ',': key='<'; break;
				case '.': key='>'; break;
				case '/': key='?'; break;
				case '\\': key='|'; break;
				default: if(key<='z'&&key>='a') key+=('A'-'a');
			}

			t->text->text[t->text->textstart++]=key;

			XSWrapText(t,t->text->text,t->text->textstart-1);

			t->global->count++;
	}
#ifdef DEBUG
	printf("pressed key #%d (%c), buffer=[%s]\n",
		key,
		(key>=32&&key<=127)?key:'*',
		t->text->text);
#endif
}

void XSEditKeyrelease(XSWidget *t) {

	int key;
	
	key=XLookupKeysym(&t->event->xkey,0);
	
	switch(key) {

		case XK_Shift_L:
		case XK_Shift_R:

			t->global->shift=False;
			break;

		default:
			return;
	}

#ifdef DEBUG
	printf("released key #%d (%c), buffer=[%s]\n",key,(key>=32&&key<=127)?key:'*',t->text->text);	
#endif
}

void XSEditFocusIn(XSWidget *t) {

	t->text->control|= XS_TEXT_CURSOR;
	XSDrawBox(t,0,0,t->width,t->height,t->bgcolor,t->control);
	XSDrawText(t,t->text);
}

void XSEditFocusOut(XSWidget *t) {

	t->text->control &= (~XS_TEXT_CURSOR);
	XSDrawBox(t,0,0,t->width,t->height,t->bgcolor,t->control);
	XSDrawText(t,t->text);
}

XSWidget *XSEdit(XSWidget *parent,int x,int y,int w,int h,char *name,int size, int flags) {

	XSWidget *wid;

	wid=XSWidgetCopy(parent,x,y,w,h,name,(int)XSEdit);
/*
	if((flags&XS_TEXT_LEFT)||(flags&XS_TEXT_RIGHT)||(flags&XS_TEXT_CENTER))
		dflags|=(~XS_TEXT_LEFT)|(~XS_TEXT_RIGHT)|(~XS_TEXT_RIGHT);

	if((flags&XS_TEXT_UP)||(flags&XS_TEXT_DOWN)||(flags&XS_TEXT_CURSOR))
		dflags|=(~XS_TEXT_UP)|(~XS_TEXT_DOWN)|(~XS_TEXT_CURSOR);
*/
        wid->text=XSCreateText(wid,name,flags);
	wid->text->textsize=size;

	if(flags&XS_SINGLELINE)	wid->text->singleline=XS_TRUE;

	if(strlen(name))	wid->text->textcurrent=strlen(name);
	else			wid->text->textcurrent=0;

        wid->control=XS_BOX_DOWN|XS_BOX_HIDEONBORDER;
        wid->bgcolor=wid->white;
        wid->fgcolor=wid->black;
	wid->cursor=XCreateFontCursor(wid->display,XC_xterm);

	wid->on.event[Expose]=		XSEditExpose;
	wid->on.event[ButtonPress]=	XSEditButtonpress;
	wid->on.event[KeyPress]=	XSEditKeypress;
	wid->on.event[KeyRelease]=	XSEditKeyrelease;
	wid->on.event[FocusIn]=		XSEditFocusIn;
	wid->on.event[FocusOut]=	XSEditFocusOut;
	wid->on.change=			XSEditOnChange;
	wid->on.create=			XSWidgetCreate;

	wid->global->count++;

        return(wid);
}
