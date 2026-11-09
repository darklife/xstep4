#include <xstep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <jpeglib.h>

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
	}
	return aux;
}

XSImage *readimage(char *name) {

	XSImage *aux=NULL;
	FILE 	*fp;
	char 	tmp[80],format[80],*data,*target,message[256];
	int 	width,height,max,depth,reference;

	fp=fopen(name,"r");
	
	if(fp) {
		
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
