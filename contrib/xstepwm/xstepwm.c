#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <xstep.h>

#include "eventnames.h"

XSWidget *desktop,*focus;

int mx,my,active=True,bw=6,th=14+6,debug=False,fgcolor;

struct XSWMDock {

	char 		*name;
	char 		*icon;
	char 		*path;
	XSWidget 	*wid;

} dock[10]={

	{ "Next",	"next.xpm",	"xv -root -quit -max .wallpaper.jpg",NULL },
	{ "XV",		"xv.xpm",	"xv",				NULL },
	{ "Clock",	"xclock.xpm",	"xstepclock -transparent",	NULL },
	{ "Netscape",	"mozilla.xpm",	"netscape",			NULL },	
	{ "Shell",	"shell.xpm",	"xterm -ls -geometry 80x40",	NULL },
	{ "Files",	"folders.xpm",	"xfm",				NULL },	
	{ "Clock",	"xclock.xpm",	"xclock",			NULL },
	{ "Clipboard",	"xclock.xpm",	"xclipboard",			NULL },	
	{ "Xlock",	"xclock.xpm",	"xlock -inwindow",		NULL },
	{ "Logout",	"recycle.xpm",	NULL,				NULL }
};

void XSWMIdle(XSWidget *wid) { usleep(1); }

void XSWMMotionNotify(XSWidget *wid) {

	if(wid->parent!=desktop) return;

	wid->geometry.x=wid->event->xbutton.x_root-mx;
	wid->geometry.y=wid->event->xbutton.y_root-my;

	XMoveWindow(wid->display,wid->window,wid->geometry.x,wid->geometry.y);

	if(debug) 
		fprintf(stderr,"  motion notify: window %x, position=%d,%d\n",
			(unsigned)wid->window,
			wid->event->xbutton.x,
			wid->event->xbutton.y);
}

void XSWMButtonPress(XSWidget *wid) {

	if(wid->parent!=desktop) return;

	mx=wid->event->xbutton.x_root-wid->geometry.x;
	my=wid->event->xbutton.y_root-wid->geometry.y;

	focus=wid;

	XRaiseWindow(wid->display,focus->window);
	XSetInputFocus(wid->display,focus->child->window,RevertToPointerRoot,CurrentTime);

	XGrabKey(wid->display,
		23,Mod1Mask,
		focus->window,
		True,
		GrabModeAsync,
		GrabModeAsync);

	wid->on.event[MotionNotify]=XSWMMotionNotify;
}

void XSWMKeyPress(XSWidget *wid) {

	XSWidget *aux;
	
	int i,key=XLookupKeysym(&wid->event->xkey,0);

	if(wid->event->xkey.state==Mod1Mask&&key==XK_Tab) {

		if(debug)
	
			fprintf(stderr,"XSWMKeyPress: window=%d, keysym=%d, keycode=%d, state=%s\n",
				(int)wid->window,
				(int)key,
				(int)wid->event->xkey.keycode,
				(wid->event->xkey.state==Mod1Mask)?"ALT":"none");

		for(aux=focus->next;aux!=focus;aux=aux->next) {

			for(i=0;i!=10;i++)
				if(aux==dock[i].wid)
					aux=desktop->child;

			if(aux==NULL) aux=desktop->child;
			if(aux->visible) break;
		}
		
		if(aux!=focus) {
		
			focus=aux;

			XRaiseWindow(wid->display,focus->window);
			XSetInputFocus(wid->display,focus->child->window,RevertToPointerRoot,CurrentTime);
		}
	}
}

void XSWMButtonRelease(XSWidget *wid) {

	if(wid->parent!=desktop) return;

	XClearArea(wid->display,wid->window,
		2,2,wid->width-4,wid->height-4,True);

	XClearArea(wid->display,wid->child->next->window,
		2,2,th-bw,th-bw,True);

	wid->on.event[MotionNotify]=NULL;
}

void XSWMExpose(XSWidget *wid) {

	XSDrawBox(wid,0,0,wid->width,wid->height,wid->invisible,XS_BOX_UP);
	XSDrawBox(wid,bw,bw+th,wid->width-bw*2,wid->height-bw*2-th,wid->invisible,XS_BOX_UPDOWN);

	XSetForeground(wid->display,wid->gc,wid->fgcolor);
	XDrawString(wid->display,wid->window,wid->gc,
		(wid->width-XTextWidth(wid->font,wid->name,strlen(wid->name)))/2,
		bw+12,
		wid->name,strlen(wid->name));

}

