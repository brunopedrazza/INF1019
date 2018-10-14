#include<sys/ipc.h> 
#include<sys/shm.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>


int main (void){ 
int seg1, status; 
char *m1;
// aloca a memória compartilhada
seg1 = shmget (8752, 8*sizeof (char), IPC_CREAT); 
// associa a memória compartilhada ao processo
m1 = (char *) shmat (seg1, 0, 0);

scanf("% s", m1);

// libera a memória compartilhada do processo
shmdt (m1);

return 0;
} 
