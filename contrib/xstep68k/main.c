#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include <xstep.h>
#include <mc68000.h>
#include <mc68901.h>
#include <misc.h>

#define CPU 		2
#define MFP 		2

mc68000 *cpu[CPU];
mc68901 *mfp[MFP];

XSWidget *d0,*w0;

void status(int sig) {

	sig=alarm(1);

	fprintf(stderr,"\33[2J\33[H");

	cpu[0]->dump(cpu[0]);
	cpu[1]->dump(cpu[1]);

	
}

int main(int argc,char **argv) {

	unsigned char *space;
	unsigned short *code;
	int i;

	d0=XSDesktop(getenv("DISPLAY"),argv,argc);

	w0=XSWindow(d0,0,0,640,480,"xstep68k");

	   XSButton(w0,-8,-8,72,24,"Quit",XSWindowClose);
	
	space=(unsigned char *)malloc(ADDRMASK+1);

	code=(unsigned short *)space;

	for(i=8;i<1024;i++) {
	
		code[i]=0x3000;
	}

	cpu[0]=create68000(space);
	cpu[1]=create68000(space);

	cpu[0]->halt=0;
	cpu[1]->halt=1;

	signal(SIGALRM,status);

	// alarm(1);
	
	// while(XSCheckEvent(d0,!XS_BLOCK)) {

	while(1) {

		fprintf(stderr,"\33[H");

		if(!cpu[0]->halt) {
		
			cpu[0]->exec(cpu[0]);
			cpu[0]->dump(cpu[0]);

		}
		
	}

	return 0;
}
