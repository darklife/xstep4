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

inline unsigned swap16(unsigned short a) {

	unsigned char *p=(unsigned char *)&a;

	p[0]^=p[1]; p[1]^=p[0]; p[0]^=p[1];

	return a;
}

inline unsigned swap32(unsigned a) {

	unsigned char *p=(unsigned char *)&a;

	p[0]^=p[3]; p[3]^=p[0]; p[0]^=p[3];
	p[1]^=p[2]; p[2]^=p[1]; p[1]^=p[2];

	return a;
}
                        

inline void *address(mc68000 *cpu,int mode,int reg,int size) {

	int   Xn=0;
	short d16;
	char  d8;
	void  *ret=0;

	switch(mode) {

		case 0:
			ret = &cpu->d[reg].ul;
			break;
		case 1:
			ret = &cpu->a[reg].ul;
			break;
		case 2:
			ret = cpu->space+cpu->a[reg].ul;
			break;
		case 3:
			cpu->clock+=2;
			cpu->a[reg].ul+=size;
			ret = cpu->space+cpu->a[reg].ul;
			cpu->a[reg].ul+=size;
			break;
		case 4:
			cpu->clock+=2;
			cpu->a[reg].ul-=size;
			ret = cpu->space+cpu->a[reg].ul;
			break;
		case 5:
			cpu->clock+=4;
			d16=swap16(*((short *)(cpu->space+cpu->pc)));
			cpu->pc+=2;
			ret = cpu->space+cpu->a[reg].ul+d16;
			break;
		case 6:	
			cpu->clock+=4;				
			d16=swap16(*((short *)(cpu->space+cpu->pc)));
			d8=d16&0xff;
			cpu->pc+=2;
			ret = cpu->space+cpu->a[reg].ul+d8+Xn; // esse Xn tah na word extra!
			break;
		case 7:
			
			switch(reg) {
			
				case 0:
					cpu->clock+=4;
					ret = (void *) swap16(*((short *)(cpu->space+cpu->pc)));
					cpu->pc+=2;
					break;						
				case 1:
					cpu->clock+=8;
					ret = (void *) swap32(*((unsigned *)(cpu->space+cpu->pc)));
					cpu->pc+=4;
					break;
				case 2:
					cpu->clock+=4;
					d16=swap16(*((short *)(cpu->space+cpu->pc)));
					cpu->pc+=2;
					ret = cpu->space+cpu->pc+d16;
					break;					
				case 3:
					cpu->clock+=4;
					d16=swap16(*((short *)(cpu->space+cpu->pc)));
					d8=d16&0xff;
					cpu->pc+=2;
					ret = cpu->space+cpu->pc+d8+Xn; // esse Xn tah na word extra!
					break;				
				case 4:
					cpu->clock+=8;
					ret = cpu->space+cpu->pc;
					cpu->pc+=size;
					break;
			}

			break;	
	}

	return ret;
}

inline unsigned peek(mc68000 *cpu,int mode,int reg,int size) {

	union {

		unsigned 	*d32;
		unsigned short 	*d16;
		unsigned char	*d8 ;
	} data;

	data.d32=address(cpu,mode,reg,size);

	switch(size) {
	
		case 1:	
			cpu->clock+=4;
			return *data.d8;
		case 2: 
			cpu->clock+=4;
			return *data.d16;
		case 4: 
			cpu->clock+=8;
			return *data.d32;
	}

	return 0;
}

inline void poke(mc68000 *cpu,int mode,int reg,int size,unsigned info) {

	union {

		unsigned 	*d32;
		unsigned short 	*d16;
		unsigned char	*d8 ;
	} data;

	data.d32=address(cpu,mode,reg,size);

	switch(size) {
	
		case 1:	
			cpu->clock+=4;
			*data.d8=info;
			break;
		case 2: 
			cpu->clock+=4;
			*data.d16=info;
			break;
		case 4: 
			cpu->clock+=8;
			*data.d32=info;
			break;
	}
}
