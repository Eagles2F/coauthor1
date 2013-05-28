#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libcsv.h"
#include "dao.h"
#include "hashmap.h"
#include "anode.h"
#include "filter.h"

//如果要跑所有的数据，清“注释”掉下面一行 #define PARTDATA 
//#define PARTDATA

//#define AUTH_NODE_LEN 1024



typedef struct single_paper
{
	unsigned long pid;
	unsigned long len;
	unsigned long aid[AUTH_NODE_LEN];
}sPaper;

typedef struct pa_info
{
	unsigned long pid;
	sPaper pinfo;
	unsigned long aid;
	unsigned long line;
	int col;//column
	hashmap *phmap;
}paInfo;

typedef struct 
{
	int col;
	unsigned int aid;
	hashmap *pmap;
	FILE *fp; // file to saved
}aMap;

// pid , aid , name , aff 
void hlog (void *data, size_t len, void *t)
{
	paInfo* pi = (paInfo *)t;
	char str[20];
	unsigned long ival;
	
	if(pi->line == 0)
	{
		pi->col++;
	}else
	{
		switch(pi->col)
		{
			case 0:
				memcpy(str,data,len);
				str[len]='\0';
				ival = atol(str);
				pi->pid = ival;
				//printf("{pid:%ld}",pi->pid);
				break;
			case 1:
				memcpy(str,data,len);
				str[len]='\0';
				ival = atol(str);
				pi->aid = ival;
				//printf("{aid:%ld}",pi->aid);
				break;
		}
		pi->col++;
	}	
}

void nlog (int c, void *t)
{
	paInfo* pi = (paInfo *)t;
	pNode p1,p2;
	int i;
	if(pi->line == 0 )
	{
		(pi->line) ++;
		pi->col = 0;
		printf("LA:%ld\n",pi->line);
		return;
	}
	//printf("A");fflush(NULL);
	if(pi->pid == pi->pinfo.pid) // in one paper
	{
		#ifdef PARTDATA
		int f1 = isNecessary(pi->aid);
		
		if(f1) 
		#endif //PARTDATA
			p1 =  pNodeSearch(pi->phmap,pi->aid);		
		for(i=0;i<pi->pinfo.len;i++)
		{
			#ifdef PARTDATA
			int f2 = isNecessary(pi->pinfo.aid[i]);
			//dbaddinfo(pi->aid,pi->pinfo.aid[i]);//
			//dbaddinfo(pi->pinfo.aid[i],pi->aid);
			if(f2)  
			#endif //PARTDATA
				p2 =  pNodeSearch(pi->phmap,pi->pinfo.aid[i]);			
		
			#ifdef PARTDATA 
			if(f1)
			#endif //PARTDATA
				pNodeAddInfo(&p1,pi->pinfo.aid[i]);
			
			#ifdef PARTDATA	
			if(f2)  
			#endif //PARTDATA
				pNodeAddInfo(&p2,pi->aid);
		
		}
		
		pi->pinfo.aid[pi->pinfo.len]=pi->aid;
		(pi->pinfo.len) ++;
		//printStruct(pi->phmap);
	}else // not in one paper
	{
		pi->pinfo.pid = pi->pid;
		pi->pinfo.aid[0]=pi->aid;
		pi->pinfo.len = 1;
	}
	(pi->line)++;
	pi->col = 0;
	printf("LB:%ld\n",pi->line);
}

void readAid(void *data, size_t len, void *t)
{
	aMap* d = (aMap *)t;
	if(d->col == 0)
	{
		// store aid
		char str[20];
		memcpy(str,data,len);
		str[len]='\0';
		d->aid = atol(str);
		printf("~%d,",d->aid);
	}
	d->col ++;
}

// DEBUG INFO
//int act = 0;

int ulongcmp(const void *a,const void *b)
{
	// i > j return >0
	// get 0 , 1 ,2 ....100
	return (*(unsigned long*)a) - (*(unsigned long*)b);
}

