#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include "funcoes.h"

#define NOVALINHA_ID 8520
#define TIPO_ID 8521
#define PRIORIDADE_ID 8522
#define NOMEARQ_ID 8523
#define FINALARQ_ID 8524
#define REALTIMEI_ID 8525
#define REALTIMED_ID 8526



Prog *listaPrioridade = NULL;
Prog *listaRoundRobin = NULL;
Prog *listaRealTime = NULL;
char progsRT [60];

void insereExec(char *nomeExec, int tipo, int prioridade, int realTimeI, int realTimeD);
int compare (const void * a, const void * b);
void execListaPrioridade(Prog *tempPrioridade);
void execListaRoundRobin (Prog *tempRoundRobin);
void rotacaoRoundRobin ();
void execListaRealTime(Prog* tempRealTime);
void alarme(int sinal);
void encerraFilho(int sinal);
void liberaProg(Prog * atual);

// Insere um novo programa para ser escalonado
void insereExec(char *nomeExec, int tipo, int prioridade, int realTimeI, int realTimeD) {

	pid_t pid = 0;

	// Testa a criação do filho senão retornar erro, o filho irá fazer o exec do programa enquanto o pai espera
	if((pid = fork()) < 0) {
		printf("Erro ao criar processo filho.\n");
		exit(1);
	}

	else if(pid == 0) {
		if(execv(nomeExec, NULL) < 0) {
			printf("Erro ao executar o programa %s.\n", nomeExec);
		}
	}

	else if(pid > 0) {
		int status;

		kill(pid, SIGSTOP);
		printf("Programa %s foi criado\n", nomeExec);

		//Round-Robin
		if (tipo == 2) {
			listaRoundRobin = insereProg(listaRoundRobin, pid, nomeExec, tipo, prioridade, realTimeI, realTimeD);
		}

		else if (tipo == 1) {

			// Procura programa com prioridade menor (ex:  prioridade 0 é a primeira a ser executada) e insere antes dele
			Prog * lp = listaPrioridade;
			Prog * antecessor = NULL;
			Prog * atual;

			//Testa entradas com prioridades invalidas
			if (prioridade < 0 || prioridade > 7){
				printf("Valor de prioridade invalido\n");
				exit(1);
			}

			if( lp == NULL ) {
				listaPrioridade = insereProg(listaPrioridade, pid, nomeExec, tipo, prioridade, realTimeI, realTimeD);
			  printf("lp \n");
			}

			else if (listaPrioridade != NULL) {
				while(lp != NULL && (lp->prioridade <= prioridade) ){

					antecessor = lp;
					lp = lp->prox;
				}

				atual = insereProg(lp, pid, nomeExec, tipo, prioridade, realTimeI, realTimeD);
				atual->ant = antecessor;

				if (antecessor == NULL) {
					listaPrioridade = atual;
				}
				else {
					antecessor->prox = atual;
					listaPrioridade =  atual;
				}
			}

			imprimePrioridades(listaPrioridade);
			printf("\n\n");
		}

		else if (tipo == 3) {
			int tempoFinal = realTimeI + realTimeD;
			int i, j;
			Prog *atual, *anteriorRT = NULL;
			Prog *auxRT = listaRealTime;

			for(i=0; i<60; i++){
				progsRT[i]=0;
			}

			if(tempoFinal > 60){
				printf("Periodo de execucao do processo longo demais, %d segundos\n", tempoFinal);
				exit(1);
			}

			else if (listaRealTime == NULL){
				listaRealTime = insereProg(listaRealTime, pid, nomeExec, tipo, prioridade, realTimeI, realTimeD);

				for(i=realTimeI;i<tempoFinal;i++){
					progsRT[i] = 1;
				}
			}

			else{
				//Arruma em um array de 60 posições as os segundos ocupados, para verificar conflitos
				for(j=realTimeI; j<realTimeD; j++){
					if(progsRT[j] == 1){
						printf("Programa %s possui parametros de escalonamento conflitantes com outro programa\n", nomeExec);
						exit(1);
					}
				}
				//Coloca o novo programa na ordem crescente de tempos de inicio
				while(auxRT != NULL && realTimeI > auxRT->realTimeI){
					anteriorRT = auxRT;
					auxRT = auxRT->prox;
				}

				for(i = realTimeI; i < tempoFinal; i++){
					progsRT[i] = 1;
				}

				atual = insereProg(auxRT, pid, nomeExec, tipo, prioridade, realTimeI, realTimeD);
				atual->ant = anteriorRT;

				if (anteriorRT == NULL) {
					listaRealTime = atual;
				}
				else {
					anteriorRT->prox = atual;
					listaRealTime = atual;   
				}
			}
		}
	}
}

