/*************************************************************************

	XSTEP 4.0 - Toolkit for X Window System
	Copyright (C) 1996-2001 Marcelo Samsoniuk,
				Gleicon S. Moraes 
				and others.

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
#include <xstep.h>
#include <math.h>

void XSImageCopy(XSImage *src,XSImage *dst,int operation) {

	int 		size,i;
	unsigned char 	*s,*d;

	s=src->data;
	d=dst->data;

	size=src->width*src->height;

	if(operation)
		while(size--)
			for(i=0;i!=src->channels;i++) 
				*d++=src->filter[i][*s++];
	else
		memcpy(s,d,size*src->channels);
}

void XSImageRGB24Swap(XSImage *image) {

	int 		size,r,g,b;
	unsigned char 	*s,*d;

	s=image->data;
	d=image->data;

	size=image->width*image->height;

	while(size--) {
	
		r = *s++; g = *s++; b = *s++;
		*d++ = b; *d++ = g; *d++ = r;
	}
	
	if(image->format == XS_IMAGE_RGB24) image->format=XS_IMAGE_BGR24;
	else image->format=XS_IMAGE_RGB24;
}
/*
void XSImageGamma(XSImage *image,double gammavalue) {

	unsigned i,j;

	gammavalue=1.0/gammavalue;

	for(i=0;i!=256;i++) 
		for(j=0;j!=image->channels;j++) {
		
			image->filter[j][i]=(unsigned char)(pow((double)image->filter[j][i]/255.0,gammavalue)*255.0);
		}
}
*/
void XSImageInverse(XSImage *image) {

	unsigned i,j;

	for(i=0;i!=256;i++) 
		for(j=0;j!=image->channels;j++)
			image->filter[j][i] = ~image->filter[j][i];
}

void XSImageClear(XSImage *image) {

	unsigned i,j;

	for(i=0;i!=256;i++) 
		for(j=0;j!=image->channels;j++)
			image->filter[j][i] = i;
}

XSImage *XSImageCreate(int width,int height,void *data,int format) {

	XSImage *image=NULL;
	int 	i,j,channels;

	switch(format) {

		case XS_IMAGE_RGB32: channels=4; break;
		case XS_IMAGE_RGB24: channels=3; break;
		case XS_IMAGE_BGR24: channels=3; break;
		case XS_IMAGE_GRAY:  channels=1; break;
		case XS_IMAGE_CMYK:  channels=4; break;
		
		default: return image;
	}

	image=(XSImage *)malloc(sizeof(XSImage));
	memset(image,0,sizeof(XSImage));

	image->format	=format;
	image->channels =channels;
	image->name     =NULL;
	image->width    =width;
	image->height   =height;

	image->filter=(unsigned char **)malloc(sizeof(unsigned char *)*image->channels);
	
	for(i=0;i!=image->channels;i++) {
	
		image->filter[i]=(char *)malloc(256);
	
		for(j=0;j!=256;j++) image->filter[i][j]=j;
	}

	if(!data) 	image->data=malloc(width*height*image->channels);
	else		image->data=data;
	
	return(image);
}

void XSImageDestroy(XSImage *image) {

	int i;
	
	for(i=0;i!=image->channels;i++) free(image->filter[i]);
	
	free(image->filter);
	free(image->data);
	free(image);
}

void XSImage2Pixmap(XSWidget *wid,XSImage *image,XSPixmap *pixmap,int operation) {

	unsigned char	*bpix;
	unsigned short	*wpix;
	unsigned int	*lpix;

	unsigned char 	*bimg;

	int x,y,bpl,i,z,wzoom,*zcache=NULL;

	static unsigned int	pixr[256],
				pixg[256],
				pixb[256],
				buildpixt=True;

	if(buildpixt||operation) {

		for(i=0;i!=256;i++) {
		
			pixr[i]=XSColor(wid,operation?image->filter[0][i]:i,0,0);
			pixg[i]=XSColor(wid,0,operation?image->filter[0][i]:i,0);
			pixb[i]=XSColor(wid,0,0,operation?image->filter[0][i]:i);
		}

		buildpixt = False;
	}

	bpl	=(wid->depth>16)?4:((wid->depth>8)?2:1);
	wzoom	=(image->width!=pixmap->width)?True:False;

#ifdef DEBUG
	fprintf(stderr,"XSImage4Pixmap: %dx%d (bpl=%d, zoom=%s)\n",
		pixmap->width,pixmap->height,bpl,wzoom?"True":"False");
#endif

	if(wzoom) {

		zcache=(int *)malloc(pixmap->width*sizeof(int));

		for(x=0;x!=pixmap->width;x++)
			zcache[x]=((x*image->width)/pixmap->width)*image->channels;
	}

	for(y=0;y!=pixmap->height;y++) {
	
		if(image->height!=pixmap->height)
			bimg = image->data + ((y*image->height)/pixmap->height)*image->width*image->channels;
		else
			bimg = image->data + y*image->width*image->channels;
			
		bpix = pixmap->image->data + pixmap->image->bytes_per_line * y;
		wpix = (unsigned short *)bpix;
		lpix = (unsigned int *)bpix;

		if(image->format==XS_IMAGE_RGB32) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*lpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*wpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*bpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];
			}
		}

		if(image->format==XS_IMAGE_RGB24) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*lpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*wpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*bpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];
			}
		}

		if(image->format==XS_IMAGE_BGR24) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*lpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*wpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*bpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];
			}
		}

		if(image->format==XS_IMAGE_GRAY) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg++)
						*lpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg++)
						*wpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg++)
						*bpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];
			}
		}

	}

	if(zcache) free(zcache);

	XSPixmapUpdate(wid,pixmap);
}

