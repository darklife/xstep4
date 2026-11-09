#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <xstep.h>

struct {

	GLint 	object[256];
	GLfloat x,y,z,ix,iy,iz,b;

} glxworld;

XSWidget *desktop,*window;

void GLXRedraw(XSWidget *wid) {

	int i;
	static int base=0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(glxworld.ix,glxworld.iy,glxworld.iz);
	glRotatef(glxworld.x, 1.0, 0.0, 0.0);
	glRotatef(glxworld.y, 0.0, 1.0, 0.0);
	glRotatef(glxworld.z, 0.0, 0.0, 1.0);

	for(i=0;i!=256;i++) glCallList(glxworld.object[i]);

	glPopMatrix();

	glXSwapBuffers(wid->display,wid->window);

	if(base!=time(0)) {

		base=time(0);
		printf("%.1f fps (z=%f,x=%f,y=%f)\n",
			glxworld.z-glxworld.b,
			glxworld.z,
			glxworld.x,
			glxworld.y);

		glxworld.b=glxworld.z;
	}
		
	glxworld.z++;
}

void GLXResize(XSWidget *wid) {

	XSConfigure(wid);

	glViewport(0, 0, (GLsizei) wid->width, (GLsizei) wid->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0, (GLfloat) wid->width/(GLfloat) wid->height, 0.1, 30.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0,-0.5,-8.0);
}

void GLXKeyPress(XSWidget *wid) {

	int key=XLookupKeysym(&wid->event->xkey,0);

	switch(key) {

		case XK_Up:		glxworld.x  -= 1.0; break;
		case XK_Down:		glxworld.x  += 1.0; break;
		case XK_Left:		glxworld.y  -= 1.0; break;
		case XK_Right:		glxworld.y  += 1.0; break;

		case XK_Escape:		XSWindowClose(wid);
	}
}

void GLXButtonPress(XSWidget *wid) {

	XSetInputFocus(wid->display,wid->window,None,RevertToNone);	
}

void GLXTriangle(GLfloat x,GLfloat y,GLfloat z,GLfloat w) {

	glShadeModel(GL_SMOOTH);

	glBegin(GL_TRIANGLES);
		glVertex3f(x-w+0.1,y-w,z);
		glVertex3f(x+w,y-w,z);
		glVertex3f(x+w,y+w-0.1,z);		
		glVertex3f(x-w,y-w+0.1,z);
		glVertex3f(x+w-0.1,y+w,z);
		glVertex3f(x-w,y+w,z);
	glEnd();
}

void GLXCreate(XSWidget *wid) {

	GLfloat color[4],pos[4]={ 
	
		1.0, 1.0, 0.0, 10.0 
	};
	GLXContext glxcontext;
	XVisualInfo *glxvisual;

	int x,y,glxattribute[] = { 

		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
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

	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
//	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_LINE_SMOOTH);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

	glxworld.x=-94;
	glxworld.y=19;

	for(x=0;x!=16;x++) {
		for(y=0;y!=16;y++) {

			color[0]=x/16.0;
			color[1]=y/16.0;
			color[2]=1.0-((x+y)/32.0);
			color[3]=0.5;
			glxworld.object[x+y*16]=glGenLists(1);
			glNewList(glxworld.object[x+y*16],GL_COMPILE);
			glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
			GLXTriangle(1.0*(x-8),1.0*(y-8),-0.7*1.0,0.425);
			glEndList();
		} 
	}

	glEnable(GL_NORMALIZE);
}

int main(int argc, char **argv) {

	(void)argc;

	desktop = XSDesktop(getenv("DISPLAY"),argv,argc);
	window  = XSWindow(desktop,0,0,640,480,"3D/GLX Application w/ XSTEP");
		  XSButton(window,-8,-8,72,24,"Quit",XSWindowClose);

	window->on.event[ConfigureNotify] 	= GLXResize;
	window->on.event[KeyPress]		= GLXKeyPress;
	window->on.event[ButtonPress]		= GLXButtonPress;
	window->on.create			= GLXCreate;
	window->on.change			= GLXRedraw;

	while(1) XSCheckEvent(desktop,XS_NOBLOCK);

	return 0;
}
