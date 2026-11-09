#include <xstep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <jpeglib.h>

XSImage  *image,*oldimage;
XSPixmap *pixmap,*old;
XSWidget *d1,*w1,*w2,*w3,*w4;

double 	zoom=1.0;

int	last,next,count,max,inroot;

XSImage *readimage(char *name) {

	XSImage *aux=NULL;
	FILE 	*fp;
	char 	tmp[80],format[80],*data,*target,message[256];
	int 	width,height,max,depth,reference;

	fp=fopen(name,"r");
	
	if(fp) {

		fgets(format,80,fp);

		if(!strncmp(format,"P5",2)) {
		
			while(fgets(tmp,80,fp)) if(tmp[0]!='#') break;
			sscanf(tmp,"%d %d",&width,&height);
			while(fgets(tmp,80,fp)) if(tmp[0]!='#') break;
			sscanf(tmp,"%d",&max);
			data=(char *)malloc(width*height);
			fread(data,1,width*height,fp);
			fclose(fp);
		
			if((aux=XSImageCreate(width,height,data,XS_IMAGE_GRAY))) aux->name=name;
		}
				
		if(!strncmp(format,"P6",2)) {
		
			while(fgets(tmp,80,fp)) if(tmp[0]!='#') break;
			sscanf(tmp,"%d %d",&width,&height);
			while(fgets(tmp,80,fp)) if(tmp[0]!='#') break;
			sscanf(tmp,"%d",&max);
			data=(char *)malloc(width*height*3);
			fread(data,1,width*height*3,fp);
			fclose(fp);
		
			if((aux=XSImageCreate(width,height,data,XS_IMAGE_RGB24))) aux->name=name;
		}
		
		if(!strncmp(format+6,"JFIF",4)) {
		
			struct jpeg_decompress_struct 	cinfo;
			struct jpeg_error_mgr 		jerr;
			
			rewind(fp);

			cinfo.err = jpeg_std_error(&jerr);
			jpeg_create_decompress(&cinfo);
			jpeg_stdio_src(&cinfo, fp);
			jpeg_read_header(&cinfo, TRUE);
			jpeg_start_decompress(&cinfo);
			
			width  = cinfo.output_width;
			height = cinfo.output_height;
			depth  = cinfo.output_components;
			
			data=(char *)malloc(width*height*depth);
			
			reference=time(NULL);
			
			for(target=data;cinfo.output_scanline < cinfo.output_height;target+=width*depth) {
			
				if(reference!=time(NULL)) {
					
					sprintf(message,"working %s: %d %%",name,cinfo.output_scanline*100/cinfo.output_height);
					
					XStoreName(d1->display,w2->window,message);
					XFlush(d1->display);
					
					fprintf(stderr,"\r%s",message);
					
					reference=time(NULL);
				}
				
				jpeg_read_scanlines(&cinfo,(JSAMPARRAY)&target, 1);				
			}
		
			fprintf(stderr,"\r");

			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
			
			fclose(fp);		

			if((aux=XSImageCreate(width,height,data,depth==3?XS_IMAGE_RGB24:XS_IMAGE_GRAY))) aux->name=name;
		}
	}
	return aux;
}

void showimage(void) {

	int w,h;

	static int oldcount;
	
        if(count<1) count=1;
        if(count>=d1->global->argc) count=d1->global->argc-1;

	if(oldcount!=count) {

		oldimage=image;

		if((image=readimage(d1->global->argv[count]))) {

			XStoreName(d1->display,w2->window,image->name);

			fprintf(stderr,
				"sucess: %dx%d pixels read from %s (%s)\n",
				image->width,
				image->height,
				image->name,
				image->format==XS_IMAGE_RGB24?"RGB24":"GRAY");

			if(oldimage) {
			
				XSImageDestroy(oldimage);
				oldimage=NULL;
			}
	
		} else {
		
			fprintf(stderr,"%s: failed to open\n",d1->global->argv[count]);
			image=oldimage;
		}
	}

	if(max) {
		
		if(!inroot) {
		
			if(w1->parent->width*1.0/image->width < w1->parent->height*1.0/image->height)
			
				zoom=w1->parent->width*1.0/image->width;
			else
				zoom=w1->parent->height*1.0/image->height;
		}
	}

	if(image) {
		
		if(inroot&&max) {
		
			w=d1->width;
			h=d1->height;
	
		} else {
		
			w=image->width*zoom;
			h=image->height*zoom;
		}

		old=pixmap; 
		pixmap=XSPixmapCreate(inroot?d1:w1,w,h,NULL,XS_PIXMAP_XSHM);

		XSImage2Pixmap(w1,image,pixmap,False);

		if(inroot) {
		
			XSetWindowBackgroundPixmap(w1->display,w1->desktop->window,pixmap->pixmap);
			XClearWindow(w1->display,w1->desktop->window);
		}

		XSetWindowBackgroundPixmap(w1->display,w1->window,pixmap->pixmap);
		w1->geometry.w=w1->width =w;
		w1->geometry.h=w1->height=h;
		XSScrollCheck(w3);

		XClearWindow(w1->display,w1->window);
	}

	if(old) {
		
		XSPixmapDestroy(w1,old);
		old=NULL;
	}

	oldcount=count;
}

void foo(XSWidget *wid) {

        int key=XLookupKeysym(&wid->event->xkey,0);

//	printf("debug: keypressed=%d\n",key);

        switch(key) {

		case XK_Home:
			count=1;
			break;
			
		case XK_End:
			count=d1->global->argc-1;
			break;

		case XK_Page_Up:
			count-=10;
			break;
			
		case XK_Page_Down:
			count+=10;
			break;

		case XK_Down:
			count++;
			break;
		
		case XK_Up:
			count--;
			break;
			
                case XK_Escape:
                	XSWindowClose(w1);
                	return;
                	
                case '=':
                	zoom+=0.1;
                	break;
                case '-':
                	zoom-=0.1;
                	break;
                case 'm':
                	max=1;
                	break;
		case 'n':
		case '1':
			max=0;
			zoom=1.0;
			break;
		case '2':
			max=0;
			zoom=2.0;
			break;
		case '3':
			max=0;
			zoom=3.0;
			break;
		case '4':
			max=0;
			zoom=4.0;
			break;
		case '5':
			max=0;
			zoom=0.5;
			break;
		case '7':
			max=0;
			zoom=0.75;
			break;
		default: 
			return;
        }

	if(zoom<0.1)  zoom=0.1;
	if(zoom>10.0) zoom=10.0;

	showimage();
}

void bar(XSWidget *wid) {

	XSWidgetCreate(wid);
	wid->on.create=NULL;

	showimage();
}

int main(int argc,char **argv) {

	int i,width=600,height=600;
	
	char *displayname;

	for(i=1;argv[i];i++) {
	
		if(!strcmp(argv[i],"-display"))  displayname=argv[i+1];
		if(!strcmp(argv[i],"-inroot"))   inroot=True;
		if(!strcmp(argv[i],"-geometry")) {

			if(argv[i+1]) sscanf(argv[i+1],"%dx%d",&width,&height);
		}
	}
	
	d1=XSDesktop(getenv("DISPLAY"),argv,argc);

	w2=XSWindow(d1,0,0,width,height,argv[0]);
	w2->on.event[KeyPress]=foo;

	w3=XSScroll(w2,0,0,0,0,600,600);

	w1=w3->viewport;
	w1->on.create=bar;

	while(XSCheckEvent(d1,XS_BLOCK));
	
	return 0;
}
