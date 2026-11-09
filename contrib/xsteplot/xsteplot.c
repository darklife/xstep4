#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xstep.h>
#include <string.h>

int 	cols,rows,color[6],vectorize=False;
float 	**data,*max,gmax;
char	**title;

void keys(XSWidget *w) {

	int key=XLookupKeysym(&w->event->xkey,0);
	
	if(key==XK_Escape) XSWindowClose(w);
}

void plot(XSWidget *w) {

	char 	smax[256];
	int	flag=1,x,y;

	for(x=0;x!=cols;x++) {	

		XSetForeground(w->display,w->gc,color[x]);

		for(y=0;y!=(vectorize?rows-1:rows);y++) {

			if(vectorize) 

				XDrawLine(w->display,w->window,w->gc,
					8+(int)y*(w->width-16)/rows,w->height-8-(int)(data[x][y]*(w->height-16)/gmax),
					8+(int)(y+1)*(w->width-16)/rows,w->height-8-(int)(data[x][y+1]*(w->height-16)/gmax));
			else
				XDrawPoint(w->display,w->window,w->gc,
					8+(int)y*(w->width-16)/rows,w->height-8-(int)(data[x][y]*(w->height-16)/gmax));


/*
			if(data[x][y]==max[x]&&flag) {
		
				XDrawLine(w->display,w->window,w->gc,
					8+(int)y*(w->width-16)/rows,8,
					8+(int)y*(w->width-16)/rows,w->height-8);

				flag=0;
			}
*/
		}
/*
		XDrawLine(w->display,w->window,w->gc,
			8,w->height-8,
			w->width-8,w->height-8);
*/
		sprintf(smax,"col[%d]=%e",x,max[x]);

		XDrawString(w->display,w->window,w->gc,
			8,16+16*x,smax,strlen(smax));

	}
}

int main(int i,char **p) {

	FILE 	 	*fp;
	XSWidget 	*d,*w;
	char 		*file="-";
	int 		x,y;


	for(i=1;p[i];i++) {
	
		if(!strncmp(p[i],"-c",2)) cols=atoi(p[i]+2); else
		if(!strncmp(p[i],"-r",2)) rows=atoi(p[i]+2); else
		if(!strncmp(p[i],"-v",2)) vectorize=True; else
		file=p[i];
	}

	fp=*file=='-'?stdin:fopen(file,"r");

	data  =(float **)malloc(cols*sizeof(float *));
	max   =(float *) malloc(cols*sizeof(float));
	gmax  =0.0;

	for(x=0;x!=cols;x++) { 
	
		max[x]=0.0;

		data[x]=(float *)malloc(rows*sizeof(float *));
	}

	for(y=0;y!=rows;y++) {
		
		for(x=0;x!=cols;x++) {
		
			fscanf(fp,"%f",&(data[x][y]));
			
			if(data[x][y]>max[x]) max[x]=data[x][y];
			if(data[x][y]>gmax)   gmax=data[x][y];
		}
	}

	if(fp!=stdin) fclose(fp);
	
	d=XSDesktop("",p,i);

	w=XSWindow(d,0,0,400,400,file);

	w->on.event[Expose]	=plot;
	w->on.event[KeyPress]	=keys;
	w->bgcolor		=w->black;
	w->index		=x;

	color[0]=XSNamedColor(d,"red");
	color[1]=XSNamedColor(d,"yellow");
	color[2]=XSNamedColor(d,"green");
	color[3]=XSNamedColor(d,"cyan");
	color[4]=XSNamedColor(d,"blue");
	color[5]=XSNamedColor(d,"magenta");
	
	while(XSCheckEvent(d,False));

	return 0;
}
