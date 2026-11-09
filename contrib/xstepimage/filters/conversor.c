#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	ATENCAO ATENCAO ATENCAO ATENCAO ATENCAO ATENCAO ATENCAO

	para compilar isso: 
	
		gcc -Wall -O6 conversor.c -o conversor

	esse codigo nao esta portavel para plataformas nao-intel,
	e exige um ambiente 32 bits unix-like, em particular que
	forneca o gcc. existem atributos para compressao de structs
	q provavelmente soh funcionam no gcc e devem dar problema
	com outros compiladores em outros unix (turbo-c nem 
	pensar, um simples xpmread() deve estourar a pilha! :)
	
	se o compilador reclamar muito com warnings, por favor
	envie uma copia do problema para marcelos@bsi.com.br.
	
	esses codigos sao altamente *beta*, provavelmente podem
	ser bastante melhorados e fornecem um bom exemplo de
	leitura e escrita de varios formatos, mas provavelmente
	nao eh o melhor exemplo nem o mais elaborado!
	
	recomendo boa sorte ao usar! :)

	ATENCAO ATENCAO ATENCAO ATENCAO ATENCAO ATENCAO ATENCAO

	RAWImage: 

	esta estrutura armazena uma imagem decodificada:
	
	width 	= largura em pixels da imagem
	height 	= altura em pixels da imagem
	line 	= matriz de ponteiros para linhas, de modo
		  que cada linha contem um buffer com o 
		  comprimento width*3. as linhas sao
		  individualmente acessiveis como:
		  
			  line[0] = primeira linha da imagem
			  line[1] = segunda linha da imagem
			  line[3] = terceira linha da imagem
			  ...
			  line[height-1] = ultima linha
		  
		  pixels individuais sao acessiveis:
		  
			  line[0][0] = pixel na posicao 0,0

		  a ordem de armazenamento utilizada eh BGR, e
		  nao RGB como se espera normalmente. isso acontece
		  pq os processadores intel transferem os numeros
		  dos registros ao contrario para a memoria, de 
		  modo que:
		  
		  	EAX=0x12345678
		  
		  eh transferido para a memoria como:
		  
		  	0x78563412
		  
		  assim:
		  
		  	EAX=0x00RRGGBB
		  	
		  eh armazenado:
		  
		  	0xBBGGRR00
		  
		  quando se faz a supressao do byte de pad, temos
		  uma sequencia de tres bytes, na ordem BGR. em 
		  plataforma nao-intel os numeros sao armazenados na
		  memoria na ordem q aparecem nos registros, entao
		  eh comum a sequencia RGB em plataformas nao-intel,
		  em particular padroes mais antigos sob plataforma
		  unix (isso explica pq todo o padrao TCP/IP exige
		  macros de inversao de bytes hoje em dia :)

	depth   = depth recomendado para a imagem, depende de
		  como a imagem eh lida, nos nao armazenamos uma
		  paleta de cor, mas armazenamos um tamanho q
		  indica a quantidade real de cores q foi puxada
		  na leitura original. atualmente isso nao 
		  esta importanto muito na escrita, mas pode dar
		  uma dica para a funcao de escrita de como
		  otimizar melhor o codigo!
*/

typedef struct {

	int 	width,
		height,
		depth;
		
	unsigned char **line;

} RAWImage;

/*
	pad() eh uma funcao que arredonda valores de
	modo que fiquem sempre multiplos de 32 bits, ou
	seja, ela adiciona os bytes necessarios a um
	offset para q ele fique completo e alinhado com
	um valor de 32 bits. alguns formatos de armazenamento
	sao criticos em relacao ao alinhamento interno e
	exigem q o comprimento das linhas seja alinhado com
	valores de 32 bits.
*/

int pad(int offset) {

	if(offset&3)	return (offset&(~3))+4;
	else 		return offset;
}

/*
	o antigo formato PCX eh bem simples: um header de 128 bytes
	indica como ler o resto da imagem. em geral as imagens sao
	comprimidas com compressao RLE. vamos estudar especificamente
	o caso de imagens comprimidas em RLE com 8 bits.
	
	o header PCX utiliza atributos para alinhar corretamente os
	dados e eliminar pads inseridos por otimizacoes do compilador,
	esses atributos sao especificos para o gcc.
*/

