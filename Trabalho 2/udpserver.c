/*
 * Gerald Cohn – 1711601
 * Bruno Pedrazza – 1412933
 *
 * udpserver.c - A simple UDP echo server
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFSIZE 65507

#define TRUE 1
#define FALSE 0

#define MAXFILES 40
#define MAXFILENAME 50

extern  int alphasort();
int file_select(const struct direct *entry);
void error(char *msg);

int criaDir(char *path, char *buf, int user, char *perms);
int deletaDirRecursivo (char *path, int flag, char *buf, char *dirname, int user);

int criaEntrada(char* path, int owner, char* permissions);
int leArquivo(char path[], char *buf, int nrbytes, int offset, int user);
int escreveArquivo(char *path, char *buf, int nrbytes, int offset, char* payload, int user, char* perm);

int verificaPermissao(char* path, int user, char opcao);
int verificaPermissaoDir(char *path, int user, char permission);
int dirVerificaPermissaoDir(char *path, int user, char permission);
int imprimePermissao(char* path, char *buf);

char** separaVirgula (char string[], int *n);
char** separaBarra (char string[], int *n);

int parse_buff (char *buf, int n, int *cmd, char *name);
void liberaMemoria(int n,  char **buf);


//Função de seleção dos arquivos
int file_select(const struct direct *entry){
  if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
      return (FALSE);
  else
      return (TRUE);
}

//error - wrapper for perror
void error(char *msg) {
	perror(msg);
	exit(1);
}

//cria novo diretorio se ele nao existe
int criaDir(char *path, char *buf, int user, char *perms){
	struct stat st;
	char pathaux[60];

	if (stat(path, &st) == -1){
		mkdir(path, 0700);
		strcpy(pathaux,path);
		strcat(pathaux,"/cabecalho_dir.txt");
		criaEntrada(pathaux, user, perms);

		//buf  = DC-­‐REP,path(string),strlen(int)
		sprintf(buf, "DC-REP, %s, %d", path, (int) strlen(path));
	}

  //Operação não pode ser realizada strlen=0
  //buf  = DC-­‐REP,path(string),strlen(int)
	else strcpy(buf, "DC-REP, , 0");
	return 0;
}

int deletaDirRecursivo (char *path, int flag, char *buf, char *dirname, int user){
	int vazio;
	struct dirent *next_file;
	char pathArq[256], str[BUFSIZE];
	DIR *folder;
	int achou = 0; // 0 se FALSE, -1 se TRUE
  char dot[256], dotdot[256];
	int ret;

	strcpy(str,path);

	if(flag == TRUE){
		if(strcmp(str, " ") != 0){
			strcpy(path, str);
			strcat(path, "/");
			strcat(path, dirname);
		}
		else{
			strcpy(path, dirname);
		}
	}

	ret = dirVerificaPermissaoDir(path, user, 'W');

	if(ret <= 0){
		printf("denied\n");
		strcpy(buf, "DR-REP,denied");
		return 0;
	}

	folder = opendir(path);
	vazio = rmdir(path);

	if(vazio >= 0){
		sprintf(buf,"DR-REP,%s,%d",str, (int) strlen(str));
		return 0;
	}

	//dot e dotdot guardam o path das pastas /. e /..
	strcpy(dot, path);
	strcat(dot, "/.");
	strcpy(dotdot, path);
	strcat(dotdot, "/..");

	//Este if serve para checar se a pasta path eh existente. Nao se entra neste if nas chamadas recursivas.
	if(flag == TRUE){
		achou = mkdir(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

		if(achou == 0){
			rmdir(path);
			printf("ERRO - Diretorio não existe\n");
			sprintf(buf,"DR-REP,%s,0",str);
			return 1;
		}
	}

  //Caso não seja vazio, deleta recursivamente
	if(vazio < 0){
		while((next_file = readdir(folder)) != NULL){
			sprintf(pathArq, "%s/%s", path, next_file->d_name);

      //Impossibilita a remoção do diretório root e do anterior
      if(strcmp(pathArq, dot) != 0 && strcmp(pathArq, dotdot) != 0){

        //Verifica se é um diretório
        if(next_file->d_type == DT_DIR){
					printf("%s is a path.\n", pathArq);
					vazio = rmdir(pathArq);

          if(vazio < 0){
						deletaDirRecursivo(pathArq, FALSE, NULL,next_file->d_name,user);
						rmdir(pathArq);
					}
				}

        //Verifica se é um arquivo regular
				else if(next_file->d_type == DT_REG){
					printf("%s is a file.\n", pathArq);
					remove(pathArq);
				}
			}
		}
		closedir(folder);
		vazio = rmdir(path);
	}

	if(vazio != 0){
		printf("ERRO - Problema para remover o diretorio\n");
		return 2;
	}

	printf("Diretorio '%s' deletado com sucesso\n", path);

	if(flag == TRUE){
		sprintf(buf,"DR-REP,%s,%d",str, (int) strlen(str));
	}
	return 0;
}

//Cria arquivo ou diretório senão existirem
int criaEntrada(char* path, int owner, char* permissions){
	int fd, w = -1;
	char temp[20];
	char *finalLinha = ",\n";
	char *virgula = ",";

  //Busca o inicio do arquivo para escrita da meta-informação
	if((fd = open(path, O_WRONLY|O_CREAT, 0777)) > 0){
		lseek(fd, 0L,SEEK_SET);
		w = write(fd, "xxxx,", 5);
		w += write(fd, path, strlen(path));
		w += write(fd, virgula, 1);
		snprintf(temp, 20, "%d", (int) strlen(path));
		w += write(fd, temp, strlen(temp));
		w += write(fd, virgula, 1);
		snprintf(temp, 20, "%d", owner);
		w += write(fd, temp, strlen(temp));
		w += write(fd, virgula, 1);
		w += write(fd, permissions,2);
		w += write(fd,finalLinha, strlen(finalLinha));

		//Volta para o início para preencher dinamicamente o tamanho da meta-informação
		lseek(fd,0L, SEEK_SET);
		snprintf(temp,20, "%04d", w);
		write(fd, temp, strlen(temp));
		close(fd);
	}

	printf("w: %d\n", w);
	return w;
}

int leArquivo(char path[], char *buf, int nrbytes, int offset, int user){
	int fd, tamMetaInfo, permDir;
	ssize_t bytesLidos;
	char tempBuf[BUFSIZE];

	//checa permissão de leitura no diretório
	permDir = verificaPermissaoDir(path, user, 'R');
	if(permDir <= 0){
		printf("ERRO - Sem permissão de leitura no diretório\n");
		return -1;
	}

  //checa permissão de leitura do arquivo. Se concedida, return = valor do offset da meta informação
	tamMetaInfo = verificaPermissao(path, user, 'R');

  //Sem permissão de leitura no arquivo
	if(tamMetaInfo == -1){
		printf("ERRO - Sem permissão de leitura do arquivo\n");
		return -1;
	}

	if((fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1){
		printf("Arquivo não existe\n");
		sprintf(buf,"RD-REP,%s,%d, ,0,%d", path, (int) strlen(path), offset);
		return 0;
	}
	strcpy(buf, "");

	bytesLidos = pread(fd, tempBuf, nrbytes, offset + tamMetaInfo);

	if(bytesLidos == 0){
		sprintf(buf,"RD-REP,%s,%d, ,0,%d",path,(int) strlen(path),offset);
		close(fd);
		return 0;
	}
	else if(bytesLidos == nrbytes){
		tempBuf[bytesLidos] = '\0';
		sprintf(buf,"RD-REP,%s,%d,%s,%d,%d",path,(int) strlen(path),tempBuf,(int) bytesLidos,offset);
	}
	else if(bytesLidos < nrbytes){
		tempBuf[bytesLidos] = '\0';
		sprintf(buf,"RD-REP,%s,%d,%s,%d,%d",path,(int) strlen(path),tempBuf,(int) bytesLidos,offset);
	}

	printf("Numero de bytes: %d\n%s\n", (int) bytesLidos, buf);
	close(fd);
	return 0;
}

int escreveArquivo(char *path, char *buf, int nrbytes, int offset, char* payload, int user, char* perm){
	int fd;
	int w, permDir;
	struct stat size;
	int tamMetaInfo;

	//Checa se arquivo a ser modificado é um dos principais da root
	if(strcmp(path,"udpserver.c") == 0 || strcmp(path,"cabecalho_instrucoes.txt") == 0){
		printf("ERRO - Não é permitido alterar arquivos root\n");
		return -2;
	}

	//Checa permissão de escrita dentro do diretório correspondente
	permDir = verificaPermissaoDir(path, user, 'W');
	if(permDir <= 0){
		printf("ERRO - Sem permissão de escrita no diretório\n");
		return -1;
	}

	//checa permissão de escrita do arquivo. Se concedida, return = valor do offset da meta informação
	tamMetaInfo = verificaPermissao(path, user, 'W');

  if(tamMetaInfo == -1){
		printf("ERRO - Sem permissão de escrita no arquivo\n");
		return -1;
	}

  //Caso não exista o arquivo é criado no path correspondente, retornando o valor do offset da meta informação
	else if(tamMetaInfo == -2) tamMetaInfo = criaEntrada(path,user, perm);

  else if (tamMetaInfo < 0){
		printf("ERRO - Problema na meta-informação, tamanho: %d \n", tamMetaInfo);
		return -3;
	}

  //deleta arquivo
	if(nrbytes == 0){
		printf("Excluindo %s\n", path);
		unlink(path);
		return -1;
	}

	printf("path: %s, nrbytes: %d, offset %d, payload %s,\n", path, nrbytes,offset,payload);
	printf("Tamanho da meta-informação: %d\n", tamMetaInfo);

	if(tamMetaInfo > 0)	{
		if((fd = open(path, O_WRONLY, 0777)) > 0){
			if (stat(path,&size) != 0) printf("ERRO\n");

			// offset maior que o tamanho do arquivo, escreve no final concatenado
			if ((offset + tamMetaInfo) > size.st_size) {
				lseek(fd,0,SEEK_END);
			}

			else {
				lseek(fd, offset + tamMetaInfo,SEEK_SET);
			}

			w = write(fd, payload, nrbytes);
			close(fd);
		}
	}

	return w;
}

//Verifica as permissão de acesso do usuário para o path do arquivo solicitado com requisição (W ou R) especificada no parâmetro opção
//Retorna o tamanho total da meta-informação no arquivo em caso de sucesso, -1 para erros e -2 para arquivos que não existem
int verificaPermissao(char* path, int user, char opcao){
	int bytesLidos, fd;
	char tempBuf[MAXFILENAME];
	int tamMetaInfo, i;
	char **infos;
	int n_infos;

  //Arquivo existe
	if((fd = open(path, O_RDONLY)) > 0){
		bytesLidos = pread(fd, tempBuf, 4,0);
		printf("Bytes lidos : %d\n", bytesLidos);
		i = 0;

		while(tempBuf[i] == '0'){
			if(i >= 4){
				printf("ERRO - Problema na meta-informação, qtd bytes iniciais: %d\n", i);
				close(fd);
				return -1;
			}
			i++;
		}

		tempBuf[4] = '\0';
		tamMetaInfo = atoi(&tempBuf[i]); //Transforma o valor em inteiro
		printf("Tamanho meta-informação: %d\n", tamMetaInfo);
		pread(fd, tempBuf, tamMetaInfo, 0);
		close(fd);
		infos = separaVirgula(tempBuf, &n_infos);

		if(n_infos != 6){
			printf("ERRO - Problema na meta-informação, qtd de parâmetros errada: %d\n", n_infos);
			liberaMemoria(n_infos, infos);
			return -1;
		}

    //Senão encontrar erros, checa as permissões
		else{
      //se usuário é o dono
			if(user == atoi(infos[3])){
        //permissão concedida para o próprio user
				if(infos[4][0] == 'W' || infos[4][0] == opcao){
					printf("Permissão concedida - owner\n");
					liberaMemoria(n_infos, infos);
					return tamMetaInfo;
				}
				else{
					liberaMemoria(n_infos, infos);
					return -1;
				}
			}

      //se usuário não é o dono
			else{
        //permissão concedida - arquivo aberto para escrita ou permissão others igual a requisição
				if(infos[4][1] == 'W' || infos[4][1] == opcao){
          printf("Permissão concedida - guest\n");
					liberaMemoria(n_infos, infos);
					return tamMetaInfo;
				}
				else{
					liberaMemoria(n_infos, infos);
					return -1;
				}
			}
		}
		liberaMemoria(n_infos, infos);
	}

  //Arquivo não existe
	else return -2;
}

//Verifica a permissão do diretório do arquivo para requisição feita, caso concedida return > 0
int verificaPermissaoDir(char *path, int user, char permission){
	int i,n,p;
	char pathdir[60];
	char **pathSeparado = separaBarra(path,&n);

	strcpy(pathdir, "");

  //Retira o nome do arquivo no final do path para testar a permissão do diretório correspondente
	for(i = 0; i < n-1; i++){
		strcat(pathdir,pathSeparado[i]);
		strcat(pathdir,"/");
	}

	strcat(pathdir,"cabecalho_dir.txt");
	printf("Path para o diretório:%s\n", pathdir);
	p = verificaPermissao(pathdir,user, permission);

  //permissão concedida
	if(p > 0){
		liberaMemoria(n,pathSeparado);
		return 1;
	}
	else{
		liberaMemoria(n,pathSeparado);
		return -1;
	}
}

//Função similar a anterior para tratar diretório (pois não existe o nome do arquivo no final do path)
int dirVerificaPermissaoDir(char *path, int user, char permission){
	int p;
	char pathdir[60];

	if(strcmp(path, " ") != 0){
      strcpy(pathdir, path);
			strcat(pathdir,"/cabecalho_dir.txt");
    }
    else{
      strcpy(pathdir,"cabecalho_dir.txt");
  }

	p = verificaPermissao(pathdir,user, permission);

  //permissão concedida
  if(p > 0){
		return 1;
	}
	else{
		printf("permissão negada\n");
		return -1;
	}
}

//Lê a meta-informação do arquivo e imprime na tela com o comando FI-REQ
int imprimePermissao(char* path, char *buf){
	int bytesLidos, fd;
	char tempBuf[MAXFILENAME];
	int tamMetaInfo, i;
	struct stat size;
	char **infos;
	int n_infos;

  //arquivo existe
	if((fd = open(path, O_RDONLY)) > 0){
		bytesLidos = pread(fd, tempBuf, 4,0);
		printf("Bytes lidos : %d\n", bytesLidos);
		i = 0;
		while(tempBuf[i] == '0'){
			if(i >= 4){
				printf("ERRO - Problema na meta-informação, qtd bytes iniciais: %d\n", i);
				close(fd);
				return -1;
			}
			i++;
		}

		tamMetaInfo = atoi(&tempBuf[i]);
		printf("Tamanho da meta-informação: %d\n", tamMetaInfo);
		pread(fd, tempBuf, tamMetaInfo, 0);
		close(fd);
		infos = separaVirgula(tempBuf, &n_infos);

		if(n_infos != 6){
			printf("ERRO - Problema na meta-informação, qtd de parâmetros errada: %d\n", n_infos);
			liberaMemoria(n_infos, infos);
			return -1;
		}

		// utiliza struct stat para obter o tamanho do arquivo
		if (stat(path,&size) != 0) printf("ERRO\n");
		sprintf(buf,"FI-REP,%s,%d,%s,'%s',%d",path, (int) strlen(path), infos[3], infos[4],size.st_size);
		liberaMemoria(n_infos, infos);
	}

  //Arquivo não existe
	else{
		sprintf(buf,"FI-REP,%s,%d,arquivo inexistente",path, (int) strlen(path));
		return -2;
	}
	return 0;
}

//Dada uma entrada padrão com uma ou mais "," aloca um array de strings com os parâmetros separados
char** separaVirgula (char string[], int *n){
	char str[BUFSIZE];
	char *p;
	int tam = 0;
	char **buf = (char**) malloc(0 * sizeof(char*));

	strcpy(str, string);

	//Quebra a string a cada "," presente entre os parâmetros do buf
	p =  strtok (str, ",");

	//Alocação dinâmica do array para cada parâmetro sem as vírgulas
	while (p != NULL) {
		buf = realloc (buf, sizeof (char*) * (tam+1));

		if (buf == NULL){
			printf("ERRO - Problema para alocar memoria\n");
	    	exit (-1);
		}

		buf[tam] = (char*) malloc((strlen(p)+1) * sizeof(char));
		strcpy(buf[tam], p);
		tam++;

    //Pega o próximo parâmetro da string quebrada até a ","
		p = strtok (NULL, ",");
	}

	*n = tam;
	return buf;

}
//Dado um diretório padrão separado por "/" aloca um array de strings com os caminhos separados
char** separaBarra (char string[], int *n){
	char str[BUFSIZE];
	char *p;
	int tam = 0;
	char **buf = (char**) malloc(0 * sizeof(char*));

	strcpy(str, string);

  //Quebra a string a cada "/" presente entre os parâmetros do buf
	p =  strtok (str, "/");

	//Alocação dinâmica do array para cada parâmetro sem as barras
	while (p != NULL) {
		buf = realloc (buf, sizeof (char*) * (tam+1));

		if (buf == NULL){
			printf("ERRO - Problema para alocar memoria\n");
	    	exit (-1);
		}

		buf[tam] = (char*) malloc((strlen(p)+1) * sizeof(char));
		strcpy(buf[tam], p);
		tam++;

    //Pega o próximo parâmetro da string quebrada até a "/"
		p = strtok (NULL, "/");
	}

	*n = tam;
	return buf;

}

//função parse da udp que interpreta o comando recebido em buf
int parse_buff (char *buf, int n, int *cmd, char *name){
  int i, count,w,ret;
  int ni;
  char **comandos;
  char str[BUFSIZE];
  struct direct **files;
  int *ini, *fim, x;

  comandos = NULL;
  comandos = separaVirgula(buf, &ni);
  
  for(i = 0; i < ni; i++){
    printf("\nComandos[%d]: %s", i, comandos[i]);
  }

	//RD-­‐REQ,path(string),strlen(int),payload(stringvazio),nrbytes(int),offset(int),user(int)
	//lê nrbytes do arquivo path a partir da posição offset
	if (strcmp(comandos[0], "RD-REQ") == 0){
		count=0;
		while(comandos[count] != NULL){
			count++;
		}

		if(count < 6){
			printf("ERRO - %d parametros. Digite  'comando', 'path', 'tamanho path', 'um espaco vazio' ,'quantidade de bytes a serem lidos','offset para inicio de leitura, e o usuário'\n", count);

			count=0;
			while(comandos[count] != NULL){
				printf("%s\n", comandos[count]);
				count++;
			}

			return 0;
		}
		leArquivo(comandos[1], buf, atoi(comandos[4]), atoi(comandos[5]), atoi(comandos[6]));
	}


	//WR-­REQ,path(string),strlen(int),payload(string),nrbytes(int),offset(int),user(int),perms(char[2])
	//escreve nrbytes do conteúdo de payload no arquivo path a partir da posição offset
  //Caso arquivo não exista ele é criado, e se nrbytes=0 deleta o arquivo
	else if (strcmp(comandos[0], "WR-REQ") == 0){
		count=0;
		while(comandos[count] != NULL){
			count++;
		}

		if(count < 7){
			printf("ERRO - %d parametros. Digite  'comando', 'path', 'tamanho path', 'string para escrita' ,'quantidade de bytes a serem escritos','offset para inicio de escrita, usuário e permissões'\n", count);
			count=0;

			while(comandos[count] != NULL){
				printf("%s\n", comandos[count]);
				count++;
			}
			return 0;
		}

		w = escreveArquivo(comandos[1], buf, atoi(comandos[4]), atoi(comandos[5]), comandos[3], atoi(comandos[6]), comandos[7]);
		strcpy(buf,"");
		sprintf(buf, "WR-REP,%s,%ld, ,%d,%d",comandos[1],strlen(comandos[1]), w,atoi(comandos[5]));

	}

	//FI-­REQ,path(string),strlen(int)
	//lista os meta-dados do arquivo
	else if (strcmp(comandos[0], "FI-REQ") == 0){
		count=0;
		while(comandos[count] != NULL){
			count++;
		}

		if(count < 3){
			printf("ERRO. %d parametros. Digite  'comando', 'arquivo', 'tamanho path'\n", count);
			count=0;
			while(comandos[count] != NULL){
				printf("%s\n", comandos[count]);
				count++;
			}
			return 0;
		}
		imprimePermissao(comandos[1],buf);
	}

	//DC-­REQ,path(string),strlen(int), dirname(string),strlen(int),user(int),permission(char[2])
	//cria um novo subdiretório dirname em path
	else if (strcmp(comandos[0], "DC-REQ") == 0){
		//str = full relative path
		if(strcmp(comandos[1], " ") != 0){
			strcpy(str, comandos[1]);

			strcat(str, "/");

			strcat(str, comandos[3]);
		}
		else{
			strcpy(str, comandos[3]);
		}

		printf("str: %s\n", str);
		criaDir(str, buf, atoi(comandos[5]),comandos[6]);
	}

	//DR‐REQ,path(string),strlen(int),dirname(string),strlen(int),user(int)
	//remove o sub-diretório dirname de path
	else if (strcmp(comandos[0], "DR-REQ") == 0){
		count=0;
		while(comandos[count] != NULL){
			count++;
		}

		if(count < 5){
			printf("ERRO - %d parametros. Digite  'comando', 'path', 'tamanho path', 'nome diretório', 'tamanho do nome do diretório' e usuário.\n", count);
			count=0;
			while(comandos[count] != NULL){
				printf("%s\n", comandos[count]);
				count++;
			}

			return 0;
		}

		deletaDirRecursivo(comandos[1], TRUE, buf, comandos[3],atoi(comandos[5]));
	}

	//DL­‐REQ,path(string),strlen(int),user(int)
  //Retorna os nomes de todos os arquivos/subdiretórios em path em um unico char array, sendo que fstlstpositions[i] indica a posição inicial e final do i-­ésimo nome em allfilenames
	else if (strcmp(comandos[0], "DL-REQ") == 0){
		printf("Instrução DL-REQ\n");

    if(strcmp(comandos[1], " ") != 0){
      strcpy(str, comandos[1]);
      strcat(str,"/cabecalho_dir.txt");
    }
    else{
      strcpy(str,"/cabecalho_dir.txt");
    }

		ret = dirVerificaPermissaoDir(comandos[1],atoi(comandos[3]), 'R');

		if(ret <= 0){
			printf("ERRO - Sem permissão de leitura no diretório\n");
			strcpy(buf, "DL-REP,negado");
			return 0;
		}

		printf("Permissão de leitura concedida\n");
		ini = (int*) malloc(MAXFILES * sizeof(int));
		fim = (int*) malloc(MAXFILES * sizeof(int));

		if(ini == NULL || fim == NULL){
			printf("ERRO - Problema para alocar memoria\n");
			exit(0);
		}

		if(strcmp(comandos[1], " ") == 0){
			strcpy(comandos[1], "./");
		}

		count = scandir( comandos[1], &files, file_select, alphasort);
		printf("Quantidade de entradas: %d", count);

		//DL­‐REP,allfilenames(string),fstlstpositions(array[40] of struct{int,int}),nrnames(int)
		strcpy(buf, "DL-REP,");

		for(i = 0, x = 0; i < count;i++){
			sprintf(str, "%s", files[i]->d_name);
			strcat(buf, str);
			ini[i] = x;
			x += strlen(str);
			fim[i] = x - 1;
		}

		strcat(buf, ",");

		for(i = 0; i < count; i++){
			sprintf(str, "%d %d ", ini[i], fim[i]);
			strcat(buf, str);
		}
	}

	else{
		printf("ERRO - Comando não existe\n");
		strcpy(buf, "ERRO - Comando não existe");
	}

  liberaMemoria(ni, comandos); //liberaMemoria causando erro no hostp da main
  return 0;
}

//Libera a memória do array passado como parâmetro
void liberaMemoria(int n,  char **buf){
	int i;

	for(i = 0; i < n; i++){
		if(buf[i] != NULL){
			free(buf[i]);
		}
	}
	if(buf != NULL) free(buf);
	return;
}

int main(int argc, char **argv) {
  	int sockfd; /* socket */
  	int portno; /* port to listen on */
  	int clientlen; /* byte size of client's address */
  	struct sockaddr_in serveraddr; /* server's addr */
  	struct sockaddr_in clientaddr; /* client addr */
  	struct hostent *hostp; /* client host info */
  	char buf[BUFSIZE]; /* message buf */
  	char *hostaddrp; /* dotted decimal host addr string */
  	int optval; /* flag value for setsockopt */
  	int n; /* message byte size */

  	char name[BUFSIZE];   // name of the file received from client
  	int cmd;              // cmd received from client

	/*
	 * check command line arguments
	 */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);

  	/*
   	 * socket: create the parent socket
   	 */
  	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  	if (sockfd < 0)
    	error("ERROR opening socket");

  	/* setsockopt: Handy debugging trick that lets
   	 * us rerun the server immediately after we kill it;
   	 * otherwise we have to wait about 20 secs.
   	 * Eliminates "ERROR on binding: Address already in use" error.
   	 */
  	optval = 1;
  	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  	/*
	 * build the server's Internet address
	 */
  	bzero((char *) &serveraddr, sizeof(serveraddr));
  	serveraddr.sin_family = AF_INET;
  	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  	serveraddr.sin_port = htons((unsigned short)portno);

  	/*
   	 * bind: associate the parent socket with a port
   	 */
  	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    	error("ERROR on binding");

  	/*
   	 * main loop: wait for a datagram, then echo it
   	 */
  	clientlen = sizeof(clientaddr);

  	while (1) {

	    /*
	     * recvfrom: receive a UDP datagram from a client
	     */
	    bzero(buf, BUFSIZE);
	    n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, (socklen_t*restrict)&clientlen);
	    if (n < 0)
	    {
	    	error("ERROR in recvfrom");
	    }

	    parse_buff(buf,n, &cmd, name);

	    /*
	     * gethostbyaddr: determine who sent the datagram
	     */

	    printf("Feedback: %s\n", buf);

	    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);

	    if (hostp == NULL)
	    	error("ERROR on gethostbyaddr");

	    hostaddrp = inet_ntoa(clientaddr.sin_addr);

	    if (hostaddrp == NULL)
	    {
	    	error("ERROR on inet_ntoa\n");
	    }

	    printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);

	    printf("server received %ld/%d bytes: %s\n", strlen(buf), n, buf);

	    /*
	     * sendto: echo the input back to the client
	     */
	    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);

	    if (n < 0)
	    	error("ERROR in sendto");
  	}
}
