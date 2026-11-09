#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include <xstep.h>
#include <mc68000.h>
#include <mc68901.h>
#include <misc.h>

#define CPU 		4
#define MFP 		2
#define CACHE 		64

mc68000 *cpu[CPU];
mc68901 *mfp[MFP];

XSWidget *d0,*w0;

void status(int sig) {

	sig=alarm(1);

	print68000(cpu,CPU);
	print68901(mfp,MFP);

	
}

int main(int argc,char **argv) {

	int i,j,cache;
	unsigned char *space;

	d0=XSDesktop(getenv("DISPLAY"),argv,argc);

	w0=XSWindow(d0,0,0,640,480,"xstep68k");

	   XSButton(w0,-8,-8,72,24,"Quit",XSWindowClose);
	
	space=(unsigned char *)malloc(ADDRMASK+1);

	cpu[0]=create68000(space);
	cpu[1]=create68000(space);
	cpu[2]=create68000(space);
	cpu[3]=create68000(space);

	mfp[0]=create68901(space,0x800000);
	mfp[1]=create68901(space,0x810000);

	cpu[0]->halt=0;
	cpu[1]->halt=1;

	cache=CACHE;

	signal(SIGALRM,status);

	alarm(1);
	
	while(XSCheckEvent(d0,!XS_BLOCK)) {

		for(i=0;i!=CPU;i++) {

			if(!cpu[i]->halt) {

				for(j=0;j!=cache;j++) {

					cpu[i]->ucode[*(unsigned short *)(space+(cpu[i]->pc&ADDRMASK))].f(cpu[i]);
					cpu[i]->ucode[*(unsigned short *)(space+(cpu[i]->pc&ADDRMASK))].f(cpu[i]);
					cpu[i]->ucode[*(unsigned short *)(space+(cpu[i]->pc&ADDRMASK))].f(cpu[i]);
					cpu[i]->ucode[*(unsigned short *)(space+(cpu[i]->pc&ADDRMASK))].f(cpu[i]);
				}

				cpu[i]->count+=(4*cache);	
			}
		}
		
		for(i=0;i!=MFP;i++) {
		
			mfp[i]->count++;
		}
	}

	return 0;
}