RAWImage *pcxread(FILE *fp) {

	struct {

		unsigned char 	manufacturer 	__attribute__ ((packed));
		unsigned char 	version		__attribute__ ((packed));	
		unsigned char 	encoder		__attribute__ ((packed));
		unsigned char 	depth		__attribute__ ((packed));
		unsigned short 	x		__attribute__ ((packed));
		unsigned short 	y		__attribute__ ((packed));
		unsigned short 	width		__attribute__ ((packed));
		unsigned short 	height		__attribute__ ((packed));
		unsigned short 	hdpi		__attribute__ ((packed));
		unsigned short 	vdpi		__attribute__ ((packed));
		unsigned char 	palette[48]	__attribute__ ((packed));
		unsigned char 	reserved	__attribute__ ((packed));
		unsigned char 	planes		__attribute__ ((packed));
		unsigned short 	bpl		__attribute__ ((packed));
		unsigned short  type		__attribute__ ((packed));
		unsigned char  	pad[58]		__attribute__ ((packed));
	
	} pcxinfo;

	RAWImage *tmp=NULL;

	int x,y,z,repeat;
	unsigned char *q,*buffer,data;

	struct {
		
		unsigned char r,g,b;
	
	} palette[256];

	fseek(fp,0,SEEK_SET);

	fread(&pcxinfo,sizeof(pcxinfo),1,fp);
	
	/*
		o arquivo PCX nao tem assinatura util, o que podemos
		fazer eh tentar achar uma consistencia no arquivo! vamos
		usar como parametro *bom* o encoder, o depth, width, 
		height e o valor bpl, em particular!
	*/

	fprintf(stderr,"check: PCX-v%d-%s-%d, %dx%d (%d bits, status=%s)\n",
		pcxinfo.version,
		pcxinfo.encoder?(pcxinfo.encoder==1?"RLE":"compressed"):"uncompressed",
		pcxinfo.encoder,
		pcxinfo.width,
		pcxinfo.height,
		pcxinfo.depth,
		pcxinfo.bpl == pad(pcxinfo.width*pcxinfo.depth/8)?"valid":"invalid");

	if(pcxinfo.bpl != pad(pcxinfo.width*pcxinfo.depth/8)) return tmp;
	if(pcxinfo.encoder>1) return tmp;
	if(pcxinfo.depth!=8)  return tmp;

	tmp=(RAWImage *)malloc(sizeof(RAWImage));
	tmp->line=(unsigned char **)malloc(pcxinfo.height*sizeof(char *));

	fseek(fp,-768,SEEK_END);

	fread(&palette,sizeof(palette),1,fp);

	fseek(fp,128,SEEK_SET);

	tmp->width	=pcxinfo.width;
	tmp->height	=pcxinfo.height;

	buffer=(unsigned char *)malloc(pcxinfo.bpl*(pcxinfo.height+1));

	x=0;
	
	while(x<pcxinfo.bpl*pcxinfo.height) {

		data=fgetc(fp);
		repeat=1;

		if(data>192) {
			
			repeat=data-192;
			data=fgetc(fp);
		}
			
		while(repeat--) buffer[x++] = data;
	}

	for(z=y=0;y!=pcxinfo.height;y++) {
	
		q=tmp->line[y]=(char *)malloc(pcxinfo.width*3);
	
		for(x=0;x!=pcxinfo.width;x++,z++) {

			*q++ = palette[buffer[z]].b;
			*q++ = palette[buffer[z]].g;
			*q++ = palette[buffer[z]].r;
		}
		
		z+=(pcxinfo.bpl-pcxinfo.width);
	}	

	free(buffer);

	fprintf(stderr,"*read: PCX-v%d-RLE, %dx%d (8 bit/pixel, compressor=RLE, status=OK)\n",
		pcxinfo.version,
		pcxinfo.width,
		pcxinfo.height);
	
	return tmp;
}

/*
	a familia Portable Pixmap tem formatos identificados
	por uma assinatura q varia de P1 a P6, sendo os formatos:
	
		P1	bitmap P&B ascii, 1 inteiro/pixel
		P2	grayscale ascii,  1 inteiro/pixel
		P3	colorido ascii, 3 inteiros/pixel
		P4	bitmap P&B, 8 pixels/byte binario
		P5	grayscale, 1 pixel/byte binario
		P6	colorico, 3 bytes/pixel binario RGB.

	os arquivos podem ser selecionados conforme a assinatura.

	os formatos P1, P2 e P3 sao codificados em ascii, os formatos
	P4, P5 e P6 sao codificados em binario, sendo P3 em ordem
	RGB (deve ser invertido para BGR).
*/

RAWImage *ppmread(FILE *fp) {

	unsigned char buffer[4096],*q;
	int type=0,x,y,z,max,r,g,b;
	
	RAWImage *tmp=NULL;

	fseek(fp,0,SEEK_SET);
	
	fgets(buffer,4096,fp);
	
	if(buffer[0]=='P') {

		type=buffer[1]-'0';

		while(fgets(buffer,4096,fp)) if(buffer[0]!='#') break;

		switch(type) {
	
			case 1:
				tmp=(RAWImage *)malloc(sizeof(RAWImage));
			
				sscanf(buffer,"%d %d",&tmp->width,&tmp->height);
			
				tmp->line=(unsigned char **)malloc(sizeof(char *)*tmp->height);
			
				for(y=0;y!=tmp->height;y++) {
				
					q=tmp->line[y]=(char *)malloc(tmp->width*3);
					
					for(x=0;x!=tmp->width;x++) {
				
						fscanf(fp,"%d",&z);
						z=z?0:255;
						
						*q++ = z;
						*q++ = z;
						*q++ = z;
					}
				}
				break;
			case 2:
				tmp=(RAWImage *)malloc(sizeof(RAWImage));
			
				sscanf(buffer,"%d %d",&tmp->width,&tmp->height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);
			
				tmp->line=(unsigned char **)malloc(sizeof(char *)*tmp->height);
			
				for(y=0;y!=tmp->height;y++) {
				
					q=tmp->line[y]=(char *)malloc(tmp->width*3);
					
					for(x=0;x!=tmp->width;x++) {
				
						fscanf(fp,"%d",&z);
						z=z*255/max;
						
						*q++ = z;
						*q++ = z;
						*q++ = z;
					}
				}
				break;
			case 3:
				tmp=(RAWImage *)malloc(sizeof(RAWImage));
			
				sscanf(buffer,"%d %d",&tmp->width,&tmp->height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);			

				tmp->line=(unsigned char **)malloc(sizeof(char *)*tmp->height);
			
				for(y=0;y!=tmp->height;y++) {
				
					q=tmp->line[y]=(char *)malloc(tmp->width*3);
					
					for(x=0;x!=tmp->width;x++) {
				
						fscanf(fp,"%d %d %d",&r,&g,&b);

						*q++ = b*255/max;
						*q++ = g*255/max;
						*q++ = r*255/max;
					}
				}
				break;
			case 4:
				tmp=(RAWImage *)malloc(sizeof(RAWImage));
			
				sscanf(buffer,"%d %d",&tmp->width,&tmp->height);
			
				tmp->line=(unsigned char **)malloc(sizeof(char *)*tmp->height);
			
				for(y=0;y!=tmp->height;y++) {
				
					q=tmp->line[y]=(char *)malloc(tmp->width*3);
					fread(buffer,tmp->width/8,1,fp);

					for(x=0;x!=tmp->width/8;x++) {
					
						for(z=0;z!=8;z++) {
						
							if(buffer[x]&(0x80>>z))
								*q++ = 0, 
								*q++ = 0, 
								*q++ = 0;
							else
								*q++ = 255,
								*q++ = 255,
								*q++ = 255;
						}
					}
				}
				break;
			case 5:
				tmp=(RAWImage *)malloc(sizeof(RAWImage));
			
				sscanf(buffer,"%d %d",&tmp->width,&tmp->height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);			

				tmp->line=(unsigned char **)malloc(sizeof(char *)*tmp->height);
			
				for(y=0;y!=tmp->height;y++) {
				
					q=tmp->line[y]=(char *)malloc(tmp->width*3);
					fread(buffer,tmp->width,1,fp);

					for(x=0;x!=tmp->width;x++) {
							
						z=buffer[x]*255/max;
						
						*q++ = z;
						*q++ = z;
						*q++ = z;
					}
				}
				break;
			case 6:
				tmp=(RAWImage *)malloc(sizeof(RAWImage));
			
				sscanf(buffer,"%d %d",&tmp->width,&tmp->height);
				
				fgets(buffer,4096,fp);
				sscanf(buffer,"%d",&max);

				tmp->line=(unsigned char **)malloc(sizeof(char *)*tmp->height);
			
				for(y=0;y!=tmp->height;y++) {
				
					q=tmp->line[y]=(char *)malloc(tmp->width*3);
					fread(buffer,tmp->width*3,1,fp);

					for(x=0;x!=tmp->width;x++) {
							
						r=buffer[x*3+0]*255/max;
						g=buffer[x*3+1]*255/max;
						b=buffer[x*3+2]*255/max;
						
						*q++ = b;
						*q++ = g;
						*q++ = r;
					}
				}
				break;
		}
	}
		
	if(tmp) 
		fprintf(stderr,"*read: PPM-P%d, %dx%d pixels\n",
			type,tmp->width,tmp->height);
	else
		fprintf(stderr,"check: PPM signature status=failed (%s)\n",
			type?"invalid PPM type":"invalid signature");
	
	return tmp;
}

