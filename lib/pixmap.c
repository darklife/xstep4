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

void XSPixmapUpdate(XSWidget *wid,XSPixmap *pixmap) {

#ifdef XSHM
	if(pixmap->type==XS_PIXMAP_XSHM) return;
#endif

	XPutImage(wid->display,
		pixmap->pixmap,
		pixmap->gc,
		pixmap->image,
		0,0,0,0,
		pixmap->width,
		pixmap->height);
}

void XSPixmapDraw(XSWidget *wid,int x,int y,XSPixmap *pix) {

	XSetClipMask(wid->display,wid->gc,pix->mask);
	XSetClipOrigin(wid->display,wid->gc,x,y);
/*
	XCopyArea(wid->display,pix->mask,wid->window,wid->gc,
		0,0,pix->width,pix->height,
		x,y);
*/
	XCopyArea(wid->display,pix->pixmap,wid->window,wid->gc,
		0,0,pix->width,pix->height,
		x,y);

	XCopyArea(wid->display,pix->pixmap,wid->window,wid->gc,
		0,0,pix->width,pix->height,
		x,y);

	XSetClipMask(wid->display,wid->gc,None);
}

XSPixmap *XSPixmapCreate(XSWidget *wid,int width,int height,void *buffer,int type) {

	int 		pad,ignore;
	XSPixmap	*pixmap;

	pixmap=(XSPixmap *)malloc(sizeof(XSPixmap));
	memset(pixmap,0,sizeof(XSPixmap));

	pixmap->width	=width;
	pixmap->height	=height;

	switch(wid->depth) {
	
		case 24: pad=4; break;
		case 16:
		case 15: pad=2; break;
		default: pad=1; break;
	}

#ifdef XSHM
	if(type==XS_PIXMAP_XSHM)
		if(*wid->desktop->name!=':')
			type=XS_PIXMAP_DEFAULT;

	if(type==XS_PIXMAP_XSHM)
		if(!XQueryExtension(wid->display,"MIT-SHM",&ignore,&ignore,&ignore)) 
			type=XS_PIXMAP_DEFAULT;

	if(type==XS_PIXMAP_XSHM) {

		pixmap->image = XShmCreateImage(wid->display,
	        	DefaultVisualOfScreen(wid->screen),
			wid->depth,
			ZPixmap, 
			buffer,
			&pixmap->xshm,
			width,height);

		pixmap->xshm.shmid = shmget(IPC_PRIVATE,
			pixmap->image->bytes_per_line*height,
			IPC_CREAT|0777);

		pixmap->xshm.shmaddr = (char *)shmat(pixmap->xshm.shmid,buffer,SHM_RND);

		pixmap->image->data=pixmap->xshm.shmaddr;

		pixmap->xshm.readOnly = False;
		XShmAttach(wid->display,&pixmap->xshm);

		pixmap->pixmap=XShmCreatePixmap(wid->display,
			wid->window,
			pixmap->image->data,
			&pixmap->xshm,
			width,
			height,
			DefaultDepthOfScreen(wid->screen));
#ifdef DEBUG
		fprintf(stderr,"XSHM image %p with shared buffer in %p (bpl=%d, calculated=%d, size=%dx%d)\n",
			pixmap->image,
			pixmap->image->data,
			pixmap->image->bytes_per_line,
			pixmap->width*pad,
			pixmap->width,
			pixmap->height);
#endif
	}
#else
	type=XS_PIXMAP_DEFAULT;
#endif

	if(type==XS_PIXMAP_DEFAULT) {

		pixmap->image=XCreateImage(wid->display,
			DefaultVisualOfScreen(wid->screen),
			DefaultDepthOfScreen(wid->screen),
			ZPixmap,
			0,
			buffer,
			width,
			height,
			pad*8,
			pad*width);

		if(!buffer) 	pixmap->image->data=(char *)malloc(pad*width*height);
		else   		pixmap->image->data=buffer;

		pixmap->pixmap=XCreatePixmap(wid->display,
			wid->window,
			width,height,
			DefaultDepthOfScreen(wid->screen));
	}

	pixmap->gc=XCreateGC(wid->display,
		pixmap->pixmap,
		0,
		&wid->values);

	pixmap->type=type;

	pixmap->next=wid->pixmap;
	
	return(wid->pixmap=pixmap);
}

void XSPixmapDestroy(XSWidget *wid,XSPixmap *pix) {

	XSPixmap *aux;

	if(pix->pixmap) {

		XFreeGC(wid->display,pix->gc);
		XFreePixmap(wid->display,pix->pixmap);
	}
	
	if(pix->image) {

#ifdef XSHM
		if(pix->type==XS_PIXMAP_XSHM) {

			XShmDetach(wid->display,&pix->xshm);
			shmdt(pix->image->data);
			shmctl(pix->xshm.shmid,IPC_RMID,NULL);
		}
#endif

		XDestroyImage(pix->image);
	}

	if(wid->pixmap == pix) wid->pixmap=pix->next;

	for(aux=wid->pixmap;aux;aux=aux->next)

		if(aux->next == pix) {
		
			aux->next=pix->next;
			break;
		}
	
	free(pix);
}

