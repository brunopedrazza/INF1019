#include<sys/ipc.h> 
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>


int main (void){ 
int seg1; 
char *m1;

// associa a memória compartilhada ao processo
m1 = (char *) shmat (8752, 0, 0);

printf("Mensagem do dia: \n %s\n", *m1);

// libera a memória compartilhada do processo
shmdt (m1);

// libera a memória compartilhada
shmctl (seg1, IPC_RMID, 0); 

return 0;
} 
