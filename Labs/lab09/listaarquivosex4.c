#include <sys/types.h> 
#include <sys/dir.h> 
#include <sys/param.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE 0 
#define TRUE 1


extern  int alphasort(); 
char pathname[MAXPATHLEN];


int file_select(struct direct *entry) 
{ 
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))  
          return (FALSE); 
    else 
          return (TRUE); 
}

int profundidade(char *pathname, long int *nArqs, long int *tamArqs, int nivel){
	
	int count; 
	struct direct **files;
	struct stat buf;
	char pathnameaux[MAXPATHLEN];
	
	long int auxnArqs = 0;
	long int auxtamArqs = 0;
	
	int prof = nivel, l;
	
	count = scandir(pathname, &files, file_select, alphasort);

	if (count <= 0) return -1;
	
	for (int i = 1; i<count + 1; ++i){
						
		printf("%*s[%s]\n",4 * nivel, "",files[i-1]->d_name);
		
		if(stat(files[i-1]->d_name, &buf) == 1) printf("Erro.\n");
		
		if((int)(files[i-1]->d_type) == 4){
			strcpy(pathnameaux, pathname);
			strcat(pathnameaux, "/");
			strcat(pathnameaux, files[i-1]->d_name);
			l = profundidade(pathnameaux, &auxnArqs, &auxtamArqs, nivel + 1);
			if(l > prof) prof = l;
		}
		
		auxnArqs++;
		auxtamArqs += buf.st_size;
		
	}
	
	*nArqs += auxnArqs;
	*tamArqs += auxtamArqs;
	
	return prof;
	
}

int main(void){

	char pathname[MAXPATHLEN];

	long int nArqs = 0, tamArqs = 0;	
	int prof;
	
	if (getcwd(pathname, MAXPATHLEN) == NULL){  
		printf("Error getting path\n");
		exit(0);
	}
	
	printf("Current Working Directory = %s\n",pathname);
	
	prof = profundidade(pathname, &nArqs, &tamArqs, 0);
	
	printf("Numero total de arquivos: %ld\n",nArqs);
	printf("Tamanho total dos arquivos: %ld\n",tamArqs);
	printf("Profundidade máxima: %d\n", prof);

	printf("\n"); /* flush buffer */ 
	
	return 0;
}