XSPixmap *XSPixmapCreateXPM(XSWidget *wid,char *xpm[]) {

	int 		ncolors,i,color[65536],bpl,x,y,z;
	unsigned 	indx=0;
	char 		name[256];
	unsigned char	sindx[3],*line;
	XImage 		*xpixmap,*xbitmap;

	XSPixmap	*pixmap;

	pixmap=(XSPixmap *)malloc(sizeof(XSPixmap));
	memset(pixmap,0,sizeof(XSPixmap));

	sscanf(xpm[0],"%d %d %d %d",&pixmap->width,&pixmap->height,&ncolors,&z);

	switch(wid->depth) {
	
		case 24: bpl=4; break;
		case 16:
		case 15: bpl=2; break;
		default: bpl=1; break;
	}

	xpixmap=XCreateImage(wid->display,DefaultVisualOfScreen(wid->screen),DefaultDepthOfScreen(wid->screen),
		ZPixmap,0,NULL,pixmap->width,pixmap->height,bpl*8,bpl*pixmap->width);

	xpixmap->data=(char *)malloc(bpl*pixmap->width*pixmap->height);

	xbitmap=XCreateImage(wid->display,DefaultVisualOfScreen(wid->screen),1,
		ZPixmap,0,NULL,pixmap->width,pixmap->height,bpl*8,bpl*pixmap->width);

	xbitmap->data=(char *)malloc(bpl*pixmap->width*pixmap->height);

	for(i=0;i!=ncolors;i++) {
		
		if(z==2) {
		
			sscanf(xpm[i+1],"%s %c %s",sindx,(char *)&x,name);
			indx=sindx[0]<<8|sindx[1];
		
		} else {
		
			sscanf(xpm[i+1],"%c %c %s",sindx,(char *)&x,name);
			indx=sindx[0];
		}
		color[indx]=XSNamedColor(wid,name);
	}

	for(y=0;y!=pixmap->height;y++) {
	
		line=xpm[y+ncolors+1];
	
		for(x=0;x!=pixmap->width;x++) {

			if(z==1) indx=line[x];
			if(z==2) indx=line[x+x]*256|line[x+x+1];

			if(color[indx]==-1)
			
				XPutPixel(xbitmap,x,y,False);
			else 
				XPutPixel(xbitmap,x,y,True);

			XPutPixel(xpixmap,x,y,color[indx]);
		}
	}

	pixmap->mask=XCreatePixmap(wid->display,wid->window,pixmap->width,pixmap->height,1);
	pixmap->gc=XCreateGC(wid->display,pixmap->mask,0,&wid->values);
	XSetLineAttributes(wid->display,pixmap->gc,0,LineSolid,CapRound,JoinRound);
	XPutImage(wid->display,pixmap->mask,pixmap->gc,xbitmap,0,0,0,0,pixmap->width,pixmap->height);

	XFreeGC(wid->display,pixmap->gc);

	pixmap->pixmap=XCreatePixmap(wid->display,wid->window,pixmap->width,pixmap->height,DefaultDepthOfScreen(wid->screen));
	pixmap->gc=XCreateGC(wid->display,pixmap->pixmap,0,&wid->values);
	XSetLineAttributes(wid->display,pixmap->gc,0,LineSolid,CapRound,JoinRound);
	XPutImage(wid->display,pixmap->pixmap,pixmap->gc,xpixmap,0,0,0,0,pixmap->width,pixmap->height);

	XDestroyImage(xpixmap);
	XDestroyImage(xbitmap);

	return(pixmap);
}

XSPixmap *XSPixmapCreateXPMF(XSWidget *wid,char *name) {

	FILE *f,*p;
	char buffer[65536],*line[65536];
	int i,j,k;
	XSPixmap *pixmap=NULL;

	if(strstr(name,".xpm.gz")) {

		sprintf(buffer,"zcat '%s'",name);
		p=popen(buffer,"r");
		f=p;

	} else {
	
		p=NULL;
		f=fopen(name,"r");

	}

	if(!f) return pixmap;

	for(i=0;fgets(buffer,65536,f)&&i!=65536;) {

		if(buffer[0]=='\"') {
		
			for(k=strlen(buffer),j=0;j!=k;j++) 
				if(buffer[j]=='\"') 
					buffer[j]='\0';

			line[i]=(char *)malloc(k);
			strcpy(line[i],buffer+1);
			i++;
		}
	}

	if(p) 	pclose(f);
	else	fclose(f);
	
	pixmap=XSPixmapCreateXPM(wid,line);

	for(j=0;j!=i;j++) if(line[j]) free(line[j]);

	pixmap->name=name;

	return(pixmap);
}
