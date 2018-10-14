#include <stdio.h>
#include <unistd.h>

int main (void){
	int i;
	
	for (i = 0; i < 3; i++){
		printf("Teste 1.\n");
		sleep(1);

	}
	
	return 0;
}
