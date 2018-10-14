#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <fcntl.h>

#define NOVALINHA_ID 8520
#define TIPO_ID 8521
#define PRIORIDADE_ID 8522
#define NOMEARQ_ID 8523
#define FINALARQ_ID 8524
#define REALTIMEI_ID 8525
#define REALTIMED_ID 8526

int main (){

	FILE *input; 
	//FILE *fileOUT;
	int *finalArquivo, *prioridade, *tipo, *novaLinha, *realTimeI, *realTimeD;
	int segFinal, segNovaLinha, segNomeExec, segPrioridade, segTipo, segRealTimeI, segRealTimeD;
	char *nomeExec, *linha;
	int contEsp, i, j, status;
	pid_t escalonador;
	//int retorno2, saida;

  /*
  if ((saida=open("fileOUT",O_RDWR|O_CREAT|O_TRUNC,0666)) == -1) { 
      perror("Error open()"); 
      return -1; 
   } 


  if ((retorno2 = dup2(saida,1)) == -1) {
      /* duplicacao de stdout */
//      perror("Error dup2()"); 
 //     return -3; 
 //  }


 

	linha = (char *) malloc (sizeof(char) * 40);

	input = fopen("exec.txt","r");
	if (input == NULL){
		printf("Erro na abertura do arquivo exec.txt.\n");
		exit(1);
	}
	
	segRealTimeI = shmget(REALTIMEI_ID, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
	if( segRealTimeI < 0 ) { 
		printf(" Erro ao criar segRealTimeI\n");
		exit(1);
	}
	realTimeI = (int *) shmat(segRealTimeI, 0, 0);

	segRealTimeD = shmget(REALTIMED_ID, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
	if( segRealTimeD < 0 ) { 
		printf(" Erro ao criar segRealTimeD\n");
		exit(1);
	}
	realTimeD = (int *) shmat(segRealTimeD, 0, 0);
	
	segNovaLinha = shmget(NOVALINHA_ID, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
	if( segNovaLinha < 0 ) { 
		printf(" Erro ao criar segNovaLinha\n");
		exit(1);
	}
	novaLinha = (int *) shmat(segNovaLinha, 0, 0);

	segFinal = shmget(FINALARQ_ID, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
	if( segFinal < 0 ) { 
		printf(" Erro ao criar segFinal\n");
		exit(1);
	}
	finalArquivo = (int *) shmat(segFinal, 0, 0);

	segPrioridade = shmget(PRIORIDADE_ID, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
	if( segPrioridade < 0 ) { 
		printf(" Erro ao criar segPrioridade\n");
		exit(1);
	}
	prioridade = (int *) shmat(segPrioridade, 0, 0);

	segTipo = shmget(TIPO_ID, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
	if( segTipo < 0 ) { 
		printf(" Erro ao criar segTipo\n");
		exit(1);
	}
	tipo = (int *) shmat(segTipo, 0, 0);

	segNomeExec = shmget(NOMEARQ_ID, sizeof(char) * 20, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
	if( segNomeExec < 0 ) { 
		printf(" Erro ao criar segNomeExec\n");
		printf(" Tamanho do nome e de no max 20 caracteres\n");
		exit(1);
	}
	nomeExec = (char *) shmat(segNomeExec, 0, 0);
	
	*novaLinha = 0;
	*finalArquivo = 0;
	
	if((escalonador = fork()) < 0) {
		printf("Erro ao criar o fork para executar o escalonador.\n");
		exit(1);
	}
	
	else if(escalonador != 0) {
		printf("Escalonador criado.\n");
	}
	
	else {
		if(execv("escalonador", NULL) < 0) {
			printf("Erro ao executar o escalonador.\n");
		}
	}
	
	while (fgets(linha,40,input) != NULL){
		contEsp = 0;
		
		linha[strlen(linha) - 1] = '\0';
		
		for (i = 0; i < strlen(linha); i++){
			if (linha[i] == ' ') contEsp++;
		}
		
		if (contEsp == 1){
			*tipo = 2;
		}
		
		else if (contEsp == 2){
			*tipo = 1;
			*prioridade = linha[strlen(linha) - 1] - '0';
		}
		
		else if (contEsp == 3){
			*tipo = 3;
			for (i = 0; i < strlen(linha); i++){
				if (linha[i] == 'I'){
					if (linha[i+1] == '='){
						if (linha[i+3] == ' '){
							*realTimeI = linha[i+2] - '0';
						}
						
						else{
							*realTimeI = (linha[i+3] - '0') + 10*(linha[i+2] - '0');
						}
					}
				}
				
				if (linha[strlen(linha) - 2] == '='){
				   *realTimeD = linha[strlen(linha) - 1] - '0';
				}
				
				else{
				   *realTimeD = 10*(linha[strlen(linha) - 2] - '0');
				   *realTimeD += linha[strlen(linha) - 1] - '0';
				 }
				
			}
		}
		
		else {
			printf("Argumento invalido.\n");
			exit(1);
		}
		
		if (*tipo != 2){
		   for (i = 0, j = 5; linha[j] != ' '; j++, i++) {
		     nomeExec[i] = linha[j];
		   }
		   nomeExec[i] = '\0';
		}
		
		else if (*tipo == 2){
		   for (i = 0, j = 5; linha[j] != '\0'; j++, i++) {
		     nomeExec[i] = linha[j];
		   }
		   nomeExec[i] = '\0';
		}
		
		*novaLinha = 1;
		
		/*
		printf("%s\n",nomeExec);
		if (*tipo == 2) {
		   printf("ROUND-ROBIN:\n");
		}
		
		else if (*tipo == 1){
		   printf("PRIORIDADE:\n");
		   printf("%d\n",*prioridade);
		}
		
		else {
		   printf("REAL-TIME:\n");
		   printf("I=%d ; D=%d\n",*realTimeI,*realTimeD);
		}
		
		*/
		
		
		
		
		sleep(1);
	}
	
	*finalArquivo = 1;
	
	wait(&status);
	
	fclose(input);
  //close(fileOUT); // Fecha arquivo de saída
	
	shmdt(tipo);
	shmdt(prioridade);
	shmdt(realTimeI);
	shmdt(realTimeD);
	shmdt(novaLinha);
	shmdt(nomeExec);
	shmdt(finalArquivo);
	
	shmctl(segNomeExec, IPC_RMID, 0);
	shmctl(segTipo, IPC_RMID, 0);
	shmctl(segPrioridade, IPC_RMID, 0);
	shmctl(segRealTimeI, IPC_RMID, 0);
	shmctl(segRealTimeD, IPC_RMID, 0);
	shmctl(segNovaLinha, IPC_RMID, 0);
	shmctl(segFinal, IPC_RMID, 0);
	
	return 0;
}
	
	
	
	
	
	
			
			
		
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
