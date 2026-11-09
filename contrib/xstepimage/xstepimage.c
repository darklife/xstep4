#include <xstep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <jpeglib.h>

XSImage  *image,*oldimage;
XSPixmap *pixmap,*old;
XSWidget *d1,*w1,*w2,*w3,*w4;

double 	zoom=1.0,igamma=1.0;

int	last,next,count,max=1,inroot,antialiasing=1,sb=False;

int pad(int offset) {

	return offset&3?offset-(offset&3)+4:offset;
}

XSImage *jpgread(FILE *fp) {

	char 	format[80],*data=NULL,*target;
	int 	width=0,height=0,depth=0;

	struct jpeg_decompress_struct 	cinfo;
	struct jpeg_error_mgr 		jerr;

	fseek(fp,0,SEEK_SET);

	fgets(format,80,fp);
		
	if(!strncmp(format+6,"JFIF",4)) {

		fseek(fp,0,SEEK_SET);

		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, fp);
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);
		
		width  = cinfo.output_width;
		height = cinfo.output_height;
		depth  = cinfo.output_components;

		fprintf(stderr,"*read: JFIF %dx%d, depth %d\n",width,height,depth);
		
		data=(char *)malloc(width*height*depth);
		
		for(target=data;cinfo.output_scanline<cinfo.output_height;target+=width*depth)
			jpeg_read_scanlines(&cinfo,(JSAMPARRAY)&target,1);
		
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		
	}

	return data?XSImageCreate(width,height,data,depth==3?XS_IMAGE_RGB24:XS_IMAGE_GRAY):NULL;
}


XSImage *pcxread(FILE *fp) {

	struct {

		unsigned char 	manufacturer 	__attribute__ ((packed));
		unsigned char 	version		__attribute__ ((packed));	
		unsigned char 	encoder		__attribute__ ((packed));
		unsigned char 	depth		__attribute__ ((packed));
		unsigned short 	x		__attribute__ ((packed));
		unsigned short 	y		__attribute__ ((packed));
		unsigned short 	width		__attribute__ ((packed));
		unsigned short 	height		__attribute__ ((packed));
		unsigned short 	hdpi		__attribute__ ((packed));
		unsigned short 	vdpi		__attribute__ ((packed));
		unsigned char 	palette[48]	__attribute__ ((packed));
		unsigned char 	reserved	__attribute__ ((packed));
		unsigned char 	planes		__attribute__ ((packed));
		unsigned short 	bpl		__attribute__ ((packed));
		unsigned short  type		__attribute__ ((packed));
		unsigned char  	pad[58]		__attribute__ ((packed));
	
	} pcxinfo;

	int x,y,z,repeat;
	unsigned char *q,*buffer,data,*raw=NULL;

	struct {
		
		unsigned char r,g,b;
	
	} palette[256];

	fseek(fp,0,SEEK_SET);

	fread(&pcxinfo,sizeof(pcxinfo),1,fp);
	
	fprintf(stderr,"check: PCX-v%d-%s-%d, %dx%d (%d bits, status=%s)\n",
		pcxinfo.version,
		pcxinfo.encoder?(pcxinfo.encoder==1?"RLE":"compressed"):"uncompressed",
		pcxinfo.encoder,
		pcxinfo.width,
		pcxinfo.height,
		pcxinfo.depth,
		pcxinfo.bpl == pad(pcxinfo.width*pcxinfo.depth/8)?"valid":"invalid");

	if(pcxinfo.bpl != pad(pcxinfo.width*pcxinfo.depth/8)) return NULL;
	if(pcxinfo.encoder>1) return NULL;
	if(pcxinfo.depth!=8)  return NULL;

	q=raw=(unsigned char *)malloc(pcxinfo.width*pcxinfo.height*3);

	fseek(fp,-768,SEEK_END);

	fread(&palette,sizeof(palette),1,fp);

	fseek(fp,128,SEEK_SET);

	buffer=(unsigned char *)malloc(pcxinfo.bpl*(pcxinfo.height+1));

	x=0;
	
	while(x<pcxinfo.bpl*pcxinfo.height) {

		data=fgetc(fp);
		repeat=1;

		if(data>192) {
			
			repeat=data-192;
			data=fgetc(fp);
		}

		while(repeat--) buffer[x++] = data;
	}

	for(z=y=0;y!=pcxinfo.height;y++) {
	
		for(x=0;x!=pcxinfo.width;x++,z++) {

			*q++ = palette[buffer[z]].b;
			*q++ = palette[buffer[z]].g;
			*q++ = palette[buffer[z]].r;
		}
		
		z+=(pcxinfo.bpl-pcxinfo.width);
	}	

	free(buffer);

	fprintf(stderr,"*read: PCX-v%d-RLE, %dx%d (8 bit/pixel, compressor=RLE, status=OK)\n",
		pcxinfo.version,
		pcxinfo.width,
		pcxinfo.height);

	return raw?XSImageCreate(pcxinfo.width,pcxinfo.height,raw,XS_IMAGE_BGR24):NULL;
}

