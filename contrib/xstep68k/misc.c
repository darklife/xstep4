#include <stdio.h>
#include <stdlib.h>
#include <mc68000.h>

char *printbit(unsigned int ul,int size) {

	unsigned i,j;
	static char sbit[33];

	j=1<<(size-1);

	for(i=0;i!=size;i++) sbit[i]=(ul&(j>>i))?'1':'0';

	sbit[i]=0;
	
	return sbit;
}

inline unsigned short swap16(w) 

	union {

		unsigned short w;
		unsigned char  b[2];

	} w;
{
	register int t;
	
	t=w.b[0];
	w.b[0]=w.b[1];
	w.b[1]=t;

	return w.w;
}

inline unsigned long swap32(l) 

	union {

		unsigned long l;
		unsigned char b[4];

	} l;
{
	register int t;
	
	t=l.b[0];
	l.b[0]=l.b[3];
	l.b[3]=t;

	t=l.b[1];
	l.b[1]=l.b[2];
	l.b[2]=t;
	
	return l.l;
}

void *address(mc68000 *cpu,int mode,int reg) {

	short d16;
	char  d8;

	switch(mode) {

		case 0:
			return &cpu->d[reg].ub;
		case 2:
			return cpu->space+cpu->a[reg].ul;
		case 3:
			return cpu->space+cpu->a[reg].ul++;
		case 4:
			return cpu->space+(--cpu->a[reg].ul);
		case 5:
			d16=swap16(*((short *)(cpu->space+cpu->pc)));
			cpu->pc+=2;
			return cpu->space+cpu->a[reg].ul+d16;
		case 6:
					
			d16=swap16(*((short *)(cpu->space+cpu->pc)));
			d8=d16&0xff;
			cpu->pc+=2;
			return cpu->space+cpu->a[reg].ul+d8;
	
	}
}
