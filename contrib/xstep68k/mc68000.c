#include <stdio.h>
#include <stdlib.h>
#include <mc68000.h>
#include <misc.h>

void dump68000(mc68000 *cpu) {

	int i;

	fprintf(stderr,"cpu MC68000/%dMB (%.3f MHz/%.3f MIPS): %s",
		(ADDRMASK+1)/1048576,
		cpu->clock/1e6,
		cpu->count/1e6,
		cpu->halt?"halted.\n":"running!\n\n");

	cpu->clock=0;
	cpu->count=0;

	if(!cpu->halt) {
	
		for(i=0;i!=8;i++)
			fprintf(stderr," D%d: 0x%08x (%s)\n",
				i,
				cpu->d[i].ul,
				printbit(cpu->d[i].ul,32));

		for(i=0;i!=7;i++)
			fprintf(stderr," A%d: 0x%08x (%s)\n",
				i,
				cpu->a[i].ul,
				printbit(cpu->a[i].ul,32));

		fprintf(stderr,"%s: 0x%08x (%s)\n",
			cpu->status.flag.s?"SSP":"USP",
			cpu->a[i].ul,
			printbit(cpu->a[i].ul,32));

		fprintf(stderr,"%s: 0x%08x (%s)\n\n",
			cpu->status.flag.s?"USP":"SSP",
			cpu->sp.ul,
			printbit(cpu->sp.ul,32));
	
		fprintf(stderr," PC: 0x%08x (%s)\n",
			cpu->pc,
			printbit(cpu->pc,32));
			fprintf(stderr," SR:     0x%04x %c-%c--%03x---%c%c%c%c%c (PC): %s\n\n",
			cpu->status.sr,
			cpu->status.flag.t?'T':'-',
			cpu->status.flag.s?'S':'-',
			cpu->status.flag.imask,
			cpu->status.flag.x?'X':'-',
			cpu->status.flag.n?'N':'-',
			cpu->status.flag.z?'Z':'-',
			cpu->status.flag.v?'V':'-',
			cpu->status.flag.c?'C':'-',
			cpu->ucode[*(unsigned short *)(cpu->space+(cpu->pc&ADDRMASK))].n);
	}
}

void exec68000(mc68000 *cpu) {

	int i;

	// for(i=0;i!=64;i++) {

        	cpu->ucode[*(unsigned short *)(cpu->space+(cpu->pc&ADDRMASK))].f(cpu);
                //cpu->ucode[*(unsigned short *)(cpu->space+(cpu->pc&ADDRMASK))].f(cpu);
                //cpu->ucode[*(unsigned short *)(cpu->space+(cpu->pc&ADDRMASK))].f(cpu);
                //cpu->ucode[*(unsigned short *)(cpu->space+(cpu->pc&ADDRMASK))].f(cpu);
        // }

	cpu->count++;
        // cpu->count+=(4*64);
}

void i68000nop(mc68000 *cpu) {

	cpu->pc+=2;
	cpu->clock+=4;
}

void i68000illegal(mc68000 *cpu) {

	cpu->pc+=2;
	cpu->clock+=4;
}

void i68000move(mc68000 *cpu) {

	unsigned short code,size;

	code=*((unsigned short *)cpu->space+cpu->pc);

	switch(code&0x3000) {

		case 1: size=1; break;
		case 2: size=4; break;
		case 3: size=2; break;
	}
	
	poke(cpu,(code>>6)&7,(code>>9)&7,size,peek(cpu,(code>>3)&7,code&7,size));
	
	cpu->pc+=2;
	cpu->clock+=4;
}

mc68000 *create68000(unsigned char *space) {

	mc68000 *cpu;
	int i,j;

	cpu=(mc68000 *)malloc(sizeof(mc68000));

	cpu->space	=space;
	cpu->clock	=0;
	cpu->count	=0;
	cpu->halt	=1;
	cpu->status.sr	=1<<13;
	cpu->a[7].ul	=*(unsigned int *)(cpu->space+0);
	cpu->pc		=*(unsigned int *)(cpu->space+4);

	cpu->exec	=exec68000;
	cpu->dump	=dump68000;

	for(i=0;i!=65535;i++) {
	
		cpu->ucode[i].f=i68000illegal;
		cpu->ucode[i].n="illegal";
	}

 	for(i=0;i!=4096;i++) {
	
		j=0x1000; cpu->ucode[j+i].f=i68000move; cpu->ucode[j+i].n="move";
		j=0x2000; cpu->ucode[j+i].f=i68000move; cpu->ucode[j+i].n="move";
		j=0x3000; cpu->ucode[j+i].f=i68000move; cpu->ucode[j+i].n="move";
	}

	return cpu;
}


