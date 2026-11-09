#include <stdio.h>
#include <stdlib.h>
#include <mc68901.h>
#include <misc.h>

void print68901(mc68901 **mfp,int count) {

	int j;
	
	fprintf(stderr,"\n");
	
	for(j=0;j!=count;j++) {
	
		fprintf(stderr,"mfp #%d MC68901/base=0x%08x (bandwidth ~%f Kbps):\n\n",
			j,
			mfp[j]->base,
			mfp[j]->count*8/1e3);
	
		fprintf(stderr," PAR%d: 0x%02x (%s)\n",
			j,
			mfp[j]->device->parallel.port.data,
			printbit((unsigned)mfp[j]->device->parallel.port.data,8));

		fprintf(stderr," SER%d: 0x%02x (%s)\n",
			j,
			mfp[j]->device->serial.port.data,
			printbit((unsigned)mfp[j]->device->serial.port.data,8));

		fprintf(stderr,"\n");
	
		mfp[j]->count=0;
	}
}

mc68901 *create68901(unsigned char *space,int base) {

	mc68901 *mfp;

	mfp=(mc68901 *)malloc(sizeof(mc68901));

	mfp->device=(void *)space+base;
	mfp->base=base;

	return mfp;
}