/*
	bmpread() testa e le uma imagem bmp. se nao tiver
	sucesso, retorna NULL. se tiver sucesso retorna um
	descritor valido para uma imagem decodificada de 
	24 bits.
*/

RAWImage *bmpread(FILE *fp) {

	RAWImage *tmp; 

	/*
		bmpinfo eh um header de arquivo BMP. o header do
		arquivo eh lido aqui e seus dados verificados. se o
		header nao apresentar consistencia, a decodificacao
		da imagem eh abortada e retornado NULL.
		
		os atributos "packed" sao necessarios para o gcc
		nao colocar pads de alinhamento 32 bits entre os shorts
		de 16 bits e os ints de 32 bits!
	*/

	struct {
	
		unsigned short 	type		__attribute__ ((packed));
		unsigned int 	filesize	__attribute__ ((packed));
		unsigned short  reserved[2]	__attribute__ ((packed));
		unsigned int	offset		__attribute__ ((packed));
		unsigned int	header		__attribute__ ((packed));	
		unsigned int	width		__attribute__ ((packed));
		unsigned int	height		__attribute__ ((packed));	
		unsigned short	planes		__attribute__ ((packed));
		unsigned short	depth		__attribute__ ((packed));
		unsigned int 	format		__attribute__ ((packed));
		unsigned int	compsize	__attribute__ ((packed));
		unsigned int	hres		__attribute__ ((packed));
		unsigned int 	vres		__attribute__ ((packed));
		unsigned int	colors		__attribute__ ((packed));
		unsigned int	realcolors	__attribute__ ((packed));
		
	} bmpinfo;
	
	unsigned 	char 	*data,*p,*q;
	int  		size,x,y,z;

	struct {
	
		unsigned char r,g,b,z;
	
	} palette[256];

	fseek(fp,0,SEEK_SET);
	
	/*
		le o header da imagem BMP e verifica se a 
		assinatura eh valida. 'BM' nao eh uma string,
		mas um short de 16 bits composto pelos 
		caracteres 'B' e 'M'. o gcc aceita composicoes
		numericas dessa forma com ateh 32 bits de
		comprimento (4 caracteres).
		
		se tiver sucesso, imprime algumas informacoes
		sobre a imagem. a consistencia inclui tambem uma
		verificacao de limite de tamanho da imagem, o
		teste eh um tamanho&(~0xfff), ou seja, eh feita uma
		mascara AND com ~0xfff, se o valor for maior que
		4096 ou menor q 0, a expressao se torna diferente
		de zero e, portanto, verdadeira.
		
		isso limita o tamanho maximo de imagens a 4096x4096
		pixels e um buffer de imagem de no maximo 50MB. se
		necessario eh possivel ampliar esse limite para 
		32768x32768, mas o buffer de imagem maximo aumentaria
		para 3GB, bem fora da realidade de memoria da maioria
		dos usuarios. assim, 4096x4096 pixels parece um
		limite bem realista.
	*/
	
	fread(&bmpinfo,sizeof(bmpinfo),1,fp);

	fprintf(stderr,"check: BMP-%s, signature is '%u' (espected %u, status=%s)\n",
		bmpinfo.format?"compressed":"uncompressed",
		bmpinfo.type,
		((unsigned short)'B'<<8)+'M',
		(bmpinfo.type!=('B'<<8)+'M')?"sucess":"failed");
	
	if(bmpinfo.type==('B'<<8)+'M') 	return NULL;
	if(bmpinfo.width&(~0xfff))	return NULL;
	if(bmpinfo.height&(~0xfff)) 	return NULL;
	if(bmpinfo.format)		return NULL;
	if(bmpinfo.planes!=1)		return NULL;
	
	fprintf(stderr,"*read: BMP-%s, size=%dx%d, %d bits (offset = %d bytes)\n",
		bmpinfo.format?"compressed":"uncompressed",
		bmpinfo.width,
		bmpinfo.height,
		bmpinfo.depth,
		bmpinfo.offset);

	/*
		vamos ler os dados. note q o BMP tem a largura
		em bits definido pelo depth da imagem. essa largura
		eh passada para bytes e ajustado para um alinhamento
		de 32 bits com a funcao PAD.
		
		depois da leitura eh possivel inclusive verificar
		se todos os bytes foram corretamente lidos conforme
		indica o header do arquivo. em geral arquivos 
		corrompidos nao causam grandes problemas, mas se
		forem arquivos com compressao, a operacao deve ser
		abortada para evitar q o descompressor cause erros
		fatais na aplicacao!
	*/

	if(bmpinfo.depth!=24) fread(&palette,(1<<bmpinfo.depth)*4,1,fp);

	size=pad(bmpinfo.width*bmpinfo.depth/8)*bmpinfo.height;
	data=(char *)malloc(size);
	fread(data,size,1,fp);

	if(bmpinfo.filesize != ftell(fp))
		fprintf(stderr,"warning! BMP image truncated: %d bytes expected, %ld bytes read\n",
			bmpinfo.filesize,
			ftell(fp));

	tmp=(RAWImage *)malloc(sizeof(RAWImage));

	tmp->width	=bmpinfo.width;
	tmp->height	=bmpinfo.height;
	tmp->depth	=bmpinfo.depth;

	/*
		imagens BMP tem as linhas armazenadas ao contrario,
		de modo q a imagem fica de cabeca para baixo! eh necessario
		colocar a imagem de uma forma ordenada para q possa ser
		depois codificada em outros formatos!
			
		para isso vamos ler os dados de 'data' e gravar em 
		'tmp', em um buffer de linhas na ordem correta.
			
		com depth 24, podemos simplesmente ler direto a imagem
		sem se preocupar com a conversao. com depth 8 temos q
		converter a imagem para 24 bits usando a palette q indexa
		as cores. com depths menores, as coisas podem ser um
		pouco mais complicadas ainda :P
		
		com depth 4, temos q ler os nibbles de um byte e 
		indexar na palette para ter a cor correta. com depth 1
		temos q ler cada bit individual. nesse caso escolhemos
		representar o bit 1 pela cor 255,255,255 e o bit
		0 pela cor 0,0,0 na escala RGB.
	*/

	p=data;

	tmp->line=(unsigned char **)malloc(bmpinfo.height*sizeof(char **));

	for(y=bmpinfo.height-1;y!=-1;y--) {
		
		tmp->line[y]=(unsigned char *)malloc(bmpinfo.width*3);

		if(bmpinfo.depth==24) {

			q=tmp->line[y];

			for(x=0;x!=bmpinfo.width*3;x++) {
			
				q[x]=p[x];
			}
			
			p+=pad(bmpinfo.width*3);
		}
		
		if(bmpinfo.depth==8) {
		
			q=tmp->line[y];
		
			for(x=0;x!=bmpinfo.width;x++) {
			
				*q++=palette[p[x]].r;
				*q++=palette[p[x]].g;
				*q++=palette[p[x]].b;
			}
			
			p+=pad(bmpinfo.width);
		}
		
		if(bmpinfo.depth==4) {
		
			q=tmp->line[y];
			
			for(x=0;x!=bmpinfo.width;x++) {
			
				*q++=palette[x%2?p[x/2]&0xf:p[x/2]>>4].r;
				*q++=palette[x%2?p[x/2]&0xf:p[x/2]>>4].g;
				*q++=palette[x%2?p[x/2]&0xf:p[x/2]>>4].b;
			}
			
			p+=pad(bmpinfo.width/2);
		}

		if(bmpinfo.depth==1) {
		
			q=tmp->line[y];
			
			for(x=0;x!=bmpinfo.width/8;x++) {

				for(z=0;z!=8;z++) {

					*q++=(p[x]&(0x80>>z))?255:0;
					*q++=(p[x]&(0x80>>z))?255:0;
					*q++=(p[x]&(0x80>>z))?255:0;
				}
			}
			
			p+=pad(bmpinfo.width/8);
		}
	}
		
	free(data);
		
	return tmp;
}

