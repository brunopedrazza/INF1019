#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#define EVER ;;

void intHandler(int sinal);
void quitHandler(int sinal);

int main (void){
	void (*p)(int);
	// ponteiro para função que recebe int como parâmetro
	//p = signal(SIGINT, intHandler);
	printf("Endereco do manipulador anterior %p\n", p);
	//p = signal(SIGQUIT, quitHandler);
	printf("Endereco do manipulador anterior %p\n", p);
	puts ("Ctrl-C desabilitado. Use Ctrl-\\para terminar");
	for(EVER);
}

void intHandler(int sinal){
	printf ("Você pressionou Ctrl-C (%d)", sinal);
}

void quitHandler(int sinal){
	puts ("Terminando o processo...");
	exit (0);
}


//Primeiro com os sinais o ctrl-c fica desabilitado e o programa só é terminado ao pressionar o ctrl-\, o número de ctrl-c pressionados após a execução são contabilizados e mostrados no final da execução.
//Sem os signal() o ctrl-c continua habilitado e aborta o programa como normalmente, e o ctrl-\ também da um quit com (core dumped)
