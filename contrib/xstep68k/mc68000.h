/*
	MC68000 Configurations:
	
	8x 8/16/32 bit data registers, 8x 16/32 bit address registers, 
	16 bit supervisor status register, 8 bit condition code register, 
	16MB of address space.
*/

#define ADDRMASK        0xffffff

typedef struct mc68000 {

	
	union {
	
		unsigned char 	ub;
		unsigned short 	uw;
		unsigned int	ul;
		
		char		b;
		short		w;
		int		l;
		
	} d[8];

	union {
	
		unsigned short 	uw;
		unsigned int	ul;
		
		short		w;
		int		l;
		
	} a[8],sp;
	
	union {
	
		unsigned char  ccr;
		unsigned short sr;
	
		struct {
		
			unsigned short c:1;
			unsigned short v:1;
			unsigned short z:1;
			unsigned short n:1;
			unsigned short x:1;
			unsigned short pad1:3;
			unsigned short imask:3;
			unsigned short pad2:2;
			unsigned short s:1;
			unsigned short pad3:1;
			unsigned short t:1;

		} flag;
			
	} status;

	unsigned int 	clock,count,halt,pc;
	void 		*space;

	struct {

		void (*f)(struct mc68000 *);
		char *n;

	} ucode[65536];

} mc68000;

mc68000	*create68000(unsigned char *space);
void 	print68000(mc68000 **,int);
