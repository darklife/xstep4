#include <xstep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE  1024

typedef struct {

	char 	*name;
	char	*latitude;
	char 	*longitude;
	char 	*place;
	char 	*country;
	char	*data;

	float	x,y,t;

} DBCache;

DBCache **db;

float tmax=0.0,tmin=100.0,dt;

char *find(char *p,int f) {

	static char q[MAXSIZE];

	while(*p&&f) if(*p++==';') f--;

	for(f=0;*p&&*p!=';';f++) q[f]=*p++;
	q[f]=0;
	
	return q;
}

float hms2f(char *p) {

	float	f;
	int 	i=0,
		hms[3]={ 0, 0, 0 };
	char 	*q;

	hms[0]=atoi(p);
	
	for(i=1;(q=strchr(p,'-'));i++) hms[i]=atoi(p=q+1);

	f=hms[0]+hms[1]/60.0+hms[2]/3600.0;

	return (strchr(p,'N')||strchr(p,'W'))?-f:f;
}

void AppExpose(XSWidget *wid) {

	DBCache *aux;

	int x,y,i,t;
	
	for(i=0;db[i];i++) {

		if(db[i]->t!=-1) {
	
			dt=127.0/(tmax-tmin);
			t=(int)(db[i]->t*dt);
			XSetForeground(wid->display,wid->gc,XSColor(wid,128+t,0,255-t));
		} else
			XSetForeground(wid->display,wid->gc,XSColor(wid,32,32,32));
	
		x=(int)(db[i]->x*(wid->width/360.0))+wid->width/2;
		y=(int)(db[i]->y*(wid->height/180.0))+wid->height/2;

		XDrawPoint(wid->display,wid->window,wid->gc,x,y);
	}
}

int main(int argc,char **argv) {

	XSWidget 	*d1,*w1;
	FILE 		*fp;
	int 		i,j,k;	
	char 		buffer[MAXSIZE],*q,*p;
	float		t0,t1;
	
	if(!(fp=fopen("nsd_cccc","r"))) {
	
		perror("fopen(nsd_cccc,r)");
		return -1;
	}

	for(i=0;fgets(buffer,MAXSIZE,fp);i++) {

		db=(DBCache **)realloc(db,sizeof(DBCache *)*(i+2));
		
		db[i]=(DBCache *)malloc(sizeof(DBCache));

		db[i]->name	=strdup(find(buffer,0));
		db[i]->latitude	=strdup(find(buffer,7));
		db[i]->longitude=strdup(find(buffer,8));
		db[i]->place	=strdup(find(buffer,3));
		db[i]->country	=strdup(find(buffer,5));
		db[i]->data	=NULL;

		db[i]->y	=hms2f(db[i]->latitude);
		db[i]->x	=hms2f(db[i]->longitude);
		db[i]->t	=-1;
	}

	db[i]=NULL;
		
	fclose(fp);

	fprintf(stderr,"nsd_cccc: read %d entries.\n",k=i);

	if(!(fp=fopen("00.TXT","r"))) {
	
		perror("fopen(00.TXT,r)");
		return -1;
	}

	for(i=0;fgets(buffer,MAXSIZE,fp);) {

		if(*buffer<'A'||*buffer>'Z') continue;
				
		for(j=(*buffer-'A')/2;j!=k;j++) {
		
			if(*buffer!=*db[j]->name) continue;
			
			if(!strncmp(buffer,db[j]->name,4)) {
			
				db[j]->data=strdup(buffer);
				i++;

				if(!(i%256)) 
					fprintf(stderr,"\r%d of %d (%d%%)...",i,k,i*100/k);
			}
		}
	}
	
	fclose(fp);

	fprintf(stderr,"cycle 00.TXT: read %d entries.\n",i);	

	for(i=0;i!=k;i++) {
	
		if(db[i]->data) {

			for(p=db[i]->data;(q=strchr(p,' '));p=q) {
		
				q++;
			
				if(isdigit(q[0])&&isdigit(q[1])&&q[2]=='/') {
				
					db[i]->t=atoi(q);

					if(db[i]->t>tmax) tmax=db[i]->t;
					if(db[i]->t<tmin) tmin=db[i]->t;
				
					break;
				}
			}
		}
	}

	fprintf(stderr,"scale: %f to %f celsius\n",tmin,tmax);

	d1=XSDesktop(getenv("DISPLAY"),argv,argc);

	w1=XSWindow(d1,0,0,800,600,"xstepweather");

	w1=XSScroll(w1,0,0,0,0,360*4,180*4);

	w1=w1->viewport;

	w1->bgcolor=w1->black;
	w1->on.event[Expose]=AppExpose;

	while(XSCheckEvent(d1,XS_BLOCK));

	return 0;
}