void writeCoAu(int c, void *t)
{
	aMap* d = (aMap *)t;
	FILE *fp = d->fp;
	hashmap *phmap = d->pmap;
	unsigned int aid = d->aid;
	
	pNode np;
	int plen;
	int i;
	
	fprintf(fp,"%d,",aid);
	if((np=hashmapGet(phmap,(unsigned long)aid)) != NULL)
	{
		
		// DEBUG INFO
		// printf("%d(%ld) ",act++,np->len);	
		plen = np->len;
		qsort((np->aid)+1,plen-1,sizeof(unsigned long),ulongcmp);
		for(i=1;i<plen-1;i++)
		{
			fprintf(fp,"%ld,",np->aid[i]);
		}
		fprintf(fp,"%ld",np->aid[plen-1]);
	}
	fprintf(fp,"\n");
	
	
	// reset info
	d->col = 0;
	d->aid = 0;
}

int main (int argc,char *argv[]) {
	FILE *fp;
	struct csv_parser *p;
	char buf[1024];
	size_t bytes_read;
	const char *file_name="data/PaperAuthor.csv";
	const char *author_csv_file_name="data/Author.csv";
	paInfo t;
	
	#ifdef PARTDATA
	loadSameNameUnoverlapped();
	#endif //PARTDATA

	if((p=(struct csv_parser *)malloc(sizeof(struct csv_parser))) == 0) return -1;
	
	if ((fp = fopen(file_name,"r"))== NULL)
	{
		fprintf(stderr, "Failed to open %s\n",file_name);
	}

	csv_init(p, (unsigned char)0);
	
	//int pid;
	//sPaper pinfo;
	//int aid;
	//int line;
	//int col;//column
	//pNode head;
	t.pid = 0;
	t.pinfo.pid = 0;
	t.aid = 0;
	t.line = 0;//0;
	t.col = 0;
	
	printf("init \n");
	//t.phmap = hashmapCreate((unsigned long)1000000);
	t.phmap = hashmapCreate((unsigned long)1000);
	
	printf("reading csv file \n"); fflush(NULL);
	while ((bytes_read=fread(buf, 1, 1024, fp)) > 0) {
		if (csv_parse(p, buf, bytes_read, hlog, nlog,&t) != bytes_read) {
			fprintf(stderr, "Error while parsing file: %s\n", 
							csv_strerror(csv_error(p)));
		}
	}
	csv_free(p);

	//fo = fopen("CoAuthor.csv","w");

	//if(fo == NULL) fprintf(stderr, "Failed to open CoAuthor.csv\n");
	//printf("writing ... ");	
	//writedata(t.head,fo);
	
	printf("store into CSV\n"); fflush(NULL);
	
	
	//////////////////////////////////////////////////////////////////////////////////////////
	//storeIntoCSV(t.phmap);
	aMap nd; // to transfer info when store into csv file
	nd.col = 0;
	nd.pmap = t.phmap;
	nd.aid = 0;
	
	FILE *fw;
	if((fw = fopen("CoAuthor.csv","w")) == NULL)
	{
		fprintf(stderr,"error opening CoAuthor.csv\n");
	}
	nd.fp = fw;
	
	FILE *fa; // Author.csv
	
	if ((fa = fopen(author_csv_file_name,"r"))== NULL)
	{
		fprintf(stderr, "Failed to open %s\n",file_name);
	}
	csv_init(p, (unsigned char)0);
	while ((bytes_read=fread(buf, 1, 1024, fa)) > 0) {
		if (csv_parse(p, buf, bytes_read, readAid, writeCoAu,&nd) != bytes_read) {
			fprintf(stderr, "Error while parsing file: %s\n", 
							csv_strerror(csv_error(p)));
		}
	}
	csv_free(p);
	fclose(fa);
	fclose(fw);
	//////////////////////////////////////////////////////////////////////////////////////////
	
	
	printf("ok\n");

	freeAuthorList();
	fclose(fp);	
	
	
	
	// DB test
	
	/*
	printf("test\n");
	int a[300];
	int alen;
	//test 2230280
	dbconnect();
	alen = getcoau(972575,a,300);
	printf("len is:%d",alen);
	for(int i=0;i<alen;i++)
		printf("%d ",a[i]);
	dbfree();
	*/
	
	hashmapDelete(t.phmap);
	
	printf("complete ! \n");
	return 0;
}