void XSWMConfigureNotify(XSWidget *wid) {

	if(debug) 
		printf("  need %dx%d by %x to %x (%s)\n",
			wid->event->xconfigurerequest.width,
			wid->event->xconfigurerequest.height,
			(unsigned)wid->event->xconfigurerequest.window,
			(unsigned)wid->window,
			(wid->event->xconfigurerequest.window==wid->child->window)?
				"resized":"configured");

	if(wid->event->xconfigurerequest.window==wid->child->window) {

		if((wid->width )!=(wid->event->xconfigurerequest.width+bw*2+4)||
		   (wid->height)!=(wid->event->xconfigurerequest.height+bw*2+4+th))

			XResizeWindow(wid->display,wid->window,
				wid->width =wid->geometry.w=wid->event->xconfigurerequest.width+bw*2+4,
				wid->height=wid->geometry.h=wid->event->xconfigurerequest.height+bw*2+4+th);
			
		XMoveWindow(wid->display,wid->child->window,
			wid->child->geometry.x,
			wid->child->geometry.y);

	}
}

void XSWMDestroyNotify(XSWidget *wid) {

	if(wid->event->xdestroywindow.window==wid->child->window) {

		wid->child->window=0;
		
		wid->child->visible=False;
		wid->visible=False;

		XSWindowClose(wid);
		
		if(debug) 
						
			fprintf(stderr,"  destroy notify: requested %x from %x, processed.\n",
				(unsigned)wid->event->xdestroywindow.window,
				(unsigned)wid->window);
			
		return;
	}

	if(debug)

		fprintf(stderr,"  destroy notify: requested %x from %x, unprocessed.\n",
			(unsigned)wid->event->xdestroywindow.window,
			(unsigned)wid->window);
}

void XSWMMapNotify(XSWidget *wid) {

	if(wid->event->xmap.window==wid->child->window) {

	        focus=wid;

		XGrabKey(wid->display,
			23,Mod1Mask,
			focus->window,
			True,
			GrabModeAsync,
			GrabModeAsync);

		XMapRaised(wid->display,focus->window);
		XSetInputFocus(wid->display,focus->child->window,RevertToPointerRoot,CurrentTime);

		wid->child->visible=wid->visible=True;
		
		if(debug)
					
			fprintf(stderr,"  map notify: requested %x from %x, processed.\n",
				(unsigned)wid->event->xmap.window,
				(unsigned)wid->window);
			
		return;
	}

	if(debug) 

		fprintf(stderr,"  map notify: requested %x from %x, unprocessed.\n",
			(unsigned)wid->event->xmap.window,
			(unsigned)wid->window);
}

void XSWMUnmapNotify(XSWidget *wid) {

	if(wid->event->xunmap.window==wid->child->window) {
/*
		focus=wid->next?wid->next:desktop->child;

		XRaiseWindow(wid->display,focus->window);
		XSetInputFocus(wid->display,focus->child->window,RevertToPointerRoot,CurrentTime);

		XGrabKey(wid->display,
			23,Mod1Mask,
			focus->window,
			True,
			GrabModeAsync,
			GrabModeAsync);
*/
		XUnmapWindow(wid->display,wid->window);
		wid->child->visible=wid->visible=False;
		
		if(debug)
					
			fprintf(stderr,"  unmap notify: requested %x from %x, processed.\n",
				(unsigned)wid->event->xunmap.window,
				(unsigned)wid->window);
			
		return;
	}

	if(debug)

		fprintf(stderr,"  unmap notify: requested %x from %x, unprocessed.\n",
			(unsigned)wid->event->xunmap.window,
			(unsigned)wid->window);
}

