#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


int main (void){

      int var;
      
      if (fork() != 0) {
         printf("Pai %d\n",getpid());
         for (var = 1; var <= 50; var++){
            printf("%d\n",var);
            sleep(2);
         }
         printf("Processo pai vai morrer.\n");
         waitpid( -1, 0, 0);
     
      } else {
         printf("Filho1 foi criado.\n");
         printf("Filho1 %d\n",getpid());
         printf("Filho1 pai %d\n",getppid());
         for (var = 100; var <= 200; var++){
            printf("%d\n",var);
            sleep(1);
         }
         printf("Filho1 vai morrer.\n");
         
         if (fork() != 0){
            waitpid( -1, 0, 0);
         
         } else {
            printf("Neto1 foi criado.\n");
            printf("Neto1 %d\n",getpid());
            printf("Neto1 pai%d\n",getppid());
            for (var = 300; var <= 350; var++){
               printf("%d\n",var);
               sleep(2);
            }
            printf("Neto1 vai morrer.\n");
            exit(0);
         }       
         exit(0);
       }    
      
}
