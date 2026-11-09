#include <stdio.h>
#include <xstep.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <X11/extensions/Xvlib.h>

#include <stdlib.h>
#include <stdio.h>

Atom 		encodingAtom;
XvAdaptorInfo 	*adaptors;
XvEncodingInfo 	*encodings;
XSWidget 	*desktop,
		*pop,
		*xv;

struct XVTree {

	char 	*sdevice,
		*schannel;

	int 	device,
		channel,
		width,
		height;
		
	struct XVTree *next;

} *xvtree,*xvaux;

int active,device,channel,width=800,height=600,pchannel,lchannel;

char **channels;

void xvChange(XSWidget *wid) {

	int a;

	if(pchannel==lchannel) return;

	for(a=0,xvaux=xvtree;xvaux;xvaux=xvaux->next,a++)
	
		if(a==pchannel) {
	
			wid=xv;
	
			if(active) XvStopVideo(wid->display,device,wid->window);
	
			device=xvaux->device;
			channel=xvaux->channel;
	
			XvSetPortAttribute(wid->display,device,encodingAtom,channel);
			XvPutVideo(wid->display,device,wid->window,wid->gc,
				0,0,width,height,
				0,0,width,height);

			active=1;
	
			lchannel=pchannel;
		}
}

void xvCreate(XSWidget *wid) {

	wid->bgcolor=wid->black;

	XSWidgetCreate(wid);

//	XvSetPortAttribute(wid->display,device,encodingAtom,channel);
//	XvPutVideo(wid->display,device,wid->window,wid->gc,
//		0,0,width,height,
//		0,0,width,height);
}

void xvGrab(XSWidget *wid) {

	XImage 		*pix;
	unsigned short 	*wptr;
	unsigned char	*iptr,data[width*height*3];
	int 		i,j;
	FILE 		*fp;
	char 		filename[256];

	XvStopVideo(xv->display,device,xv->window);

	pix=XGetImage(xv->display,xv->window,0,0,640,480,0xffffff,ZPixmap);

	XvPutVideo(xv->display,device,xv->window,xv->gc,
		0,0,width,height,
		0,0,width,height);

	wptr=(unsigned short *)pix->data;
	iptr=data;

	for(j=i=640*480;i--;wptr++) {
	
		*iptr++ = *wptr>>8;
		*iptr++ = (*wptr&0x7e0)>>3;
		*iptr++ = *wptr<<3;
	}

	sprintf(filename,"video-%ld.ppm",time(0));

	fp=fopen(filename,"w");
	
	fprintf(fp,"P6\n%d %d\n%d\n",640,480,255);	

	fwrite(data,j,3,fp);

	fprintf(stderr,"grabbed image RGB=(%x,%x,%x): %s\n",
		XSNamedColor(xv,"red"),
		XSNamedColor(xv,"green"),
		XSNamedColor(xv,"blue"),
		filename);

	fclose(fp);
}

int main(int argc,char **argv) {

	int version, release, a, b, c, d, n;
	
	XvAttribute 	*at;

	desktop=XSDesktop(":0",argv,argc);

	if(XvQueryExtension(desktop->display,&version,&release,&a,&b,&c)!=Success)
		exit(fprintf(stderr,"X Video Extension not available here!\n"));
 
	fprintf(stderr,"detected xVideo Extension V%dR%d:\n",version,release);

	XvQueryAdaptors(desktop->display,desktop->window,&n,&adaptors);

	for(d=a=0;a!=n;a++) {

		fprintf(stderr,"  device %ld: %s type={ %s%s%s%s%s}, ports=%ld\n",
			adaptors[a].base_id,
			adaptors[a].name,
			adaptors[a].type&XvInputMask ?"Input " :"",
			adaptors[a].type&XvOutputMask?"Output ":"",
			adaptors[a].type&XvVideoMask ?"Video " :"",
			adaptors[a].type&XvStillMask ?"Still " :"",
			adaptors[a].type&XvImageMask ?"Image " :"",
			adaptors[a].num_ports);		
	
		if(!(adaptors[a].type&XvVideoMask)) continue;
	
		XvQueryEncodings(desktop->display, adaptors[a].base_id, &c, &encodings);
		
		for(b=0;b!=c;b++,d++) {

			fprintf(stderr,"    channel %ld: %s (%ldx%ld)\n",
				encodings[b].encoding_id,
				encodings[b].name,
				encodings[b].width,
				encodings[b].height);
		
			if(b==0) {
			
				channel=encodings[b].encoding_id;
				device=adaptors[a].base_id;
			}
		
			xvaux		=(struct XVTree *)malloc(sizeof(struct XVTree));
			
			xvaux->sdevice	=strdup(adaptors[a].name);
			xvaux->schannel	=strdup(encodings[b].name);
			xvaux->device	=adaptors[a].base_id;
			xvaux->channel	=encodings[b].encoding_id;
			xvaux->width	=encodings[b].width;
			xvaux->height	=encodings[b].height;
			xvaux->next	=xvtree;
			
			xvtree=xvaux;
		}

		at=XvQueryPortAttributes(desktop->display,adaptors[a].base_id,&c);

		for(b=0;b!=c;b++) {
	
			fprintf(stderr,"    attribute %s: min/max=(%d/%d), %s%s\n",
				at[b].name,
				at[b].min_value,
				at[b].max_value,
				(at[b].flags & XvGettable) ? "R" : "",
				(at[b].flags & XvSettable) ? "W" : "");
		}
	}

	encodingAtom = XInternAtom(desktop->display, "XV_ENCODING", False);

	channels=(char **)malloc(sizeof(char *)*(d+1));

	for(a=0,xvaux=xvtree;xvaux;xvaux=xvaux->next,a++) 
		channels[a]=xvaux->schannel;

	channels[a]=NULL;

	fprintf(stderr,"xVideo ready...starting graphic sub-system...\n");

	xv=XSWindow(desktop,0,0,width,height+40,"XSTEPVideo!");
	xv->on.create=xvCreate;

	XSButton(xv,-8  ,-8,72,24,"Quit",XSWindowClose);
	XSButton(xv,-88 ,-8,72,24,"Grab",xvGrab);

	XSPopup(xv,8,-8,160,21,channels,&pchannel);
	XSButton(xv,176,-8,72,24,"Change",xvChange);

	while(XSCheckEvent(desktop,!XS_BLOCK)) usleep(1);

	return 0;
}