/*
	X pixmap consiste eh um arquivo codificado em C, eh necessario
	separar os dados do codigo interno, em principio, tudo q estiver
	entre "" deve ser processado. o formato simplesmente indica
	o inicio com resolucao, quantidade de cores e caracteres por pixel,
	segue uma paleta de cores e entao a imagem. nao existe uma
	assinatura identificavel, mas o arquivo eh bem caracterizado por
	existir um em uma linha logo no inicio!
	
	inicialmente vamos procurar a assinatura valida e entao 
	analizar as linhas e armazenar o que estiver entre aspas, entao a 
	decodificacao que segue eh bastante simples.
*/

RAWImage *xpmread(FILE *fp) {

	char buffer[4096],*p,*q,index[16],color[16];
	unsigned short *s;

	RAWImage *tmp=NULL;

	int status=0,width,height,colors,type,palette[65536],x=0,y=0;

	/*
		vamos ler a coisa aqui dentro.
		
		inicialmente procuramos a assinatura, q eh a
		string 'char'. devemos procurar pulando comentarios
		e coisas do genero. se existir, admitimos que 
		o arquivo eh valido, do contrario, nao iniciamos
		a procura por nada.
		
		depois disso procuramos os dados de forma ordenada, 
		qq variacao de construcao indica um arquivo corrompido
		e a leitura deve ser abortada.
		
		os dados estao sempre entre aspas. inicialmente
		precisamos procurar 4 numeros, para descobrir
		width, height, color-number e xpm-type. com isso
		lemos uma sequencia de linhas com a paleta e
		outra sequencia com a imagem.
	*/

	fseek(fp,0,SEEK_SET);

	while(fgets(buffer,4096,fp)) {

		if(status&&status!=4) {
	
			p=strchr(buffer,'\"');
			
			if(!p) 	continue;
			else 	p++;
			
			q=strchr(p,'\"');
			*q=0;
							
			switch(status) {
			
				case 1:
					sscanf(p,"%d %d %d %d",
						&width,&height,&colors,&type);
					
					tmp=(RAWImage *)malloc(sizeof(RAWImage));
					tmp->width=width;
					tmp->height=height;
					tmp->line=(unsigned char **)malloc(sizeof(char *)*height);
					
					status=2;
					break;
				case 2:
					sscanf(p,"%s c %s",index,color);
					
					if(type==2)
						palette[*((unsigned short *)index)]=strtol(color+1,NULL,16);
					if(type==1)
						palette[*((unsigned char *)index)]=strtol(color+1,NULL,16);
					x++;	
										
					if(x==colors) status=3;
					break;
				case 3:
					q=tmp->line[y]=(char *)malloc(width*3);
					s=(unsigned short *)p;
				
					for(x=0;x!=width;x++) {
					
						if(type==2) {
						
							*q++ = palette[*s]&0xff;
							*q++ = (palette[*s]>>8)&0xff;
							*q++ = palette[*s]>>16;
						
							s++;
						}
						
						if(type==1) {
						
							*q++ = palette[(unsigned)(*p)]&0xff;
							*q++ = (palette[(unsigned)(*p)]>>8)&0xff;
							*q++ = palette[(unsigned)(*p)]>>16;
							
							p++;
						}
					}
					
					y++;
					
					if(y==height) status=4;
					break;
			}
		
		} else 
			if(strstr(buffer,"char")) 
				status=1;
	}

	if(status==4) {

		fprintf(stderr,"*read: XPM-%d: %dx%d pixels, %d colors\n",
			type,width,height,colors);
	
		return tmp;

	} else { 

		fprintf(stderr,"check: XPM-%d: %dx%d pixels, %d colors (failed in stage %d)\n",
			type,width,height,colors,status);

		return NULL;
	}
}