void XSWMCreateNotify(XSWidget *wid) {

	XSWidget 	*aux,*inside=NULL,*next,*last;
	static int 	px,py;

	if(wid!=desktop) return;

	if(wid->event->xcreatewindow.override_redirect==True) return;

	for(aux=desktop->child;aux;aux=aux->next)
		if(aux->window==wid->event->xcreatewindow.window) {
			if(aux->attributes.override_redirect == True)
				return;
			else
				inside=aux;
		}

	if(!(px=wid->event->xcreatewindow.x)) px=rand()%desktop-> width/2;
	if(!(py=wid->event->xcreatewindow.y)) py=rand()%desktop->height/2;

	aux=XSWindow(desktop,
		px,py,
		wid->event->xcreatewindow.width+bw*2+4,
		wid->event->xcreatewindow.height+bw*2+4+th,
		"xstepwm.window");

  	    XSButton(aux,bw,bw,th-bw,th-bw,"",XSWindowClose);

	aux->child->control=XS_BOX_UPDOWN;

	aux->on.create=NULL;
	aux->attributes.override_redirect = True;
	aux->fgcolor=fgcolor; 

	if(debug)

		fprintf(stderr,"  *** size %u x %u\n",
			aux->width	=aux->geometry.w,
			aux->height	=aux->geometry.h);

	if(!inside) {

		XSWindow(aux,
			bw+2,
			bw+2+th,
			-(bw+2),
			-(bw+2),
			"xstepwm.child");

		aux->child->on.create=NULL;
		aux->child->window=wid->event->xcreatewindow.window;

		aux->child->geometry.x=bw+2;
		aux->child->geometry.y=bw+2+th;
		aux->child->geometry.w=wid->event->xcreatewindow.width;
		aux->child->geometry.h=wid->event->xcreatewindow.height;
		aux->child->gc=0;

	} else {

		inside->parent=aux;
			
		last=inside->last;
		next=inside->next;
	
		inside->last=NULL;
		inside->next=aux->child;
		
		if(aux->child) 
			aux->child->last=inside;
		
		aux->child=inside;
		
		if(last) last->next=next;
		if(next) next->last=last;
	}

	aux->on.event[CreateNotify]	=NULL;
        aux->on.event[Expose]           =XSWMExpose;
	aux->on.event[ButtonPress]      =XSWMButtonPress;
	aux->on.event[ButtonRelease]    =XSWMButtonRelease;
	aux->on.event[KeyPress]		=XSWMKeyPress;
	aux->on.event[MotionNotify]	=XSWMIdle;
	aux->on.event[MapNotify]	=XSWMMapNotify;
	aux->on.event[UnmapNotify]	=XSWMUnmapNotify;
	aux->on.event[DestroyNotify]	=XSWMDestroyNotify;
	aux->on.event[ConfigureNotify]	=XSWMConfigureNotify;

	aux->attributes.event_mask 	=SubstructureNotifyMask+
					 ButtonMotionMask;
	aux->visible=False;

	XSWidgetCreate(aux);

	XReparentWindow(aux->display,aux->child->window,aux->window,bw+2,bw+2+th);
	XSetWindowBorderWidth(aux->display,aux->child->window,0);

	if(debug)

		fprintf(stderr,"  create notify: window %x inside %x, done.\n",
			(unsigned)aux->child->window,
			(unsigned)aux->window);
}

void XSWMExec(XSWidget *wid) {

	char *argv[4];
	int  child,i,j,k=8;

	argv[0]="/bin/sh";
	argv[1]="-c";
	argv[2]=dock[wid->index].path;
	argv[3]=NULL;
/*
	XSetFunction(desktop->display,desktop->gc,GXinvert);

	for(i=0;i!=k/2;i++,usleep(1)) {
	
		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*i/(k*2),
			desktop->height/2-desktop->height*i/(k*2),
			desktop->width*i/k,
			desktop->height*i/k);
	
		XFlush(desktop->display);
	}

	for(j=0;i!=k;i++,j++,usleep(1)) {
	
		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*i/(k*2),
			desktop->height/2-desktop->height*i/(k*2),
			desktop->width*i/k,
			desktop->height*i/k);

		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*j/(k*2),
			desktop->height/2-desktop->height*j/(k*2),
			desktop->width*j/k,
			desktop->height*j/k);

		XFlush(desktop->display);
	}

	for(;j!=k;j++,usleep(1)) {
	
		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*j/(k*2),
			desktop->height/2-desktop->height*j/(k*2),
			desktop->width*j/k,
			desktop->height*j/k);

		XFlush(desktop->display);
	}
	
	XSetFunction(desktop->display,desktop->gc,GXcopy);
*/
	if(!(child=fork())) execv(argv[0],argv);

	if(debug) fprintf(stderr,"forked child %d: %s\n",child,argv[0]);
}

void XSWMChild(int child) {

	int i,j,k=8;

	wait(&child);
/*
	XSetFunction(desktop->display,desktop->gc,GXinvert);

	for(i=k-1;i!=k/2;i--,usleep(1)) {
	
		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*i/(k*2),
			desktop->height/2-desktop->height*i/(k*2),
			desktop->width*i/k,
			desktop->height*i/k);
	
		XFlush(desktop->display);
	}

	for(j=k-1;i!=-1;i--,j--,usleep(1)) {
	
		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*i/(k*2),
			desktop->height/2-desktop->height*i/(k*2),
			desktop->width*i/k,
			desktop->height*i/k);

		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*j/(k*2),
			desktop->height/2-desktop->height*j/(k*2),
			desktop->width*j/k,
			desktop->height*j/k);

		XFlush(desktop->display);
	}

	for(;j!=-1;j--,usleep(1)) {
	
		XDrawRectangle(desktop->display,desktop->window,desktop->gc, 
			desktop->width /2-desktop->width*j/(k*2),
			desktop->height/2-desktop->height*j/(k*2),
			desktop->width*j/k,
			desktop->height*j/k);

		XFlush(desktop->display);
	}
	
	XSetFunction(desktop->display,desktop->gc,GXcopy);
*/
}

