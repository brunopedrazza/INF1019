#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "funcoes.h"

Prog *insereProg(Prog * lp, pid_t pid, char *nomeExec, int tipo, int prioridade, int realTimeI, int realTimeD) {

	Prog * np = (Prog *)malloc(sizeof(Prog));
	np->pid = pid;
	np->nomeExec = nomeExec;
	np->ant = NULL;
	np->prox = NULL;
	np->tipo = tipo;
	np->prioridade = prioridade;
	np->realTimeI = realTimeI;
	np->realTimeD = realTimeD;

	// Round-Robin (Inserção no final da lista)
	if (tipo == 2) {
		Prog * RR = lp;
		if (RR != NULL) {
			while (RR->prox != NULL) {
				RR = RR->prox;
			}
			RR->prox = np;
			np->ant = RR;
			return lp;
		}
		else {
			return np;
		}
	}

	// Prioridade (Coloca no início da lista passada como parametro, vai andando pra frente)
	else if (tipo == 1) {
		if (lp != NULL) {
			lp->ant = np;
		}
		np->prox = lp;
	}

	//Real Time (Coloca no inicio da lista passada como parametro, ja inicializado com posicao correta)
	else if (tipo == 3) {
		if (lp != NULL) {
			lp->ant = np;
		}
		np->prox = lp;
	}

	return np;
}

void imprimePrioridades(Prog * atual) {

	Prog* anterior = atual;
	while( anterior != NULL ) {
		printf("PID: %d  prioridade:%d\n", anterior->pid, anterior->prioridade);
		anterior = anterior->prox;
	}
}