/*
	gravacao BMP, pode ser otimizada futuramente para
	aproveitar melhor a informacao de depth previo do 
	arquivo e usar um depth de gravacao mais otimizado, que
	economiza armazenamento. por hora, as imagens sao gravadas
	em 24 bits, garantindo integridade da imagem.
*/

void bmpwrite(FILE *fp,RAWImage *tmp) {

	struct {
	
		unsigned short 	type		__attribute__ ((packed));
		unsigned int 	filesize	__attribute__ ((packed));
		unsigned short  reserved[2]	__attribute__ ((packed));
		unsigned int	offset		__attribute__ ((packed));
		unsigned int	header		__attribute__ ((packed));	
		unsigned int	width		__attribute__ ((packed));
		unsigned int	height		__attribute__ ((packed));	
		unsigned short	planes		__attribute__ ((packed));
		unsigned short	depth		__attribute__ ((packed));
		unsigned int 	format		__attribute__ ((packed));
		unsigned int	compsize	__attribute__ ((packed));
		unsigned int	hres		__attribute__ ((packed));
		unsigned int 	vres		__attribute__ ((packed));
		unsigned int	colors		__attribute__ ((packed));
		unsigned int	realcolors	__attribute__ ((packed));
		
	} bmpinfo;

	int i;

	/*
		a gravacao BMP eh simples, consiste em gravar a imagem
		24 bits diretamente. o maior problema no caso eh inverter
		a imagem, mas RAWImage jah vem com linhas enderecaveis
		individualmente, entao a coisa se torna absurdamente 
		*simples*;
	*/

	bmpinfo.type		=('M'<<8)+'B';
	bmpinfo.filesize	=pad(tmp->width*3)*tmp->height+sizeof(bmpinfo);
	bmpinfo.offset		=sizeof(bmpinfo);
	bmpinfo.header		=40;
	bmpinfo.width		=tmp->width;
	bmpinfo.height		=tmp->height;
	bmpinfo.planes		=1;
	bmpinfo.depth		=24;
	bmpinfo.format		=0;
	bmpinfo.compsize	=0;
	bmpinfo.colors		=16777216;
	bmpinfo.realcolors	=bmpinfo.colors;

	fwrite(&bmpinfo,sizeof(bmpinfo),1,fp);

	for(i=bmpinfo.height-1;i!=-1;i--)
		fwrite(tmp->line[i],pad(tmp->width*3),1,fp);

	fprintf(stderr,"write: BMP-%s, size=%dx%d, %d bits (offset = %d bytes)\n",
		bmpinfo.format?"compressed":"uncompressed",
		bmpinfo.width,
		bmpinfo.height,
		bmpinfo.depth,
		bmpinfo.offset);
}

/*
	o portable bitmap PBM eh um ppm tipo P4, com 8 pixels
	por byte. para a imagem ficar legal, nos geramos um bitmap
	fazendo difusao por erro, assim a imagem de 24 bits fica
	bem bonita com 1 bit :)
	
	a difusao calcula os pixels em cinza (R+G+B)/3, q dah 
	um valor entre 0 e 255. como soh podemos usar 0 ou
	255, a diferenca q falta eh acumulada como erro. o erro
	eh somado na proxima media (R+B+G)/3+erro, assim, 
	o erro vai aumentando ateh interferir na imagem e se
	dissipar. isso faz a imagem nao ser continua, mas variar
	a frequencia dos pixels!
*/