void XSWMDoExit(XSWidget *wid) {

	XSWindowClose(wid);
	active=False;
}

void XSWMDoRestart(XSWidget *wid) {

	char *argv[3];

	argv[0]="/usr/local/bin/xstepwm";
	argv[1]="-transparent";
	argv[2]=NULL;

	execv(argv[0],argv);
}

void XSWMExit(XSWidget *wid) {

	XSWidget *aux;

	aux=XSDialog(desktop,wid,400,180,"Exit XStepWM ?");

	    XSButton(aux,-8,-8,72,24,"Exit",XSWMDoExit);
	    XSButton(aux,-88,-8,72,24,"Cancel",XSWindowClose);
	    XSButton(aux,-168,-8,72,24,"Restart",XSWMDoRestart);
}

void XSWMDockEnter(XSWidget *wid) {

	int i;

	wid->width =80;
	wid->height=80;
	wid->text=XSCreateText(wid,dock[wid->index].name,XS_TEXT_DOWN);

	XRaiseWindow(wid->display,wid->window);

	XMoveResizeWindow(wid->display,wid->window,
		wid->geometry.x-(80-52)/2,wid->geometry.y-(80-52),
		wid->width,wid->height);
	
	XClearArea(wid->display,wid->window,0,0,0,0,True);

	for(i=0;i!=wid->index;i++) 
		XMoveWindow(wid->display,dock[i].wid->window,
			dock[i].wid->geometry.x-(80-52)/2,
			dock[i].wid->geometry.y);

	for(i=wid->index+1;i!=10;i++) 
		XMoveWindow(wid->display,dock[i].wid->window,
			dock[i].wid->geometry.x+(80-52)/2,
			dock[i].wid->geometry.y);
}

void XSWMDockLeave(XSWidget *wid) {

	int i;

	wid->width =52;
	wid->height=52;
	wid->text=XSDestroyText(wid->text);
	
	XMoveResizeWindow(wid->display,wid->window,
		wid->geometry.x,wid->geometry.y,
		wid->width,wid->height);

	XLowerWindow(wid->display,wid->window);

	XClearArea(wid->display,wid->window,0,0,0,0,True);
}

int main(int argc,char **argv) {

	int 		i,x,y;
	XSWidget 	*aux;
	char		localdir[1024];

	signal(SIGCHLD,XSWMChild);

	for(i=1;argv[i];i++) {
	
		if(!strcmp(argv[i],"-debug")) debug=True;
	}

	if(!(desktop=XSDesktop(getenv("DISPLAY"),argv,argc))) return(-1);

	setenv("DISPLAY",desktop->name,1);

	for(aux=desktop;aux->last;aux=aux->last);

	for(i=0;aux;i++) {

		if(debug)

			printf("%s: found screen %d, window %x, size %dx%d, depth %d%s",
				argv[0],
				i,
				(unsigned)aux->window,
				aux->width,
				aux->height,
				aux->depth,
				desktop==aux?" (default screen)\n":"\n");

		aux=aux->next;
	}	

	fgcolor=XSNamedColor(desktop,"yellow");

	desktop->font=desktop->global->helvetica12b;
	desktop->fgcolor=fgcolor; 

	x=desktop->width/2-52*5;
	y=desktop->height-52;
	
	getcwd(localdir,1024);
	
	if(chdir(".xstepwm")) chdir("/usr/local/share/xstepwm");

	for(i=0;i!=10;i++) {
	
		dock[i].wid=XSButton(desktop,
			x+i*52,y,52,52,
			NULL,
			(i==9)?XSWMExit:XSWMExec);
			
		dock[i].wid->pixmap=XSPixmapCreateXPMF(dock[i].wid,dock[i].icon);

		dock[i].wid->attributes.override_redirect = True;
		dock[i].wid->attributes.background_pixmap = ParentRelative;
		dock[i].wid->index=i;
		dock[i].wid->control=0;

		dock[i].wid->on.event[EnterNotify]=XSWMDockEnter;
		dock[i].wid->on.event[LeaveNotify]=XSWMDockLeave;
	}

	chdir(localdir);

	desktop->on.event[CreateNotify]=XSWMCreateNotify;
	desktop->on.event[KeyPress]=XSWMKeyPress;

	XSelectInput(desktop->display,desktop->window,
		SubstructureNotifyMask|KeyPressMask);

	while(active) {
	
		XSCheckEvent(desktop,XS_BLOCK);

		if(debug) 
			fprintf(stderr,"  *** received event %s to window %x\n",
				table[desktop->event->xany.type].name,
				(unsigned)desktop->event->xany.window);

	}

	if(debug) fprintf(stderr,"  shutdown: sucess.\n");

	return 0;
}
