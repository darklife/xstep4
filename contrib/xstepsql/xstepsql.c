#include <stdio.h>
#include <stdlib.h>
#include <xstep.h>
#include <mysql.h>

/*
	variaveis diversas
*/

XSWidget	*DBWid,
		*TBWid,
		*DSWid,
		*DTWid;

/*
	funcoes superpoderosas do mysql! podem eventualmente
	ser substituidas facilmente para trabalhar com outros
	tipos de bancos de dados.
*/

MYSQL	mysql;

void SQLConnect(char *host,char *user,char *passwd) {

	mysql_init(&mysql);
	mysql_real_connect(&mysql,host,user,passwd,"mysql",0,NULL,0);
}

char ***SQLQuery(char *query,int *cols,int *rows) {

	MYSQL_RES	*res;
	MYSQL_ROW	row;

	int 		x,y;
		
	char ***list;

	fprintf(stderr,"%s: ",query);

	mysql_query(&mysql,query);	
	res=mysql_store_result(&mysql);

	fprintf(stderr,"%p\n",res);

	if(!res) return NULL;

	*rows=mysql_num_rows(res);
	*cols=mysql_num_fields(res);

	list=(char ***)malloc(*cols*sizeof(char *));

	for(x=0;x!=*cols;x++) {
	
		list[x]=(char **)malloc(*rows*sizeof(char *));
	}

	for(y=0;y!=*rows;y++) {
	
		row=mysql_fetch_row(res);
		
		for(x=0;x!=*cols;x++) {
		
			list[x][y]=(char *)malloc(strlen(row[x]));
			strcpy(list[x][y],row[x]);
			
			printf("  res[%d][%d]=%s\n",x,y,list[x][y]);
		}
	}

	mysql_free_result(res);

	return list;
}

void SQLFree(char ***list,int cols,int rows) {

	int x,y;

	if(!list) return;
	
	for(x=0;x!=cols;x++) {
	
		for(y=0;y!=rows;y++) free(list[x][y]);
		
		free(list[x]);
	}

	free(list);
}

void SQLUpdate(XSWidget *wid) {

	static int DBline=0,TBline=0,DSline=0,DTline=0;
	
	char query[256];

	if(DBline!=DBWid->line) {

		DBline=DBWid->line;
		SQLFree(TBWid->matrix,TBWid->cols,TBWid->rows);

		sprintf(query,"use %s",DBWid->matrix[0][DBline]);

			SQLQuery(query,&DBWid->cols,&DBWid->rows);
		TBWid->matrix=SQLQuery("show tables",&TBWid->cols,&TBWid->rows);
		TBWid->line=0;

		TBline=TBWid->line;

		SQLFree(DSWid->matrix,DSWid->cols,DSWid->rows);
		SQLFree(DTWid->matrix,DTWid->cols,DTWid->rows);
	
		sprintf(query,"describe %s",TBWid->matrix[0][TBline]);
				
		       SQLQuery(query,&DSWid->cols,&DSWid->rows);	
		DSWid->matrix=SQLQuery("show tables",&DSWid->cols,&DSWid->rows);
		DSWid->line=0;
	
		sprintf(query,"select * from %s",TBWid->matrix[0][TBline]);
		
		       SQLQuery(query,&DTWid->cols,&DTWid->rows);	
		DTWid->matrix=SQLQuery("show tables",&DTWid->cols,&DTWid->rows);
		DTWid->line=0;
	}
}

int main(int argc,char **argv) {

	XSWidget	*desktop,
			*window;

	/*
		conecta na base de dados
	*/

	SQLConnect("localhost","root","");

	desktop=XSDesktop(getenv("DISPLAY"),argv,argc);
	window=XSWindow(desktop,0,0,800+32,400+56,"xstepsql");
	window->on.change=SQLUpdate;

	/*
		XSTextScroll() pode vir a mudar na versao 4.0 e 
		retornar o viewport diretamente. cuidado!
	*/

	DBWid=(XSTextScroll(window,8,8,200,200,0,0,NULL))->viewport;
	TBWid=(XSTextScroll(window,8+200+8,8,200,200,0,0,NULL))->viewport;
	DSWid=(XSTextScroll(window,8+400+8+8,8,400,200,0,0,NULL))->viewport;
	DTWid=(XSTextScroll(window,8,8+200+8,-8,200,0,0,NULL))->viewport;

	DBWid->matrix=SQLQuery("show databases",	&DBWid->cols,&DBWid->rows);
	TBWid->matrix=SQLQuery("show tables",		&TBWid->cols,&TBWid->rows);
	DSWid->matrix=SQLQuery("describe host",		&DSWid->cols,&DSWid->rows);
	DTWid->matrix=SQLQuery("select * from user",	&DTWid->cols,&DTWid->rows);

	XSButton(window,-8,-8,72,24,"Quit",XSWindowClose);

	while(XSCheckEvent(desktop,XS_BLOCK));

	return 0;
}