void pbmwrite(FILE *fp,RAWImage *tmp) {

	int x,y,z,pixel,error=0;
	unsigned char *q,*p;
	
	fprintf(fp,"P4\n%d %d\n",tmp->width,tmp->height);

	p=(char *)malloc(tmp->width/8);

	for(y=0;y!=tmp->height;y++) {

		q=tmp->line[y];

		for(x=0;x!=tmp->width/8;x++)

			for(z=0;z!=8;z++) {
			
				pixel = (*q++ + *q++ + *q++)/3+error;
				
				error=pixel>127?pixel-255:pixel-0;
				
				if(pixel>127) 
					p[x]&=~(0x80>>z);
				else
					p[x]|= 0x80>>z;
			}

		fwrite(p,tmp->width/8,1,fp);
	}
	
	free(p);

	fprintf(stderr,"write: PBM-P4, size=%dx%d (B&W)\n",
		tmp->width,
		tmp->height);
}

/*
	portable grayscale pixmap (PGM), eh um ppm P5,
	no seu codigo tem um PGM P5, bom, eu nao sei se eh
	a mesma especificacao, mas tou seguindo os docs
	q eu tenho de ppm.
	
	tem um ppm P2 q eh o PGM em ASCII, o P5 eh 
	binario, mais veloz. basicamente, para a gravacao
	ser veloz, nos convertemos para grayscale em um
	buffer e fazemos um fwrite() dele, linha a linha :)
*/

void pgmwrite(FILE *fp,RAWImage *tmp) {

	int x,y;
	unsigned char *q,*p;
	
	fprintf(fp,"P5\n%d %d\n%d\n",tmp->width,tmp->height,255);

	p=(char *)malloc(tmp->width);

	for(y=0;y!=tmp->height;y++) {

		q=tmp->line[y];

		for(x=0;x!=tmp->width;x++)
			p[x] =(*q++ + *q++ + *q++)/3;

		fwrite(p,tmp->width,1,fp);
	}
	
	free(p);

	fprintf(stderr,"write: PGM-P5, size=%dx%d (grayscale)\n",
		tmp->width,
		tmp->height);
}

/*
	portable pixmap (PPM), eh um ppm P6 colorido. tem um
	P3 em ASCII, como no PGM, esse P6 eh binario, a ideia
	eh bem simples! apenas escreve as linhas no arquivo,
	em RGB...mas tem q tomar cuidado! existe diferenca entre
	big e litle endian, entao tem q inverter RGB para 
	BGR! :)
*/         

void ppmwrite(FILE *fp,RAWImage *tmp) {

	int x,y;
	unsigned char *q,*p;
	
	fprintf(fp,"P6\n%d %d\n%d\n",tmp->width,tmp->height,255);

	p=(char *)malloc(tmp->width*3);

	for(y=0;y!=tmp->height;y++) {

		q=tmp->line[y];

		for(x=0;x!=tmp->width;x++) {
			
			p[x*3+0]=q[x*3+2];
			p[x*3+1]=q[x*3+1];
			p[x*3+2]=q[x*3+0];
		}
		fwrite(p,tmp->width*3,1,fp);
	}
	
	free(p);

	fprintf(stderr,"write: PPM-P6, size=%dx%d (color)\n",
		tmp->width,
		tmp->height);
}

/*
	o formato xpm na realidade foi criado para permitir
	a inclusao de imagens dentro de codigo C para X de modo
	q as imagens possam ser compiladas dentro da aplicacao,
	para tanto, basta seguir uma especificacao minima
	de formato.
	
	infelizmente o formato xpm tem capacidade de trabalhar
	com poucas cores, entao escolhemos fazer difusao por erro
	e trabalhar com formato 1, com 64 cores, de modo que o
	arquivo fica bem mais compacto que no formato 2.
	
	a difusao por erro procura a cor mais proxima em uma 
	palette pequena, por exemplo, 64 cores representando um
	total de 16 milhoes. como a probabilidade de encontrar
	a cor certa eh baixa, a cor mais proxima eh utilizada e
	a diferenca eh acumulada como um erro, q eh utilizado 
	no proximo pixel para compensar a imprecisao anterior.
	
	isso cria flutuacoes na frequencia das cores q permite ao
	conjunto otico manter a intensidade media total, de modo
	q apenas 64 cores equivalem as 16 milhoes de cores.
*/

void xpmwrite(FILE *fp,RAWImage *tmp) {

	int x,y,z,r,g,b;
	unsigned char *q;
	unsigned char *map=".#abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	
	fprintf(fp,"/* XPM */\n");
	fprintf(fp,"static char * image_xpm[] = {\n");
	fprintf(fp,"\"%d %d 4096 2\",\n",tmp->width,tmp->height);

	/*
		z = 0 a 4095 (12 bits), de modo que temos:
		
			z = 0000rrrrggggbbbb
		
		assim:
		
			0000rrrrggggbbbb >> 8 & 0xf
			000000000000rrrr
			
			0000rrrrggggbbbb >> 4 & 0xf
			000000000000gggg
			
			0000rrrrggggbbbb & 0xf
			000000000000bbbb
		
		basicamente vamos trabalhar com 12 bits de cor,
		fazendo uma reducao direta de 24 para 12 bits.		
	*/

	for(z=x=0;x!=64;x++)
		for(y=0;y!=64;y++,z++)
			fprintf(fp,"\"%c%c c #%02x%02x%02x\",\n",
				map[y],map[x],
				((z>>8)&0xf)*16,
				((z>>4)&0xf)*16,
				((z&0xf)*16));

	for(y=0;y!=tmp->height;y++) {

		q=tmp->line[y];

		fprintf(fp,"\"");

		for(x=0;x!=tmp->width;x++) {
		
			b=(*q++)&0xf0;
			g=(*q++)&0xf0;
			r=(*q++)&0xf0;
		
			fprintf(fp,"%c%c",
				map[(r<<4|g|b>>4)&63],
				map[(r<<4|g|b>>4)>>6]);
		}
			
		fprintf(fp,"\",\n");
	}

	fprintf(fp,"};\n");
	
	fprintf(stderr,"write: XPM/2, size=%dx%d (4096 colors)\n",
		tmp->width,
		tmp->height);
}


