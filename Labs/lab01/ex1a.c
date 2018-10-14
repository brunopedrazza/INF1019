#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>


int main (void){
      if (fork() != 0) {
         printf("pid pai: %d\n",getpid());
         waitpid( -1, 0, 0);
         printf("Pai finalizado.\n");
     
      } else {
         printf("pid filho: %d\n",getpid());
         printf("pid pai : %d\n",getppid());
         printf("Filho finalizado.\n");
         exit(0);
       }
}
