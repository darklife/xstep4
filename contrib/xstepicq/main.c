#include	<xstep.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/param.h>

#define OK	0
#define TRUE	1

u_int		makeList	__P((char ***));
extern void	setupShow	__P((XSWidget *)),
		aboutShow	__P((XSWidget *));
static void	BtMessageClick	__P((XSWidget *)),
		BtStarClick	__P((XSWidget *)),
		BtSetupClick	__P((XSWidget *)),
		ListClick	__P((XSWidget *));

XSWidget	*myDesktop,*winMain,*winMainList;
char		***list;
u_int		listRows;

int main(int argc, char **argp, char **envp) {

	XSWidget *wid;

	myDesktop=XSDesktop(getenv("DISPLAY"),argp,argc);
	winMain=XSWindow(myDesktop,0,0,150,300,"XStepICQ");

	list=malloc(4);

	if(!list) {

		perror("malloc");
		return -TRUE;
	}

	listRows=makeList(list);

	wid=XSLabel(winMain,25,4,-4,18,"Online pals",XS_BOX_DOWN);
	wid->fgcolor=wid->white;
	wid->font=wid->helvetica12b;
	wid->bgcolor=wid->darkgray;

	winMainList=XSTextScroll(winMain,4,23,-4,-33,1,listRows,list);
	/* winMainList->on.doubleclick=ListClick; */

	XSButton(winMain,4,-4,60,25,"Message",BtMessageClick);
	XSButton(winMain,64,-4,-64,25,"*",BtStarClick);
	XSButton(winMain,-4,-4,60,25,"Setup",BtSetupClick);

	while(XSCheckEvent(myDesktop,XS_BLOCK));
	return OK;
}

u_int makeList(char ***p) {

	u_int i=4, size=sizeof(unsigned);

	printf("p=%p\nlist=%p\n",p,list);

	p[0]=malloc(size*i);

	p[0][0]="naly";
	p[0][1]="esc2";
	p[0][2]="quicktime";
	p[0][3]="cportela";

	return i;
}

static void BtMessageClick(XSWidget *p) {

	printf("Message to pal #%d.\n",winMainList->viewport->line);
}

static void BtStarClick(XSWidget *p) {

	return aboutShow(myDesktop);
}

static void BtSetupClick(XSWidget *p) {

	return setupShow(myDesktop);
}

static void ListClick(XSWidget *p) {

	printf("asdafsd\n");
}
