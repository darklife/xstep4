#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <xstep.h>

struct {

	struct OBJECT {

		char	*name;
		int  	vsize,psize;

		struct VECTOR { 
		
			GLfloat x,y,z; 

		} **vector;

		struct POLYGON { 

			int 	size,
				vector[64];
				
		} **polygon;

		struct OBJECT *next;

		GLint glxlint;
		
	} *object;

	GLfloat x,y,z,ix,iy,iz,b,ax,ay,az;

} glxworld;

XSWidget *desktop,*window,*control;

int light;

void GLXRedraw(XSWidget *wid) {

	struct OBJECT *aux;
	
	int z;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	for(z=0,aux=glxworld.object;aux;aux=aux->next,z++) {

		glPushMatrix();
	
		glTranslatef(glxworld.ix,glxworld.iy,glxworld.iz);

		glRotatef(glxworld.x*z, 1.0, 0.0, 0.0);
		glRotatef(glxworld.y, 0.0, 1.0, 0.0);
		glRotatef(glxworld.z*z, 0.0, 0.0, 1.0);

		glCallList(aux->glxlint);

		glPopMatrix();
	}

	glXSwapBuffers(wid->display,wid->window);
}

void GLXResize(XSWidget *wid) {

	printf("*** CONFIGURE NOTIFY ***\n");

	XSConfigure(wid);

	glViewport(0, 0, (GLsizei) wid->width, (GLsizei) wid->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();   
	gluPerspective(45, (GLfloat) wid->width/(GLfloat) wid->height, 0.1, 100);
//	gluPerspective(0.0, (GLfloat) wid->width/(GLfloat) wid->height, 1.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0,0.0,-20.0);
}

void GLXKeyPress(XSWidget *wid) {

	int key=XLookupKeysym(&wid->event->xkey,0);

	switch(key) {

		case XK_Up:		glxworld.ax -= 1.0; break;
		case XK_Down:		glxworld.ax += 1.0; break;
		case XK_Left:		glxworld.ay -= 1.0; break;
		case XK_Right:		glxworld.ay += 1.0; break;
		case 'w':		glxworld.iz -= 0.5; break;
		case 'z':		glxworld.iz += 0.5; break;
		case 'a':		glxworld.az -= 0.5; break;
		case 'd':		glxworld.az += 0.5; break;

		case XK_Escape:		XSWindowClose(window);
//					XSWindowClose(control);
	}
}

void GLXButtonPress(XSWidget *wid) {

	XSetInputFocus(wid->display,wid->window,None,RevertToNone);	
}

void GLXCreate(XSWidget *wid) {

	GLfloat nx,ny,nz,ax,ay,az,bx,by,bz,sn,cx,cy,cz;
	GLfloat color[4];
	GLfloat LightAmbient[] 	= { 0.5, 0.5, 1.0, 1.0 };
	GLfloat LightDiffuse[] 	= { 0.5, 0.5, 1.0, 1.0 };
	GLfloat LightSpecular[] = { 0.5, 0.5, 1.0, 1.0 };
	GLfloat LightPosition[]	= { 1.0, 1.0, 1.0, 0.0 };
	GLint	materialspec	= 20;
	GLfloat specularity[]	= { 0.0, 0.0, 0.0, 1.0 };
	GLfloat MaterialSpecular [] = { 0, 0, 0, 1.0 };
	GLfloat MaterialShininess [] = { 100.0 };

	GLXContext glxcontext;
	XVisualInfo *glxvisual;

	struct OBJECT *aux;

	int x=0,y=0,z,glxattribute[] = { 

		GLX_RGBA,
		GLX_RED_SIZE, 	1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 	1,
		GLX_DOUBLEBUFFER,
		None 
	};

	XSWidgetCreate(wid);

	glxvisual = glXChooseVisual(desktop->display,
		DefaultScreen(desktop->display),
		glxattribute);

	if (!glxvisual) {

		fprintf(stderr,
			"fatal error: can't initialize GLX in display %s\n",
			desktop->global->name);

		exit(-1);
	}

	glxcontext = glXCreateContext(desktop->display,glxvisual,NULL,True);

	glXMakeCurrent(desktop->display,window->window,glxcontext);
/*
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR,LightSpecular);
*/
	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);

	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

//	glClearColor(0.0,0.0,0.5,1.0);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT,GL_FILL);

        glEnable(GL_LINE_SMOOTH);
                        

	for(z=0,aux=glxworld.object;aux;aux=aux->next,z++) {



		color[0]=1.0;
		color[1]=1.0;
		color[2]=1.0;
		color[3]=1.0;

		aux->glxlint=glGenLists(1);
		glNewList(aux->glxlint,GL_COMPILE);

		fprintf(stderr,"get object %d/%d/%s",aux->glxlint,z,aux->name); 

		glMaterialfv(GL_FRONT,GL_SPECULAR,specularity);
		glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
		glMateriali(GL_FRONT,GL_SHININESS,materialspec);
//		glLightModelfv(GL_LIGHT_MODEL_AMBIENT,LightAmbient);

		for(x=0;x!=aux->psize;x++) {

/*
			normal vector:

			          |nx ny nz|
			N = AxB = |ax ay az|=(ay*bz-az*by)nx+(az*bx-ax*bz)ny+(ax*by-ay*bx)nz
 			          |bx by bz|

			N = N/|N|

			A and B must be in the same plane.
*/

			ax = aux->vector[aux->polygon[x]->vector[1]]->x
			    -aux->vector[aux->polygon[x]->vector[0]]->x;
			ay = aux->vector[aux->polygon[x]->vector[1]]->y
			    -aux->vector[aux->polygon[x]->vector[0]]->y;
			az = aux->vector[aux->polygon[x]->vector[1]]->z
			    -aux->vector[aux->polygon[x]->vector[0]]->z;

			bx = aux->vector[aux->polygon[x]->vector[2]]->x
			    -aux->vector[aux->polygon[x]->vector[1]]->x;
			by = aux->vector[aux->polygon[x]->vector[2]]->y
			    -aux->vector[aux->polygon[x]->vector[1]]->y;
			bz = aux->vector[aux->polygon[x]->vector[2]]->z
			    -aux->vector[aux->polygon[x]->vector[1]]->z;

			nx=ay*bz-az*by;
			ny=az*bx-ax*bz;
			nz=ax*by-ay*bx;

			sn=sqrt(nx*nx+ny*ny+nz*nz);

			nx/=sn;
			ny/=sn;
			nz/=sn;

			// object

			glBegin(GL_TRIANGLE_FAN);
			glNormal3f(nx,ny,nz);
			glColor3f(1,0,0); 
			
			cx=cy=cz=0.0;
			
			for(y=0;y!=aux->polygon[x]->size;y++) {

				glVertex3f(
					aux->vector[aux->polygon[x]->vector[y]]->x,
					aux->vector[aux->polygon[x]->vector[y]]->y,
					aux->vector[aux->polygon[x]->vector[y]]->z);

				cx+=aux->vector[aux->polygon[x]->vector[y]]->x;
				cy+=aux->vector[aux->polygon[x]->vector[y]]->y;
				cz+=aux->vector[aux->polygon[x]->vector[y]]->z;
			}
			
			glEnd();
/*
			cx=cx/(float)y;
			cy=cy/(float)y;
			cz=cz/(float)y;

			// AxB

			glBegin(GL_LINE_STRIP);
			glColor3f(0,1.0,0);
			glVertex3f(cx,cy,cz);
			glVertex3f(nx/40+cx,ny/40+cy,nz/40+cz);
			glEnd();
*/

		}
		glEndList();
	}

	wid->on.event[ConfigureNotify] 	= GLXResize;

}