XSImage *ppmread(FILE *fp) {

	unsigned char buffer[4096],*q,*raw=NULL;
	int type=0,x,y,z,max,r,g,b,width,height;
	
	fseek(fp,0,SEEK_SET);
	
	fgets(buffer,4096,fp);
	
	if(buffer[0]=='P') {

		type=buffer[1]-'0';

		while(fgets(buffer,4096,fp)) if(buffer[0]!='#') break;

		switch(type) {
	
			case 1:
			
				sscanf(buffer,"%d %d",&width,&height);
			
				q=raw=(unsigned char *)malloc(width*height*3);
			
				for(y=0;y!=height;y++) {
				
					for(x=0;x!=width;x++) {
				
						fscanf(fp,"%d",&z);
						z=z?0:255;
						
						*q++ = z;
						*q++ = z;
						*q++ = z;
					}
				}
				
				break;
			case 2:
			
				sscanf(buffer,"%d %d",&width,&height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);
			
				q=raw=(unsigned char *)malloc(width*height*3);
			
				for(y=0;y!=height;y++) {
				
					for(x=0;x!=width;x++) {
				
						fscanf(fp,"%d",&z);
						z=z*255/max;
						
						*q++ = z;
						*q++ = z;
						*q++ = z;
					}
				}
				break;
			case 3:
				sscanf(buffer,"%d %d",&width,&height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);

				q=raw=(unsigned char *)malloc(width*height*3);
			
				for(y=0;y!=height;y++) {
				
					for(x=0;x!=width;x++) {
				
						fscanf(fp,"%d %d %d",&r,&g,&b);

						*q++ = b*255/max;
						*q++ = g*255/max;
						*q++ = r*255/max;
					}
				}
				break;
			case 4:
			
				sscanf(buffer,"%d %d",&width,&height);
			
				q=raw=(unsigned char *)malloc(width*height*3);
			
				for(y=0;y!=height;y++) {
				
					fread(buffer,width/8,1,fp);

					for(x=0;x!=width/8;x++) {
					
						for(z=0;z!=8;z++) {
						
							if(buffer[x]&(0x80>>z))
							
								*q++ = 0,   
								*q++ = 0,   
								*q++ = 0;
							else
								*q++ = 255, 
								*q++ = 255, 
								*q++ = 255;
						}
					}
				}
				break;
			case 5:
			
				sscanf(buffer,"%d %d",&width,&height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);			

				q=raw=(unsigned char *)malloc(width*height*3);
			
				for(y=0;y!=height;y++) {
				
					fread(buffer,width,1,fp);

					for(x=0;x!=width;x++) {
							
						z=buffer[x]*255/max;
						
						*q++ = z;
						*q++ = z;
						*q++ = z;
					}
				}
				break;
			case 6:
			
				sscanf(buffer,"%d %d",&width,&height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);

				q=raw=(unsigned char *)malloc(width*height*3);
			
				for(y=0;y!=height;y++) {
				
					fread(buffer,width*3,1,fp);

					for(x=0;x!=width;x++) {
							
						r=buffer[x*3+0]*255/max;
						g=buffer[x*3+1]*255/max;
						b=buffer[x*3+2]*255/max;
						
						*q++ = b;
						*q++ = g;
						*q++ = r;
					}
				}
				break;
			
		}
	}

		
	if(raw)
		fprintf(stderr,"*read: PPM-P%d, %dx%d pixels\n",
			type,width,height);

	return raw?XSImageCreate(width,height,raw,XS_IMAGE_BGR24):NULL;
}

