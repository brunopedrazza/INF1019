#include<sys/ipc.h> 
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>


int main (void){ 
int seg1, seg2, seg3, *m1, *m2, *m3, id, pid, status; 
// aloca a memória compartilhada
seg1 = shmget (IPC_PRIVATE, 4*sizeof (int), IPC_CREAT); 
seg2 = shmget (IPC_PRIVATE, 4*sizeof (int), IPC_CREAT); 
seg3 = shmget (IPC_PRIVATE, 4*sizeof (int), IPC_CREAT); 

// associa a memória compartilhada ao processo
m1 = (int *) shmat (seg1, 0, 0);
m2 = (int *) shmat (seg2, 0, 0);
m3 = (int *) shmat (seg3, 0, 0);

//Preencher matrizes
*m1 = 2;
printf ("Pai%d\n", *m1); 
*(m1+4) = 3;
*(m1+8) = 4;
*(m1+12) = 5;

*m2 = 1;
*(m2+4) = 0;
*(m2+8) = 2;
*(m2+12) = 2;

// comparar o retorno com -1
if ((id = fork()) < 0) { 
	puts ("Erro na criação do novo processo"); 
	exit (-2); 
} 

else if (id == 0) { 
	*m3 = *m1 + *m2; 
	*(m3+4) = *(m1+4) + *(m2+4); 	
	printf ("Filho\n"); 
} 

else{ 
	pid = wait (&status); 
	*(m3+8) = *(m1+8) + *(m2+8); 
	*(m3+12) = *(m1+12) + *(m2+12); 	
	printf ("Pai\n"); 
} 

printf("Matriz Soma = \n %d \t %d\n", *m3, *(m3+4));
printf("%d \t %d\n", *(m3+8), *(m3+12));

// libera a memória compartilhada do processo
shmdt (m1);
shmdt (m2);
shmdt (m3); 

// libera a memória compartilhada
shmctl (seg1, IPC_RMID, 0); 
shmctl (seg2, IPC_RMID, 0); 
shmctl (seg3, IPC_RMID, 0); 
return 0;
} 