struct OBJECT *GLXReadObject(char *name,struct OBJECT *last) {

	struct OBJECT *aux;

	int j,k;
	FILE *f;
	char buffer[4096];

	printf("loading object from file %s...",name);
	fflush(stdout);

	aux=(struct OBJECT *)malloc(sizeof(struct OBJECT));
		
	f=fopen(name,"r");
		
	do fgets(buffer,4096,f); while(*buffer=='#');
	
	aux->name=(char *)malloc(1+strlen(buffer));
	strcpy(aux->name,buffer);
	
	do fgets(buffer,4096,f); while(*buffer=='#');

	sscanf(buffer,"%d %d",&aux->vsize,&aux->psize);
		
	printf(" [detected: %d vectors, %d poligons]...",
		aux->vsize,
		aux->psize);
			
	fflush(stdout);

	aux->vector=(struct VECTOR **)malloc(aux->vsize*sizeof(struct VECTOR));
		
	for(j=0;j!=aux->vsize;j++) {
		
		aux->vector[j]=(struct VECTOR *)malloc(sizeof(struct VECTOR));
		
		fscanf(f,"%s %f %f %f",
			buffer,
			&aux->vector[j]->x,
			&aux->vector[j]->y,
			&aux->vector[j]->z);
			
		aux->vector[j]->x/=2.0;
		aux->vector[j]->y/=2.0;
		aux->vector[j]->z/=2.0;
	}

	aux->polygon=(struct POLYGON **)malloc(aux->psize*sizeof(struct POLYGON));
		
	for(j=0;j!=aux->psize;j++) {

		aux->polygon[j]=(struct POLYGON *)malloc(sizeof(struct POLYGON));		

		do fscanf(f,"%s",buffer); while(!atoi(buffer));
			
		aux->polygon[j]->size=atoi(buffer);

		for(k=0;k!=aux->polygon[j]->size;k++) {
		
			fscanf(f,"%d",&aux->polygon[j]->vector[k]);	
			aux->polygon[j]->vector[k]--;
		}
	}

	printf(" wow! done!\n");
		
	fclose(f);

	aux->next=last;

	return(aux);
}

void GLXQuit(XSWidget *wid) {

//	XSWindowClose(control);
	XSWindowClose(window);
}

int main(int argc, char **argv) {

	int base=0,i,fps=0;
		
	for(i=1;i!=argc;i++) 
		glxworld.object=GLXReadObject(argv[i],glxworld.object);

	desktop = XSDesktop(getenv("DISPLAY"),argv,argc);
/*
	control = XSWindow(desktop,0,0,400,200,"GLX Controls");
		  XSCheck (control,8,8+21*0,160,21,"Light",&light,1);
		  XSButton(control,-8,-8,72,24,"Quit",GLXQuit);
*/
	window = XSWindow(desktop,0,0,640,480,argv[0]);

	window->on.create 		= GLXCreate;
	window->on.event[KeyPress]	= GLXKeyPress;
	window->on.event[ButtonPress]	= GLXButtonPress;

/*
	glxworld.z+=60.0;
	glxworld.x+=-60.0;
	glxworld.y+=0.0;
*/
	while(XSCheckEvent(desktop,XS_NOBLOCK)) {

		if(desktop->child) GLXRedraw(window);

		if(base!=time(0)) {

			base=time(0);
			printf("%d fps (z=%f,x=%f,y=%f)\n",
				fps,
				glxworld.z,
				glxworld.x,
				glxworld.y);

			fps=0;

		} else fps++;
		
		glxworld.z+=glxworld.az;
		glxworld.x+=glxworld.ax;
		glxworld.y+=glxworld.ay;

		usleep(1);
	}
	return 0;
}
