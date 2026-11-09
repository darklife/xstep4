#include <xstep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <jpeglib.h>

XSImage *XSIReadJPEG(FILE *fp) {

	XSImage *aux=NULL;
	char 	format[80],*data,*target;
	int 	width,height,depth;

	struct jpeg_decompress_struct 	cinfo;
	struct jpeg_error_mgr 		jerr;

	if(fp) {

		rewind(fp);
		
		fgets(format,80,fp);
		
		if(!strncmp(format+6,"JFIF",4)) {
					
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
			
			for(	target=data;
				cinfo.output_scanline<cinfo.output_height;
				target+=width*depth)
					jpeg_read_scanlines(&cinfo,(JSAMPARRAY)&target,1);
			
			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
			
			fclose(fp);

			if((aux=XSImageCreate(width,height,data,depth==3?XS_IMAGE_RGB24:XS_IMAGE_GRAY))) aux->name=name;
		}
	}
	return aux;
}
