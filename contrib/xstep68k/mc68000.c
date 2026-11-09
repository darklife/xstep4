#include <stdio.h>
#include <stdlib.h>
#include <mc68000.h>
#include <misc.h>

void print68000(mc68000 **cpu,int count) {

	int i,j;

	fprintf(stderr,"[2J[H");

	for(j=0;j!=count;j++) {

		fprintf(stderr,"cpu #%d MC68000/%dMB (%.3f MHz/%.3f MIPS): %s",
			j,
			(ADDRMASK+1)/1048576,
			cpu[j]->clock/1e6,
			cpu[j]->count/1e6,
			cpu[j]->halt?"halted.\n":"running!\n\n");

		cpu[j]->clock=0;
		cpu[j]->count=0;

		if(!cpu[j]->halt) {
	
			for(i=0;i!=8;i++)
				fprintf(stderr," D%d: 0x%08x (%s)\n",
					i,
					cpu[j]->d[i].ul,
					printbit(cpu[j]->d[i].ul,32));

			for(i=0;i!=7;i++)
				fprintf(stderr," A%d: 0x%08x (%s)\n",
					i,
					cpu[j]->a[i].ul,
					printbit(cpu[j]->a[i].ul,32));
	
			fprintf(stderr,"%s: 0x%08x (%s)\n",
				cpu[j]->status.flag.s?"SSP":"USP",
				cpu[j]->a[i].ul,
				printbit(cpu[j]->a[i].ul,32));

			fprintf(stderr,"%s: 0x%08x (%s)\n\n",
				cpu[j]->status.flag.s?"USP":"SSP",
				cpu[j]->sp.ul,
				printbit(cpu[j]->sp.ul,32));
	
			fprintf(stderr," PC: 0x%08x (%s)\n",
				cpu[j]->pc,
				printbit(cpu[j]->pc,32));

			fprintf(stderr," SR:     0x%04x %c-%c--%03x---%c%c%c%c%c (PC): %s\n\n",
				cpu[j]->status.sr,
				cpu[j]->status.flag.t?'T':'-',
				cpu[j]->status.flag.s?'S':'-',
				cpu[j]->status.flag.imask,
				cpu[j]->status.flag.x?'X':'-',
				cpu[j]->status.flag.n?'N':'-',
				cpu[j]->status.flag.z?'Z':'-',
				cpu[j]->status.flag.v?'V':'-',
				cpu[j]->status.flag.c?'C':'-',
				cpu[j]->ucode[*(unsigned short *)(cpu[j]->space+(cpu[j]->pc&ADDRMASK))].n);
		}
	}
}

void i68000nop(mc68000 *cpu) {

	cpu->pc+=2;
	cpu->clock+=4;
}

void i68000illegal(mc68000 *cpu) {

	cpu->pc+=2;
	cpu->clock+=4;
}

mc68000 *create68000(unsigned char *space) {

	mc68000 *cpu;
	int i;

	cpu=(mc68000 *)malloc(sizeof(mc68000));

	cpu->space	=space;
	cpu->clock	=0;
	cpu->count	=0;
	cpu->halt	=1;
	cpu->status.sr	=1<<13;
	cpu->a[7].ul	=*(unsigned int *)(cpu->space+0);
	cpu->pc		=*(unsigned int *)(cpu->space+4);

	for(i=0;i!=65535;i++) {
	
		cpu->ucode[i].f=i68000illegal;
		cpu->ucode[i].n="illegal";
	}
	return cpu;
}