XSImage *bmpread(FILE *fp) {

	struct {
	
		unsigned short 	type		__attribute__ ((packed));
		unsigned int 	filesize	__attribute__ ((packed));
		unsigned short  reserved[2]	__attribute__ ((packed));
		unsigned int	offset		__attribute__ ((packed));
		unsigned int	header		__attribute__ ((packed));	
		unsigned int	width		__attribute__ ((packed));
		unsigned int	height		__attribute__ ((packed));	
		unsigned short	planes		__attribute__ ((packed));
		unsigned short	depth		__attribute__ ((packed));
		unsigned int 	format		__attribute__ ((packed));
		unsigned int	compsize	__attribute__ ((packed));
		unsigned int	hres		__attribute__ ((packed));
		unsigned int 	vres		__attribute__ ((packed));
		unsigned int	colors		__attribute__ ((packed));
		unsigned int	realcolors	__attribute__ ((packed));
		
	} bmpinfo;
	
	unsigned char 	*data,*p,*q,*raw=NULL;
	int  		size,x,y,z,width,height,depth;

	struct {
	
		unsigned char r,g,b,z;
	
	} palette[256];

	fseek(fp,0,SEEK_SET);
		
	fread(&bmpinfo,sizeof(bmpinfo),1,fp);

	fprintf(stderr,"check: BMP-%s, signature is '%u' (espected %u, status=%s)\n",
		bmpinfo.format?"compressed":"uncompressed",
		bmpinfo.type,
		((unsigned short)'B'<<8)+'M',
		(bmpinfo.type!=('B'<<8)+'M')?"failed":"sucess");
	
	if(bmpinfo.type==('B'<<8)+'M') 	return NULL;
	if(bmpinfo.width&(~0xfff))	return NULL;
	if(bmpinfo.height&(~0xfff)) 	return NULL;
	if(bmpinfo.format)		return NULL;
	if(bmpinfo.planes!=1)		return NULL;
	
	fprintf(stderr,"*read: BMP-%s, size=%dx%d, %d bits (offset = %d bytes)\n",
		bmpinfo.format?"compressed":"uncompressed",
		bmpinfo.width,
		bmpinfo.height,
		bmpinfo.depth,
		bmpinfo.offset);


	if(bmpinfo.depth!=24) fread(&palette,(1<<bmpinfo.depth)*4,1,fp);

	size=pad(bmpinfo.width*bmpinfo.depth/8)*bmpinfo.height;
	data=(char *)malloc(size);
	fread(data,size,1,fp);

	if(bmpinfo.filesize != ftell(fp))
		fprintf(stderr,"warning! BMP image truncated: %d bytes expected, %ld bytes read\n",
			bmpinfo.filesize,
			ftell(fp));

	width	=bmpinfo.width;
	height	=bmpinfo.height;
	depth	=bmpinfo.depth;

	p=data;

	raw=(unsigned char *)malloc(width*height*3);

	for(y=bmpinfo.height-1;y!=-1;y--) {

		q=raw+width*3*y;
		
		if(bmpinfo.depth==24) {

			for(x=0;x!=bmpinfo.width*3;x++) {
			
				q[x]=p[x];
			}
			
			p+=pad(bmpinfo.width*3);
		}
		
		if(bmpinfo.depth==8) {
		
			for(x=0;x!=bmpinfo.width;x++) {
			
				*q++=palette[p[x]].r;
				*q++=palette[p[x]].g;
				*q++=palette[p[x]].b;
			}
			
			p+=pad(bmpinfo.width);
		}
		
		if(bmpinfo.depth==4) {
		
			for(x=0;x!=bmpinfo.width;x++) {
			
				*q++=palette[x%2?p[x/2]&0xf:p[x/2]>>4].r;
				*q++=palette[x%2?p[x/2]&0xf:p[x/2]>>4].g;
				*q++=palette[x%2?p[x/2]&0xf:p[x/2]>>4].b;
			}
			
			p+=pad(bmpinfo.width/2);
		}

		if(bmpinfo.depth==1) {
		
			for(x=0;x!=bmpinfo.width/8;x++) {

				for(z=0;z!=8;z++) {

					*q++=(p[x]&(0x80>>z))?255:0;
					*q++=(p[x]&(0x80>>z))?255:0;
					*q++=(p[x]&(0x80>>z))?255:0;
				}
			}
			
			p+=pad(bmpinfo.width/8);
		}
	}
		
	free(data);

	return raw?XSImageCreate(width,height,raw,XS_IMAGE_BGR24):NULL;
}

