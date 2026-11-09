#include	<xstep.h>
#include	<stdio.h>
#include	<sys/param.h>

static void	BtOkClick	__P((XSWidget *));

XSWidget *winAbout;

void aboutShow(XSWidget *p) {

	winAbout=XSWindow(p,0,0,400,400,"About..");

	XSButton(winAbout,-4,-4,50,25,"Ok",BtOkClick);
}

static void BtOkClick(XSWidget *p) {
	return XSWindowClose(p);
}
