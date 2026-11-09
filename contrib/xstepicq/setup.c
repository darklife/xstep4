#include	<xstep.h>
#include	<stdio.h>
#include	<sys/param.h>

void	BtOkClick	__P((XSWidget *));

XSWidget	*winSetup;
char		nick[21]="esc2",passwd[21]="********",server[51]="icq.icq.com",port[5]="5190";

void setupShow(XSWidget *p) {

	winSetup=XSWindow(p,0,0,400,140,"Setup");

	XSLabel(winSetup,4,8,92,22,"Nick",XS_NONE);
	XSEdit(winSetup,100,8,180,22,nick,sizeof(nick)-1,XS_TEXT_LEFT|XS_SINGLELINE);

	XSLabel(winSetup,4,32,92,22,"Passwd",XS_NONE);
	XSEdit(winSetup,100,32,180,22,passwd,sizeof(passwd)-1,XS_TEXT_LEFT|XS_SINGLELINE);

	XSLabel(winSetup,4,56,92,22,"Server",XS_NONE);
	XSEdit(winSetup,100,56,-4,22,server,sizeof(server)-1,XS_TEXT_LEFT|XS_SINGLELINE);

	XSLabel(winSetup,4,80,92,22,"Port",XS_NONE);
	XSEdit(winSetup,100,80,180,22,port,sizeof(port)-1,XS_TEXT_LEFT|XS_SINGLELINE);

	XSButton(winSetup,-4,-4,60,25,"Ok",BtOkClick);
}

void BtOkClick(XSWidget *p) {

	printf("%s\n%s\n%s\n%s\n",nick,passwd,server,port);

	return XSWindowClose(winSetup);
}
