typedef struct mc68901 {

	struct device901 {

		struct {
	
			union {
		
				unsigned short pad;
				unsigned char  data;
								
			} port,active,direction;
				
		} parallel;
	
		struct {
	
			struct {
	
				union {
		
					unsigned short pad;
					unsigned char  data;
		
					unsigned char parallel7:1;
					unsigned char parallel6:1;
					unsigned char timera:1;
					unsigned char rxbuffer:1;
					unsigned char rxerror:1;
					unsigned char txbuffer:1;
					unsigned char txerror:1;
					unsigned char timerb:1;
		
				} one;
	
				union {
			
					unsigned short pad;
					unsigned char  data;
			
					unsigned char parallel5:1;
					unsigned char parallel4:1;
					unsigned char timerc:1;
					unsigned char timerd:1;
					unsigned char parallel3:1;
					unsigned char parallel2:1;
					unsigned char parallel1:1;
					unsigned char parallel0:1;
	
				} two;

			} enable,pending,inservice,mask;	
		
			union {
		
				unsigned short pad;
				unsigned char  data;

			} vector;
	
		} interrupt;
	
		struct {
	
			union {
			
				unsigned short pad;
				unsigned char  data;

			} a,b,c,d;

			union {
			
				unsigned short pad;
				unsigned char  data;

				unsigned char mode:4;
				unsigned char reset:1;

			} controla,controlb;

			union {
			
				unsigned short pad;
				unsigned char  data;

				unsigned char modec:3;
				unsigned char pad2:1;
				unsigned char moded:3;

			} controlcd;
		
		} timer;
	
		struct {
	
			union {
			
				unsigned short pad;
				unsigned char data;
			
			} sync;

			union {
			
				unsigned short pad;
				unsigned char data;
			
				unsigned char unused:1;
				unsigned char parity:2;
				unsigned char bits:2;
				unsigned char size:2;
				unsigned char clock:1;
			
			} control;
			
			union {
			
				unsigned short pad;
				unsigned char data;
				
				unsigned char enable:1;
				unsigned char match:1;
				unsigned char start:1;
				unsigned char sync:1;
				unsigned char frame:1;
				unsigned char parity:1;
				unsigned char overrun:1;
				unsigned char ready:1;
			
			} rxstatus;

			union {
			
				unsigned short pad;
				unsigned char data;
				
				unsigned char enable:1;
				unsigned char mode:2;
				unsigned char stop:1;
				unsigned char end:1;
				unsigned char send:1;
				unsigned char underrun:1;
				unsigned char ready:1;
			
			} txstatus;

			union {
			
				unsigned short pad;
				unsigned char data;
			
			} port;
	
		} serial;

	} *device;

	unsigned int base,count;

} mc68901;

void print68901(mc68901 **mfp,int count);
mc68901 *create68901(unsigned char *space,int base);
                
