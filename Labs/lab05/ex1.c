#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (void) { 
   int fd[2];
   int nr, nw;
   
   char text[] = "Bom dia.";
   char* textR = (char *)malloc(strlen(text) * sizeof (char));
   
   if (pipe(fd) < 0){
      puts ("Erro ao abrir os pipes"); 
      exit (-1);
   }
    
   if (fork() == 0) { 
      close(fd[0]); 
      nw = write(fd[1], text, strlen(text));
      printf("%d dados escritos\n", nw);
   } 
   
   else {  
      close(fd[1]); 
      nr = read(fd[0], textR, sizeof(textR));  
      printf("%s\n",textR);
      printf("%d dados lidos\n", nr);
   }
   
   close(fd[0]);
   close(fd[1]);
   free(textR);

   return 0;
}