void XSPixmap2Image(XSWidget *wid,XSPixmap *pixmap,XSImage *image,int operation) {

	unsigned char	*bpix;
	unsigned short	*wpix;
	unsigned int	*lpix;

	unsigned char 	*bimg;

	int x,y,bpl,i,z=0,wzoom,*zcache=NULL;

	static unsigned int	pixr[256],
				pixg[256],
				pixb[256],
				buildpixt=True;

	if(buildpixt||operation) {

		for(i=0;i!=256;i++) {
		
			pixr[i]=XSColor(wid,operation?image->filter[0][i]:i,0,0);
			pixg[i]=XSColor(wid,0,operation?image->filter[0][i]:i,0);
			pixb[i]=XSColor(wid,0,0,operation?image->filter[0][i]:i);
		}

		buildpixt = False;
	}

	bpl	=(wid->depth>16)?4:((wid->depth>8)?2:1);
	wzoom	=(image->width!=pixmap->width)?True:False;

#ifdef DEBUG
	fprintf(stderr,"XSImage4Pixmap: %dx%d (bpl=%d, zoom=%s)\n",
		pixmap->width,pixmap->height,bpl,wzoom?"True":"False");
#endif

	if(wzoom) {

		zcache=(int *)malloc(pixmap->width*sizeof(int));

		for(x=0;x!=pixmap->width;x++)
			zcache[x]=((x*image->width)/pixmap->width)*image->channels;
	}

	for(y=0;y!=pixmap->height;y++) {
	
		if(image->height!=pixmap->height)
			bimg = image->data + ((y*image->height)/pixmap->height)*image->width*image->channels;
		else
			bimg = image->data + y*image->width*image->channels;
			
		bpix = pixmap->image->data + pixmap->image->bytes_per_line * y;
		wpix = (unsigned short *)bpix;
		lpix = (unsigned int *)bpix;

		if(image->format==XS_IMAGE_RGB32) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*lpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*wpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*bpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];
			}
		}

		if(image->format==XS_IMAGE_RGB24) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*lpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*wpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*bpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];
			}
		}

		if(image->format==XS_IMAGE_BGR24) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*lpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*wpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*bpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];
			}
		}

		if(image->format==XS_IMAGE_GRAY) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*lpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*wpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=zcache[x])
						*bpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg++)
						*lpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg++)
						*wpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg++)
						*bpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];
			}
		}

	}

	if(zcache) free(zcache);

	XSPixmapUpdate(wid,pixmap);
}

/*
	TEST: will work only  for RGB24 to 16 bpp!
*/

