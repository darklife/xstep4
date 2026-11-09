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

#ifndef XSTEP
#define XSTEP

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#ifdef __cplusplus

	extern "C" {
#endif

#define	XS_EXPOSECACHESIZE 	32

#define XS_NONE			0
#define XS_TRUE			1
#define XS_FALSE		0

#define XS_TEXT_CENTER		0
#define XS_TEXT_MIDDLE		0
#define XS_TEXT_LEFT		1
#define	XS_TEXT_RIGHT		2
#define XS_TEXT_UP		4
#define XS_TEXT_DOWN		8
#define XS_TEXT_CURSOR		16

#define XS_BOX_UP		32
#define XS_BOX_DOWN		64
#define XS_BOX_UPDOWN		128
#define XS_BOX_HIDEONBORDER	256
#define XS_BOX_NONE		XS_BOX_HIDEONNBORDER

#define XS_SINGLELINE		512

#define XS_INVISIBLE		-1
#define XS_PIXMAP		-2
#define XS_COPYFROMPARENT	-3

#define XS_NOBLOCK		1
#define XS_BLOCK		0

#define XS_PIXMAP_DEFAULT	0
#define XS_PIXMAP_XSHM		1

#define XS_IMAGE_RGB24		1
#define XS_IMAGE_BGR24		2
#define XS_IMAGE_RGB32		3
#define XS_IMAGE_GRAY		4
#define XS_IMAGE_CMYK		5

#define XS_ABSOLUT		1
#define XS_DELTA		2

#ifndef NULL
#define NULL ((void *)0L)
#endif

typedef struct XSGlobal {

        Display         *display;
        Screen          *screen;

        Cursor          cursor;
        Atom            atom;
        XWMHints        *hints;
        XFontStruct     *helvetica12m,
                        *helvetica12b,
                        *helvetica24b,
                        *courier12m;

	int		argc,
			count,
			animate,
			shift,
			capslock;
	char		**argv,
			*name;

	XEvent		*event;

} XSGlobal;

typedef struct XSGeometry {

	int 			x,y,w,h;

} XSGeometry;

typedef struct XSPixmap {

	char 			*name;
	int			width,height,
				type;
	Pixmap			pixmap,mask;
	GC			gc;
	XShmSegmentInfo		xshm;
	XImage			*image;
	struct XSPixmap		*next,*last;

} XSPixmap;

typedef struct XSImage {

	char 			*name;
	
	unsigned char 		*data,
				**filter;

	int			width,
				height,
				channels,
				format;

	struct XSImage		*next,
				*last;

} XSImage;

typedef struct XSText {

	int			x,y,
				width,
				height,
				lines,
				control,
				textchecksum,
				textsize,
				textcurrent,	/* bug fix */
				textstart,
				textend;
	char 			*text,
				singleline;
	XFontStruct		*font;
	struct XSText		*next,*last;

} XSText;

typedef struct XSWidget {

	char			*name,
				*data,
				**list,
				***matrix;
	int			bgcolor,
				fgcolor,
				eventmask,
				width,
				height,
				type,
				status,
				control,
				valuemask,
				attributemask,
				black, 
				white, 
				gray,
				darkgray,
				invisible,
				visible,
				exposec,
				depth,
				*idata,
				size,
				checksum,
				rows,
				cols,
				*pointer,
				page,
				line,
				min,
				max,
				index,
				tune;
	unsigned char		colormap[64];
	Window			window;
	Display			*display;
	Screen			*screen;
	XSetWindowAttributes	attributes;
	GC			gc;
	Cursor          	cursor;
	XGCValues		values;
	XEvent			*event;
	XRectangle      	clipmask[XS_EXPOSECACHESIZE];
	XFontStruct		*font,
        			*helvetica12m,
                        	*helvetica12b,
                        	*helvetica24b,
                        	*courier12m;
	XClassHint		classhint;
	XSizeHints		*sizehints;
	XSGeometry		geometry;
	XSPixmap		*pixmap;
	XSText			*text;

	struct {

		void		(*event[LASTEvent]) 	(struct XSWidget *);
		void		(*click)		(struct XSWidget *);
		void		(*doubleclick)		(struct XSWidget *);
		void		(*keypress)		(struct XSWidget *);
		void		(*focusin)		(struct XSWidget *);	
		void		(*focusout)		(struct XSWidget *);
		void		(*change)		(struct XSWidget *);
		void		(*resize)		(struct XSWidget *);
		void		(*create)		(struct XSWidget *);
		void		(*destroy)		(struct XSWidget *);
	} on;

	struct {
	
		XSPixmap	*check_on,
				*check_off,
				*radio_on,
				*radio_off,
				*bar_point,
				*bar_bg,
				*bar_up,
				*bar_down,
				*bar_left,
				*bar_right;
	
	} pix;

	struct XSWidget		*parent,
				*child,
				*next,
				*last,
				*desktop,
				*group,
				*viewport;
	struct XSGlobal		*global;

} XSWidget;


void 		XSDrawBox	(XSWidget *,int,int,int,int,int,int);
void		XSConfigure	(XSWidget *);
int 		XSColor		(XSWidget *,int,int,int);
int		XSNamedColor	(XSWidget *,char *);
int		XSCheckEvent	(XSWidget *,int);
int		XSCheckBox	(XSWidget *,int,int,int,int);
XSWidget *	XSWidgetCopy	(XSWidget *,int,int,int,int,char *,int);
XSWidget *	XSWindow	(XSWidget *,int,int,int,int,char *);
XSWidget *	XSMenu		(XSWidget *,int,int,int,int,char **,int,int *);
XSWidget *	XSDialog	(XSWidget *,XSWidget *,int,int,char *);
XSWidget *	XSButton	(XSWidget *,int,int,int,int,char *,void (*f)(XSWidget *));
XSWidget *	XSXSWidget	(XSWidget *,int,int,int,int,char *);
XSWidget *	XSLabel		(XSWidget *,int,int,int,int,char *,int);
XSWidget *	XSCheck		(XSWidget *,int,int,int,int,char *,int *,int);
XSWidget *	XSRadio		(XSWidget *,int,int,int,int,char *,int *,int);
XSWidget *	XSScroll	(XSWidget *,int,int,int,int,int,int);
XSWidget *	XSTextScroll 	(XSWidget *,int,int,int,int,int,int,char ***);
XSWidget *	XSPopup		(XSWidget *,int,int,int,int,char **,int *);
XSWidget *	XSEdit		(XSWidget *,int,int,int,int,char *,int,int);
XSWidget *	XSTune		(XSWidget *,int,int,int,int,int *);

void		XSWindowClose	(XSWidget *);
void		XSWindowDelete	(XSWidget *);
void 		XSWidgetCreate	(XSWidget *);
XSText   *	XSCreateText	(XSWidget *,char *,int);
int 		XSDrawText	(XSWidget *,XSText *);
void 		XSWrapText	(XSWidget *,char *,int);
void 		XSTreeDelete	(XSWidget *);

int		XSCheckSum	(char *);
XSWidget * 	XSDesktop	(char *,char **,int);

XSImage	 *	XSImageCreate	(int,int,void *,int);
XSPixmap *      XSPixmapCreate	(XSWidget *,int,int,void *,int);
void		XSPixmapDestroy (XSWidget *,XSPixmap *);
void		XSPixmapDraw	(XSWidget *,int,int,XSPixmap *);
void		XSPixmapUpdate	(XSWidget *,XSPixmap *);
void		XSImage2Pixmap	(XSWidget *,XSImage *,XSPixmap *,int);
void		XSImageGamma	(XSImage *,double);
void		XSImageClear	(XSImage *);
void		XSImageInverse	(XSImage *);

XSPixmap * 	XSPixmapCreateXPM	(XSWidget *,char **);
XSPixmap * 	XSPixmapCreateXPMF	(XSWidget *,char *);

XSText	 *	XSCreateText	(XSWidget *,char *,int);
XSText	 * 	XSDestroyText	(XSText *);

void		XSMode(int mode);

#ifdef __cplusplus

	}
#endif
#endif