int comparaRT(const void *rt1, const void *rt2) {
	Prog * a;
	Prog * b;
	int r;

	a = (Prog*)rt1;
	b = (Prog*)rt2;
	r = (a->realTimeI - b->realTimeI);

	return r;
}

void execListaPrioridade(Prog *tempPrioridade) {
	if (listaPrioridade != NULL && listaRealTime == NULL) {
    kill(listaPrioridade->pid, SIGCONT);
		if( tempPrioridade != listaPrioridade) {
			kill(tempPrioridade->pid, SIGSTOP);
			kill(listaPrioridade->pid, SIGCONT);
		}
		else {
			kill(listaPrioridade->pid, SIGCONT);
		}
	}
}

void execListaRoundRobin (Prog *tempRoundRobin) {
	if (listaRealTime == NULL && listaPrioridade == NULL && listaRoundRobin != NULL) {
		kill(listaRoundRobin->pid, SIGCONT);
		if (listaPrioridade == NULL && tempRoundRobin == NULL) {
			alarm(1);
		}
	}
}

void rotacaoRoundRobin () {

	Prog *novoFinal; //Processo que estava executando, que será colocado no final da fila
	Prog *listaAux;  //Copia para não alterar a lista em si

	//Testa se lista não está vazia, e se não tem somente um programa
	if (listaRoundRobin != NULL && listaRoundRobin->prox != NULL) {

		novoFinal = listaRoundRobin;

		listaRoundRobin = listaRoundRobin->prox;
		novoFinal->prox = NULL;
		listaRoundRobin->ant = NULL;
		listaAux = listaRoundRobin;

		while (listaAux->prox != NULL) {
			listaAux = listaAux->prox;
		}

		listaAux->prox = novoFinal;
		novoFinal->ant = listaAux;
	}
}

// Executa o programa pois esta no seu tempo de inicio, e roda pela duração prevista
void execListaRealTime(Prog* tempRealTime) {
	time_t tempoPrograma;
	unsigned int tempoAtual;

	tempoPrograma = time(NULL);
	kill(listaRealTime->pid, SIGCONT);
	tempoAtual = ((unsigned int)time(NULL) - tempoPrograma) % 60;
	if (tempoAtual == tempRealTime->realTimeD){
		kill(listaRealTime->pid, SIGSTOP);
	}
}



//Função que trata o sinal de SIGALRM, é invocada com a função alarm()
void alarme(int sinal) {
	if (listaRoundRobin != NULL) {
		kill(listaRoundRobin->pid, SIGSTOP);
		printf("Roda os processos após 0.5 segundos\n");
		rotacaoRoundRobin();
		if (listaPrioridade == NULL && listaRealTime == NULL) {
			alarm(1);
		}
	}
	else if (listaPrioridade != NULL) {
		kill(listaRoundRobin->pid, SIGSTOP); 
		if (listaRealTime == NULL) {
			alarm(1);
		}
	}
}

 //Função que trata o sinal de SIGCHLD, é invocada quando um filho é encerrado