XSImage *xpmread(FILE *fp) {

	char buffer[4096],*p,*q,index[16],color[16],*raw=NULL;
	unsigned short *s;

	int status=0,width,height,colors,type,palette[65536],x=0,y=0;

	fseek(fp,0,SEEK_SET);

	while(fgets(buffer,4096,fp)) {

		if(status&&status!=4) {
	
			p=strchr(buffer,'\"');
			
			if(!p) 	continue;
			else 	p++;
			
			q=strchr(p,'\"');
			*q=0;
							
			switch(status) {
			
				case 1:
					sscanf(p,"%d %d %d %d",
						&width,&height,&colors,&type);
					
					raw=(unsigned char *)malloc(width*height*3);
					
					status=2;
					break;
				case 2:
					sscanf(p,"%s c %s",index,color);
					
					if(type==2)
						palette[*((unsigned short *)index)]=strtol(color+1,NULL,16);
					if(type==1)
						palette[*((unsigned char *)index)]=strtol(color+1,NULL,16);
					x++;	
										
					if(x==colors) status=3;
					break;

				case 3:
				
					q=raw+width*y*3;
					s=(unsigned short *)p;
				
					for(x=0;x!=width;x++) {
					
						
					
						if(type==2) {
						
							*q++ = palette[*s]&0xff;
							*q++ = (palette[*s]>>8)&0xff;
							*q++ = palette[*s]>>16;
						
							s++;
						}
						
						if(type==1) {
						
							*q++ = palette[(unsigned)(*p)]&0xff;
							*q++ = (palette[(unsigned)(*p)]>>8)&0xff;
							*q++ = palette[(unsigned)(*p)]>>16;
							
							p++;
						}
					}
					
					y++;
					
					if(y==height) {
					
						status=4;

						fprintf(stderr,"*read: XPM-%d: %dx%d pixels, %d colors\n",
							type,width,height,colors);
					}

					break;
			}
		
		} else 
			if(strstr(buffer,"char")) 
				status=1;
	}

	if(status!=4) {

		fprintf(stderr,"check: XPM-%d: %dx%d pixels, %d colors (failed in stage %d)\n",
			type,width,height,colors,status);
		
		free(raw);
		
		return NULL;
	}

	return raw?XSImageCreate(width,height,raw,XS_IMAGE_BGR24):NULL;
}

XSImage *gifread(FILE *fp) {

	XSImage *aux=NULL;
	char     cmd[1024];
	FILE 	*fpx;
	
	sprintf(cmd,"giftopnm %s",d1->global->argv[count]);
	
	fpx=popen(cmd,"r");

	aux=ppmread(fpx);
	
	pclose(fpx);

	return aux;	
}

void XSImageGamma(XSImage *image,double gammavalue) {

        unsigned i,j;

        gammavalue=1.0/gammavalue;

        for(i=0;i!=256;i++) 
                for(j=0;j!=image->channels;j++) {

                        image->filter[j][i]=(unsigned char)(pow((double)image->filter[j][i]/255.0,gammavalue)*255.0);
                }
}

XSImage *readimage(char *name) {

	XSImage *aux=NULL;
	FILE 	*fp;
	
	fp=fopen(name,"r");
	
	if(fp) {

		if(!aux) aux=jpgread(fp);
		if(!aux) aux=gifread(fp);
		if(!aux) aux=bmpread(fp);
		if(!aux) aux=ppmread(fp);
		if(!aux) aux=xpmread(fp);
		if(!aux) aux=pcxread(fp);

		if(aux) aux->name = name;
		else	fprintf(stderr,"FAILED TO LOAD IMAGE %s\n",name);

		fclose(fp);
	}

	return aux;
}


