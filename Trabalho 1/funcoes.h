#ifndef FUNCOES_H
#define FUNCOES_H

typedef struct programa Prog;

struct programa {

	pid_t pid;
	char *nomeExec;
	int tipo;
	int prioridade;
	int realTimeI;
	int realTimeD;
	struct programa * prox;
	struct programa * ant;

};

Prog *insereProg (Prog * p, pid_t pid, char *nomeExec, int tipo, int prioridade, int realTimeI, int realTimeD);

void imprimePrioridades(Prog * atual);

#endif
