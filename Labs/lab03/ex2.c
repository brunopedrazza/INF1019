#include <stdio.h>
#include <unistd.h>
#include <signal.h>
void handler(int sinal);
int main (void){
	signal(SIGKILL, handler);
	puts ("vou parar...");
	pause();
	puts ("Continuei!");
	return 0;
}
void handler(int sinal){
	printf ("Sinal %d recebido\n", sinal);
}


//Não é possível interomper o SIGKILL uma vez que ele foi justamente criado para interromper sem excessões os processos. Uma possível estratégia para evitar ataques ou processos que prejudiquem o sistema.

//./ex2 &
//[4] 3479
//kill -s KILL 3479
//[4]    Killed                        ./ex2


