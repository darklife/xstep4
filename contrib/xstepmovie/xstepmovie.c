#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <mpeg.h>
#include <xstep.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

/*
	descritores p/ objetos graficos utilizados.
*/

XSWidget 	*desktop,
		*window;
XSPixmap 	*pixmap;
XSImage  	*ximage;

unsigned	delay,rate;

/*
	funcao de atualizacao do bitmap na tela: GetMPEGFrame()
	retorna um frame em formato RGB32 e XSImage2Pixmap() copia
	o ximage local p/ o pixmap do X server, para  ser entao
	atualizado no background da janela.
*/

void XSMPEGUpdate(XSWidget *wid) {
      
      	static int reference=0,count=0,total=0;
      
	if(GetMPEGFrame(ximage->data)) {

		XSImage2PixmapAA(window,ximage,pixmap,False);
		XClearWindow(desktop->display,window->window);
		XFlush(desktop->display);

		/*
			printa o fps a cada segundo.
		*/
		
		count++; total++;

//		usleep(delay);
		
		if(time(0)==reference) return;

		reference=time(0);
		
		if(count<rate) delay/=2;
		if(count>rate) delay*=2;
		
		fprintf(stderr,"\r%s/%d: %d fps, d/r=%d/%d ",
			wid->name,
			total,
			count,
			delay,
			rate);
		
		count=0;
		
		return;
	}

	/*
		se nao tem mais frames no mpeg, cai fora.
	*/
	wid->on.change=NULL;

	XSWindowClose(wid);
}

/*
	funcao principal, inicializa o programa, le o mpeg e
	cria os objetos graficos. no final tem o loop de eventos.
*/

int main (int argc, char **argv){

	FILE		*fp;
	ImageDesc	img;	/* descritor retornado pela libmpeg */
	char		*name,*displayname;
	double		zoom=1.0;
	int		n;

	/*
		parametros default e parametros de comando.
	*/

	displayname	= getenv("DISPLAY");
	zoom		= 1.0;

	for(n=1;argv[n];n++) {
	
		if(!strcmp(argv[n],"-display")) displayname=argv[n+1];
		if(!strcmp(argv[n],"-zoom")) 	zoom=atof(argv[n+1]);
	}

	/*
		tenta abrir conexao com o display.
	*/

	if(!(desktop=XSDesktop(displayname,argv,argc))) {

		perror("XSDesktop");
		return -1;   	
   	}
	
	/*
		tenta abrir arquivo mpeg. pode ser o arquivo
		ou pode ser stdin, se argv[n-1] for NULL. se o 
		arquivo falhar na abertura, cai fora.
	*/

	fp 	= strcmp(argv[n-1],"-")?fopen(argv[n-1],"r"):stdin;
	name	= fp == stdin?"(stdin)":argv[n-1];

	if(!fp) {

		perror ("fopen");
		return -1;
	}
      
      	/*
      		seta flags globais na libmpeg e tenta abrir o
      		arquivo mpeg no descompressor. se o formato nao for
      		valido, cai fora.
      	*/
      
	SetMPEGOption (MPEG_DITHER,(int)FULL_COLOR_DITHER);

	if(!OpenMPEG(fp,&img)) {

		perror("OpenMPEG");
		return -1;
	}
	
	/*
		printa informacoes sobre o movie.
	*/

	delay=1000000/(rate=img.PictureRate);
	
	fprintf (stderr,"MPEG movie %s: %dx%d pixels (depth %d, xstep zoom %f)\n",
		name,
		img.Width, 
		img.Height, 
		img.PixelSize,
		zoom);

	fprintf (stderr,"MPEG movie %s: required picture rate = %d, required bit rate = %d\n",
		name,
		img.PictureRate,
		img.BitRate);

	/*
		cria elementos graficos no tamanho da imagem:
		
			- janela
			- pixmap (no X server)
			- imagem (no X client)
			
		o pixmap eh linkado como background da janela,
		assim o X server atualiza ele automaticamente quando
		houver um refresh da janela.
		
		o pixmap tem um lado client e um lado server,
		conectado normalmente por XSHM: o objetivo eh acelerar
		a transferencia de imagens do client p/ o server.
		
		a imagem ximage eh um buffer independente de 
		display, com formato RGB de 32 bits. ele eh convertido
		para o formato do display na copia para o pixmap.
	*/

	ximage=XSImageCreate(img.Width,img.Height,NULL,XS_IMAGE_RGB32);

	window=XSWindow(desktop,0,0,
		(int)(img.Width*zoom),
		(int)(img.Height*zoom),name);

	pixmap=XSPixmapCreate(desktop,
		(int)(img.Width*zoom),
		(int)(img.Height*zoom),
		NULL,
		XS_PIXMAP_XSHM);

	window->attributes.background_pixmap = pixmap->pixmap;
	window->on.change = XSMPEGUpdate;

	/*
		loop de eventos: ira rodar ateh q nao tenha
		mais janelas abertas no objeto desktop. o usleep()
		faz o programa parar um intervalo de tempo, no
		caso joga ele p/ o fim da fila de processos ativos
		e em uma maquina idle isso se traduz a bloquear
		o numero de loops/segundo a 50.
	*/

	while(XSCheckEvent(desktop,XS_NOBLOCK)) usleep(1);

	/*
		joga fora o pixmap alocado previamente.
	*/

	XSPixmapDestroy(desktop,pixmap);
				
	fprintf(stderr,"done.\n");
			
	return 0;
}