/*
	a gravacao em postscript consiste apenas de dois
	conjuntos padroes de codigo q processam um conjunto
	de dados, q eh a imagem de 24 bits RGB.

	o codigo de auxilio para processamento postscript eh
	baseado em impressoes postscript de aplicativos de
	edicao de imagem. a imagem eh centralizada em uma folha
	de papel padrao carta 8.5" x 11".
*/

void epswrite(FILE *fp,RAWImage *tmp) {

	float	x_scale,	
		y_scale,	
		x_index,	
		y_index,
		scale;

	int x,y;
	
	unsigned char *z;

	/*
		calculos para dimensionar e centralizar a 
		imagem no papel. as dimensoes sao todas medidas
		em 'pontos tipograficos', q equivale a 1/72 
		de polegada.
	*/

	scale=468.0/tmp->width;

	x_scale=tmp->width*scale;
	y_scale=tmp->height*scale;

	x_index=72.0;
	y_index=356.0-y_scale/2+72;

	fprintf(fp,"%%!PS-Adobe-2.0 EPSF-2.0\n");
	fprintf(fp,"%%%%Based on XV postscript output, from John Bradley\n");
	fprintf(fp,"%%%%EndComments\n");
	fprintf(fp,"%%%%EndProlog\n");
	fprintf(fp,"%% remember original state\n");
	fprintf(fp,"/origstate save def\n");
	fprintf(fp,"%% build a temporary dictionary\n");
	fprintf(fp,"20 dict begin\n");
	fprintf(fp,"%% define string to hold a scanline's worth of data\n");

	fprintf(fp,"/pix %d string def\n",
		tmp->width*3);

	fprintf(fp,"%% define space for color conversions\n");
	
	fprintf(fp,"/grays %d string def  %% space for gray scale line\n",
		tmp->width);

	fprintf(fp,"/npixls 0 def\n");
	fprintf(fp,"/rgbindx 0 def\n");
	fprintf(fp,"%% lower left corner\n");
	
	fprintf(fp,"%f %f translate\n",
		x_index,
		y_index);
	
	fprintf(fp,"%% size of image (on paper, in 1/72inch coords)\n");

	fprintf(fp,"%f %f scale\n",
		x_scale,
		y_scale);

	fprintf(fp,"%% define 'colorimage' if it isn't defined\n");
	fprintf(fp,"%%   ('colortogray' and 'mergeprocs' come from xwd2ps\n");
	fprintf(fp,"%%     via xgrab)\n");
	fprintf(fp,"/colorimage where   %% do we know about 'colorimage'?\n");
	fprintf(fp,"  { pop }           %% yes: pop off the 'dict' returned\n");
	fprintf(fp,"  {                 %% no:  define one\n");
	fprintf(fp,"    /colortogray {  %% define an RGB->I function\n");
	fprintf(fp,"      /rgbdata exch store    %% call input 'rgbdata'\n");
	fprintf(fp,"      rgbdata length 3 idiv\n");
	fprintf(fp,"      /npixls exch store\n");
	fprintf(fp,"      /rgbindx 0 store\n");
	fprintf(fp,"      0 1 npixls 1 sub {\n");
	fprintf(fp,"        grays exch\n");
	fprintf(fp,"        rgbdata rgbindx       get 20 mul    %% Red\n");
	fprintf(fp,"        rgbdata rgbindx 1 add get 32 mul    %% Green\n");
	fprintf(fp,"        rgbdata rgbindx 2 add get 12 mul    %% Blue\n");
	fprintf(fp,"        add add 64 idiv      %% I = .5G + .31R + .18B\n");
	fprintf(fp,"        put\n");
	fprintf(fp,"        /rgbindx rgbindx 3 add store\n");
	fprintf(fp,"      } for\n");
	fprintf(fp,"      grays 0 npixls getinterval\n");
	fprintf(fp,"    } bind def\n");
	fprintf(fp,"    %% Utility procedure for colorimage operator.\n");
	fprintf(fp,"    %% This procedure takes two procedures off the\n");
	fprintf(fp,"    %% stack and merges them into a single procedure.\n");
	fprintf(fp,"    /mergeprocs { %% def\n");
	fprintf(fp,"      dup length\n");
	fprintf(fp,"      3 -1 roll\n");
	fprintf(fp,"      dup\n");
	fprintf(fp,"      length\n");
	fprintf(fp,"      dup\n");
	fprintf(fp,"      5 1 roll\n");
	fprintf(fp,"      3 -1 roll\n");
	fprintf(fp,"      add\n");
	fprintf(fp,"      array cvx\n");
	fprintf(fp,"      dup\n");
	fprintf(fp,"      3 -1 roll\n");
	fprintf(fp,"      0 exch\n");
	fprintf(fp,"      putinterval\n");
	fprintf(fp,"      dup\n");
	fprintf(fp,"      4 2 roll\n");
	fprintf(fp,"      putinterval\n");
	fprintf(fp,"    } bind def\n");
	fprintf(fp,"    /colorimage { %% def\n");
	fprintf(fp,"      pop pop     %% remove 'false 3' operands\n");
	fprintf(fp,"      {colortogray} mergeprocs\n");
	fprintf(fp,"      image\n");
	fprintf(fp,"    } bind def\n");
	fprintf(fp,"  } ifelse          %% end of 'false' case\n");

	fprintf(fp,"%d %d 8			%% dimensions of data\n",
		tmp->width,
		tmp->height);

	fprintf(fp,"[%d 0 0 -%d 0 %d]		%% mapping matrix\n",
		tmp->width,
		tmp->height,
		tmp->height);

	fprintf(fp,"{currentfile pix readhexstring pop}\n");
	fprintf(fp,"false 3 colorimage\n");
	fprintf(fp,"%% 24 bit image in hex\n");

	for(y=0;y!=tmp->height;y++) {
	
		z=tmp->line[y];
	
		for(x=0;x!=tmp->width;x++) {

			if(y==tmp->height-1 && x==tmp->width-1) break;
			if(x%12==0) fprintf(fp,"\n");

			fprintf(fp,
				"%02x%02x%02x",
					*z++,
					*z++,
					*z++);
		}
	}

	fprintf(fp,"showpage\n");
	fprintf(fp,"%% stop using temporary dictionary\n");
	fprintf(fp,"end\n");
	fprintf(fp,"%% restore original state\n");
	fprintf(fp,"origstate restore\n");
	fprintf(fp,"%%%%Trailer\n");

	fprintf(stderr,"write: PostScript-2/ASCII, image size %dx%d pixels, 24bit RGB\n",
		tmp->width,
		tmp->height);
}