void encerraFilho(int sinal) {
	int status, tipo;
 	Prog *PR = listaPrioridade;
 	Prog *RR = listaRoundRobin;
 	Prog *RT = listaRealTime;
 	pid_t pid;

 	pid = waitpid(-1, &status, WUNTRACED | WCONTINUED | WNOHANG);


 	if (WIFEXITED(status) == 1){
 		//Whiles inicializam os tipos para todas as instâncias da lista
 		while( RT != NULL ){
 	   if(RT->pid == pid) {
 	    tipo = 3;
 	   }
 	   RT = RT->prox;
 	 	}

 		while( PR != NULL ){
 			if(PR->pid == pid) {
 				tipo = 1;
 			}
 			PR = PR->prox;
 	  }

		while( RR != NULL ){
 	    if(RR->pid == pid) {
 	    	tipo = 2;
 	    }
 	    RR = RR->prox;
 	  }

 	    printf("\nPrograma anterior terminou a execucao\n");

 	    if (tipo == 1) {
 	    	if (listaPrioridade != NULL) {
 					//Se for um processo único, exclue e esvazia a lista
 					if (listaPrioridade->prox == NULL) {
 						free(listaPrioridade);
 						listaPrioridade = NULL;
 					}
 					//Senão tira o programa atual da lista, e faz o proximo programa ser o primeiro da lista
 					else {
 						listaPrioridade = listaPrioridade->prox;
 						free(listaPrioridade->ant);
 						listaPrioridade->ant = NULL;
 					}
 				}
 	    }

 	    else if (tipo == 2) {
 	    	if (listaRoundRobin != NULL) {
 				//Se for um processo único, exclue e esvazia a lista
 	    		if (listaRoundRobin->prox == NULL) {
 	    			free(listaRoundRobin);
 	    			listaRoundRobin = NULL;
 	    		}
 				//Senão tira o programa atual da lista, e faz o proximo programa ser o primeiro da lista
 	    		else {
 	    			listaRoundRobin = listaRoundRobin->prox;
 	    			free(listaRoundRobin->ant);
 	    			listaRoundRobin->ant = NULL;
 	    		}
 	    	}
 	    }

 	    else if (tipo == 3) {
 	    	if (listaRealTime != NULL) {
 				//Se for um processo único, exclue e esvazia a lista
 	    		if (listaRealTime->prox == NULL) {
 	    			free(listaRealTime);
 	    			listaRealTime = NULL;
 	    		}
 	    		else {
						listaRealTime = listaRealTime->prox;
						free(listaRealTime->ant);
						listaRealTime->ant = NULL;
					}
 				}
 			}
		}
	}

void liberaProg(Prog * atual) {

 	while(atual != NULL) {
 		Prog * proximo = atual->prox;
 		free(atual);
 		atual = proximo;
 	}
}

