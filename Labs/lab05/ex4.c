#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main (void) { 
   int fd[2];
   int n;
   
   if (pipe(fd) < 0){
      puts ("Erro ao abrir os pipes"); 
      exit (-1);
   }
   
   if (fork() == 0) {
      close(fd[1]);
      while (1){ 
         read(fd[0], &n, sizeof(int));
         printf("Filho1 : %d\n",n);
         sleep(2);
      }
   }
   
   else {
      if (fork() == 0) {
         close(fd[1]);
         while (1){
            read(fd[0], &n, sizeof(int));
            printf("Filho2 : %d\n",n);
            sleep(2);
         }
      }

      else {
         n = 0; 
         close(fd[0]);
         while (1){ 
            write(fd[1], &n, sizeof(int));
            n += 5;
            sleep(1);
         } 
      }
   }
   
   close(fd[0]);
   close(fd[1]);

   return 0;
}

/*
Filho1 : 0
Filho2 : 5
Filho1 : 10
Filho2 : 15
Filho1 : 20
Filho2 : 25
Filho1 : 30
Filho2 : 35
Filho1 : 40
Filho2 : 45
Filho1 : 50
Filho2 : 55
Filho1 : 60
Filho2 : 65
Filho1 : 70
Filho2 : 75
Filho1 : 80
Filho2 : 85
Filho1 : 90
Filho2 : 95
Filho1 : 100
*/