void showimage(void) {

	int w,h;

	static int oldcount;
	
        if(count<1) count=1;
        if(count>=d1->global->argc) count=d1->global->argc-1;

	if(oldcount!=count) {

		oldimage=image;

		if((image=readimage(d1->global->argv[count]))) {

			XStoreName(d1->display,w2->window,image->name);

			fprintf(stderr,
				"sucess: %dx%d pixels read from %s (%s)\n",
				image->width,
				image->height,
				image->name,
				image->format==XS_IMAGE_RGB24?"RGB24":(image->format==XS_IMAGE_BGR24?"BGR24":"GRAY"));

			if(oldimage) {
			
				XSImageDestroy(oldimage);
				oldimage=NULL;
			}
	
		} else {
		
			XStoreName(d1->display,w2->window,"*** failed to open ***");
		
			fprintf(stderr,"%s: failed to open\n",d1->global->argv[count]);
			image=oldimage;
		}
	}

	if(max==1) {
		
		if(!inroot) {
		
			if(w1->parent->width*1.0/image->width < w1->parent->height*1.0/image->height)
			
				zoom=w1->parent->width*1.0/image->width;
			else
				zoom=w1->parent->height*1.0/image->height;
		}
	}

	if(max==2) {
		
		if(!inroot) {
		
				zoom=w1->parent->width*1.0/image->width;
		}
	}

	if(image) {
		
		if(inroot&&max) {
		
			w=d1->width;
			h=d1->height;
	
		} else {
		
			w=image->width*zoom;
			h=image->height*zoom;
		}

		old=pixmap; 
		pixmap=XSPixmapCreate(inroot?d1:w1,w,h,NULL,XS_PIXMAP_XSHM);

		if(igamma!=1.0) XSImageGamma(image,igamma);

		if(antialiasing) 
			XSImage2PixmapAA(w1,image,pixmap,True);
		else 
			XSImage2Pixmap(w1,image,pixmap,True);

		if(inroot) {
		
			XSetWindowBackgroundPixmap(w1->display,w1->desktop->window,pixmap->pixmap);
			XClearWindow(w1->display,w1->desktop->window);
		}

		XSetWindowBackgroundPixmap(w1->display,w1->window,pixmap->pixmap);

		w1->geometry.w=w1->width =w;
		w1->geometry.h=w1->height=h;
		
		if(sb) XSScrollCheck(w3);
		else {
		
			w1->event->xconfigure.x=(w1->parent->width -w)/2;
			w1->event->xconfigure.y=(w1->parent->height-h)/2;
			w1->event->xconfigure.width =w;
			w1->event->xconfigure.height=h; 		

			XMoveResizeWindow(w1->display,w1->window,
				(w1->parent->width -w)/2,(w1->parent->height-h)/2,
				w,h);
		}

		XClearWindow(w1->display,w1->window);
	}

	if(old) {
		
		XSPixmapDestroy(w1,old);
		old=NULL;
	}

	oldcount=count;
}

void foo(XSWidget *wid) {

        int key=XLookupKeysym(&wid->event->xkey,0);

//	printf("debug: keypressed=%d\n",key);

        switch(key) {

		case XK_Home:
			count=1;
			break;
			
		case XK_End:
			count=d1->global->argc-1;
			break;

		case XK_Page_Up:
			count-=10;
			break;
			
		case XK_Page_Down:
			count+=10;
			break;

		case XK_Down:
			count++;
			break;
		
		case XK_Up:
			count--;
			break;
			
                case XK_Escape:
                	XSWindowClose(w1);
                	return;
                	
                case '=':
                	zoom+=0.1;
                	break;
                case '-':
                	zoom-=0.1;
                	break;
                case 'm':
                	max=1;
                	break;
                case 'a':
                	antialiasing=!antialiasing;
                	break;
                case 'l':
                	max=2;
                	break;
		case 'n':
		case '1':
			max=0;
			zoom=1.0;
			break;
		case '2':
			max=0;
			zoom=2.0;
			break;
		case '3':
			max=0;
			zoom=3.0;
			break;
		case '4':
			max=0;
			zoom=4.0;
			break;
		case '5':
			max=0;
			zoom=0.5;
			break;
		case '6':
			max=0;
			zoom=1.5;
			break;
		case '7':
			max=0;
			zoom=0.75;
			break;
		default: 
			return;
        }

	if(zoom<0.1)  zoom=0.1;
	if(zoom>10.0) zoom=10.0;

	showimage();
}

void bar(XSWidget *wid) {

	XSWidgetCreate(wid);
	wid->on.create=NULL;

	showimage();
}

int main(int argc,char **argv) {

	int i,width=800,height=800;
	
	char *displayname;

	for(i=1;argv[i];i++) {
	
		if(!strcmp(argv[i],"-display"))  	displayname=argv[i+1];
		if(!strcmp(argv[i],"-root"))   		inroot=True;
		if(!strcmp(argv[i],"-antialiasing"))    antialiasing=True;
		if(!strcmp(argv[i],"-sb"))   		sb=True;
		if(!strcmp(argv[i],"-max"))   		max=True;
		if(!strcmp(argv[i],"-gamma"))    {

			if(argv[i+1]) igamma=atol(argv[i+1]);
		}
		if(!strcmp(argv[i],"-geometry")) {

			if(argv[i+1]) sscanf(argv[i+1],"%dx%d",&width,&height);
		}
	}
	
	d1=XSDesktop(getenv("DISPLAY"),argv,argc);

	w2=XSWindow(d1,0,0,width,height,argv[0]);
	w2->on.event[KeyPress]=foo;

	if(sb) {
	
		w3=XSScroll(w2,0,0,0,0,600,600);
		w1=w3->viewport;

	} else {
	
		w1=XSWindow(w2,0,0,0,0,"");
	}

	w1->on.create=bar;	
		w2->bgcolor=d1->black;

	while(XSCheckEvent(d1,XS_BLOCK));
	
	return 0;
}