int main() {

	int segFinal, segNovaLinha, segNomeExec, segPrioridade, segTipo, segRealTimeI, segRealTimeD;
	char *nomeExec;
	int *finalArquivo, *prioridade, *tipo, *novaLinha, *realTimeI, *realTimeD;
	time_t inicioTempo = time(NULL);
	unsigned int tempoAtual;

	signal(SIGCHLD, encerraFilho);
	signal(SIGALRM, alarme);

	segRealTimeI = shmget(REALTIMEI_ID, sizeof(int), S_IRUSR | S_IWUSR );
	if( segRealTimeI < 0 ) {
		printf(" Erro ao criar segRealTimeI\n");
		exit(1);
	}
	realTimeI = (int *) shmat(segRealTimeI, 0, 0);

	segRealTimeD = shmget(REALTIMED_ID, sizeof(int), S_IRUSR | S_IWUSR );
	if( segRealTimeD < 0 ) {
		printf(" Erro ao criar segRealTimeD\n");
		exit(1);
	}
	realTimeD = (int *) shmat(segRealTimeD, 0, 0);

	segNovaLinha = shmget(NOVALINHA_ID, sizeof(int), S_IRUSR | S_IWUSR );
	if( segNovaLinha < 0 ) {
		printf(" Erro ao criar segNovaLinha\n");
		exit(1);
	}
	novaLinha = (int *) shmat(segNovaLinha, 0, 0);

	segFinal = shmget(FINALARQ_ID, sizeof(int), S_IRUSR | S_IWUSR );
	if( segFinal < 0 ) {
		printf(" Erro ao criar segFinal\n");
		exit(1);
	}
	finalArquivo = (int *) shmat(segFinal, 0, 0);

	segPrioridade = shmget(PRIORIDADE_ID, sizeof(int), S_IRUSR | S_IWUSR );
	if( segPrioridade < 0 ) {
		printf(" Erro ao criar segPrioridade\n");
		exit(1);
	}
	prioridade = (int *) shmat(segPrioridade, 0, 0);

	segTipo = shmget(TIPO_ID, sizeof(int), S_IRUSR | S_IWUSR );
	if( segTipo < 0 ) {
		printf(" Erro ao criar segTipo\n");
		exit(1);
	}
	tipo = (int *) shmat(segTipo, 0, 0);

	segNomeExec = shmget(NOMEARQ_ID, sizeof(char) * 20, S_IRUSR | S_IWUSR );
	if( segNomeExec < 0 ) {
		printf(" Erro ao criar segNomeExec\n");
		printf(" Tamanho do nome e de no max 20 caracteres\n");
		exit(1);
	}
	nomeExec = (char *) shmat(segNomeExec, 0, 0);

	while(1) {
		Prog *tempPrioridade = listaPrioridade;
		//printf("tempPrioridade %d\n", tempPrioridade->prioridade);
		Prog *tempRoundRobin = listaRoundRobin;
		Prog *tempRealTime   = listaRealTime;

		if(*novaLinha == 1){
			printf("Nome Executavel: %s, tipo: %d, Real Time Inicio: %d, Real Time Duracao: %d, prioridade: %d \n", nomeExec, *tipo, *realTimeI, *realTimeD, * prioridade);
			*novaLinha = 0;
			insereExec(nomeExec, *tipo, *prioridade, *realTimeI, *realTimeD);
		}

		else if (*finalArquivo == 1 && (listaPrioridade == NULL && listaRealTime == NULL && listaRoundRobin == NULL)) {
			break;
		}

		tempoAtual = ((unsigned int)time(NULL) - inicioTempo) % 60;

		//printf("tempoAtual %d\n", tempoAtual);
		//printf("ProgsRt %c\n", progsRT[tempoAtual]);

		while (listaRealTime != NULL && progsRT[tempoAtual] == 1){
			if( tempoAtual == tempRealTime->realTimeI){
				execListaRealTime(tempRealTime);

				if(tempRealTime->prox == NULL){
					tempRealTime = listaRealTime;
				}
				else{
					tempRealTime = tempRealTime->prox;
				}
			}
		}

		execListaPrioridade(tempPrioridade);
		execListaRoundRobin(tempRoundRobin);
	}

	printf("Todos os programas foram executados na ordem correta pelo escalonador\n");

	// libera memoria compartilhada do processo
	shmdt(tipo);
	shmdt(prioridade);
	shmdt(realTimeI);
	shmdt(realTimeD);
	shmdt(novaLinha);
	shmdt(nomeExec);
	shmdt(finalArquivo);

	//Remove memoria compartilhada
	shmctl(segNomeExec, IPC_RMID, 0);
	shmctl(segTipo, IPC_RMID, 0);
	shmctl(segPrioridade, IPC_RMID, 0);
	shmctl(segRealTimeI, IPC_RMID, 0);
	shmctl(segRealTimeD, IPC_RMID, 0);
	shmctl(segNovaLinha, IPC_RMID, 0);
	shmctl(segFinal, IPC_RMID, 0);

	//Libera listas alocadas para o escalonador
	liberaProg(listaRealTime);
	liberaProg(listaRoundRobin);
	liberaProg(listaPrioridade);

	return 0;
}