void XSImage2PixmapAA(XSWidget *wid,XSImage *image,XSPixmap *pixmap,int operation) {

	unsigned char	*bpix;
	unsigned short	*wpix;
	unsigned int	*lpix;

	unsigned char 	*aimg,*bimg;

	int x,y,bpl,i,z,w,wzoom,*xcache=NULL,*ycache=NULL;

	int r,g,b,fa,fb,fc,fd,k;

	static unsigned int	pixr[256],
				pixg[256],
				pixb[256],
				buildpixt=True;

	k=image->channels;
	r=g=b=0;

	if(buildpixt||operation) {

		for(i=0;i!=256;i++) {
		
			pixr[i]=XSColor(wid,operation?image->filter[0][i]:i,0,0);
			pixg[i]=XSColor(wid,0,operation?image->filter[0][i]:i,0);
			pixb[i]=XSColor(wid,0,0,operation?image->filter[0][i]:i);
		}

		buildpixt = False;
	}

	bpl	=(wid->depth>16)?4:((wid->depth>8)?2:1);
	wzoom	=(image->width!=pixmap->width)?True:False;

#ifdef DEBUG
	fprintf(stderr,"XSImage4Pixmap: %dx%d (bpl=%d, zoom=%s)\n",
		pixmap->width,pixmap->height,bpl,wzoom?"True":"False");
#endif

	if(wzoom) {

		xcache=(int *)malloc(pixmap->width*sizeof(int));

		for(x=0;x!=pixmap->width;x++)  xcache[x]=(16*x*(image->width-1))/pixmap->width;

		ycache=(int *)malloc(pixmap->height*sizeof(int));

		for(y=0;y!=pixmap->height;y++) ycache[y]=(16*y*(image->height-1))/pixmap->height;
		
	}

	for(w=y=0;y!=pixmap->height;y++,w=wzoom?(ycache[y]>>4):y) {

		bimg = image->data + w*image->width*k;

		if(wzoom) {

			aimg = image->data + (w+1)*image->width*k;
			fd=ycache[y]&15;
			fc=15-fd;
		}
			
		bpix = pixmap->image->data + pixmap->image->bytes_per_line * y;
		wpix = (unsigned short *)bpix;
		lpix = (unsigned int *)bpix;

		if(image->format==XS_IMAGE_RGB32) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*lpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==2) {
				
					for(z=x=0;x!=pixmap->width;x++,z=(xcache[x]>>4)*4) {
				
						fb=xcache[x]&15;
						fa=15-fb;
					
						r = (fa*fc*bimg[z+0]+fb*fc*bimg[z+4]+fa*fd*aimg[z+0]+fb*fd*aimg[z+4])/(15*15);
						g = (fa*fc*bimg[z+1]+fb*fc*bimg[z+5]+fa*fd*aimg[z+1]+fb*fd*aimg[z+5])/(15*15);
						b = (fa*fc*bimg[z+2]+fb*fc*bimg[z+6]+fa*fd*aimg[z+2]+fb*fd*aimg[z+6])/(15*15);
						 
						*wpix++ = pixr[r]+pixg[g]+pixb[b];						
					}
				}

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*bpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*lpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*wpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=4)
						*bpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];
			}
		}

		if(image->format==XS_IMAGE_RGB24) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*lpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];

				if(bpl==2) {
				
					for(z=x=0;x!=pixmap->width;x++,z=(xcache[x]>>4)*3) {
				
						fb=xcache[x]&15;
						fa=15-fb;
					
						r = (fa*fc*bimg[z+0]+fb*fc*bimg[z+3]+fa*fd*aimg[z+0]+fb*fd*aimg[z+3])/(15*15);
						g = (fa*fc*bimg[z+1]+fb*fc*bimg[z+4]+fa*fd*aimg[z+1]+fb*fd*aimg[z+4])/(15*15);
						b = (fa*fc*bimg[z+2]+fb*fc*bimg[z+5]+fa*fd*aimg[z+2]+fb*fd*aimg[z+5])/(15*15);
						 
						*wpix++ = pixr[r]+pixg[g]+pixb[b];						
					}
				}

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*bpix++ = pixr[bimg[z+0]]+pixg[bimg[z+1]]+pixb[bimg[z+2]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*lpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*wpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*bpix++ = pixr[bimg[0]]+pixg[bimg[1]]+pixb[bimg[2]];
			}
		}

		if(image->format==XS_IMAGE_BGR24) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*lpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];

				if(bpl==2) {
				
					for(z=x=0;x!=pixmap->width;x++,z=(xcache[x]>>4)*3) {
				
						fb=xcache[x]&15;
						fa=15-fb;
					
						b = (fa*fc*bimg[z+0]+fb*fc*bimg[z+3]+fa*fd*aimg[z+0]+fb*fd*aimg[z+3])/(15*15);
						g = (fa*fc*bimg[z+1]+fb*fc*bimg[z+4]+fa*fd*aimg[z+1]+fb*fd*aimg[z+4])/(15*15);
						r = (fa*fc*bimg[z+2]+fb*fc*bimg[z+5]+fa*fd*aimg[z+2]+fb*fd*aimg[z+5])/(15*15);
						 
						*wpix++ = pixr[r]+pixg[g]+pixb[b];						
					}
				}

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*bpix++ = pixr[bimg[z+2]]+pixg[bimg[z+1]]+pixb[bimg[z+0]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*lpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*wpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg+=3)
						*bpix++ = pixr[bimg[2]]+pixg[bimg[1]]+pixb[bimg[0]];
			}
		}

		if(image->format==XS_IMAGE_GRAY) {

			if(wzoom) {

				if(bpl==4) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*lpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];

				if(bpl==2) for(z=x=0;x!=pixmap->width;x++,z=(xcache[x]>>4)) {
				
						fb=xcache[x]&15;
						fa=15-fb;
					
						r=(fa*fc*bimg[z+0]+fb*fc*bimg[z+1]+fa*fd*aimg[z+0]+fb*fd*aimg[z+1])/(15*15);
						 
						*wpix++ = pixr[r]+pixg[r]+pixb[r];
					}

				if(bpl==1) for(z=x=0;x!=pixmap->width;x++,z=xcache[x])
						*bpix++ = pixr[bimg[z]]+pixg[bimg[z]]+pixb[bimg[z]];
			} else {

				if(bpl==4) for(x=0;x!=pixmap->width;x++,bimg++)
						*lpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];

				if(bpl==2) for(x=0;x!=pixmap->width;x++,bimg++)
						*wpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];

				if(bpl==1) for(x=0;x!=pixmap->width;x++,bimg++)
						*bpix++ = pixr[*bimg]+pixg[*bimg]+pixb[*bimg];
			}
		}

	}

	if(xcache) free(xcache);
	if(ycache) free(ycache);

	XSPixmapUpdate(wid,pixmap);
}
