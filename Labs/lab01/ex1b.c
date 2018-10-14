#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


int main (void){

      int var = 0;
      if (fork() != 0) {
         var += 50;
         printf("Pai %d - %d \n",getpid(), var);
         waitpid( -1, 0, 0);
         printf("Pai apos %d - %d \n",getpid(), var);
     
      } else {
         var += 10;
         printf("Filho %d - %d \n",getpid(), var);
         exit(0);
       }
       
      
}