int main(int argc,char **argv) {

	RAWImage 	*raw=NULL;
	FILE 		*fp;

	int 		status=0;

	/*
		verifica os argumentos.
	*/
	
	if(argc==1) {
	
		fprintf(stderr,"error! please use:\n\n");
		fprintf(stderr,"  %s source destination\n\n",argv[0]);

		fprintf(stderr,"supported sources:\n\n");
		fprintf(stderr,"  .pcx: 8 bit RLE-compressed PCX\n");
		fprintf(stderr,"  .bmp: 1/4/8/24 bit BMP-uncompressed\n");
 		fprintf(stderr,"  .ppm: 24 bit Portable Pixmap (PPM-P6/P3)\n");
 		fprintf(stderr,"  .pgm: 8 bit Grayscale Portable Pixmap (PPM-P5/P2)\n");
 		fprintf(stderr,"  .pbm: 1 bit B&W Portable Pixmap (PPM-P4/P1)\n");
		fprintf(stderr,"  .xpm: 1-12 bit X Pixmap (XPM-1/2)\n\n");
		
		fprintf(stderr,"supported destionations:\n\n");
		fprintf(stderr,"  .bmp: 24 bit BMP-uncompressed\n");
		fprintf(stderr,"  .eps: 24 bit Postscript (PS2/ASCII)\n");
 		fprintf(stderr,"  .ppm: 24 bit Portable Pixmap (PPM-P6)\n");
 		fprintf(stderr,"  .pgm: 8 bit Grayscale Portable Pixmap (PPM-P5)\n");
 		fprintf(stderr,"  .pbm: 1 bit B&W Portable Pixmap (PPM-P4)\n");
 		fprintf(stderr,"  .xpm: 12 bit X Pixmap (XPM-2)\n\n");
 
		return -1;
	}

	/*
		abre o arquivo e sucessivamente testa o arquivo para
		encontrar uma assinatura valida e dados consistentes. se
		nada for encontrado, aborta o estagio de escrita.
	*/

	if(!(fp=fopen(argv[1],"r"))) {
	
		fprintf(stderr,"error! can't open '%s'\n",argv[1]);
		return -1;
	}

	/*
		estas 4 opcoes de checagem verificam assinaturas 
		validas, sendo que:
		
			pcxread() verifica 1 format valido de PCX 8 bits em RLE
			bmpread() verifica 4 formatos validos de BMP (1, 4, 8 e 24 bits)
			ppmread() verifica 6 formatos validos de PPM (ascii/bin, com 1, 8 e 24 bits)
			xpmread() verifica 2 formatos validos de XPM (6 e 12 bits)
	*/

	if(!raw) raw=pcxread(fp);
	if(!raw) raw=bmpread(fp);
	if(!raw) raw=xpmread(fp);
	if(!raw) raw=ppmread(fp);

	fclose(fp);

	if(!raw) {
	
		fprintf(stderr,"error! can't read image %s, aborting...\n",
			argv[1]);
			
		return -1;

	} 

	if(argv[2]) {

		fp=fopen(argv[2],"w");
			
		if(!fp) {
			fprintf(stderr,"error! can't open '%s'\n",argv[2]);
			return -1;
		}

		/*
			a gravacao eh feita conforme a extensao, nesse
			caso temos apenas 7 formatos de escrita.
			
			comparativamente, temos, ateh o momento:
			
				BMP: 4 formatos RD vs 1 formato WR
				PPM: 6 formatos RD vs 3 formatos WR
				XPM: 2 formatos RD vs 1 formato WR
				EPS: 1 formato WR
				PCX: 1 formato RD
				
			total de formatos legiveis eh 13, formatos
			gerados eh 6.
		*/

		if(strstr(argv[2],".bmp"))  bmpwrite(fp,raw),status=1;
		if(strstr(argv[2],".eps"))  epswrite(fp,raw),status=1;
		if(strstr(argv[2],".pgm"))  pgmwrite(fp,raw),status=1;
		if(strstr(argv[2],".ppm"))  ppmwrite(fp,raw),status=1;
		if(strstr(argv[2],".pbm"))  pbmwrite(fp,raw),status=1;
		if(strstr(argv[2],".xpm"))  xpmwrite(fp,raw),status=1;

		fclose(fp);

		if(status) 
			fprintf(stderr,"wow! sucess! converted image [%s] to [%s]!\n\n",
				argv[1],argv[2]);
		else
			fprintf(stderr,"failed! can't write '%s', unknow format\n",
				argv[2]);			
	}

	return 0;
}
