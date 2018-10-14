#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main (void) { 
   int fd[2];
   int entrada;
   pid_t pid;
   
   if (pipe(fd) < 0){
      puts ("Erro ao abrir os pipes"); 
      exit (-1);
   }
   
   if ((entrada=open("arqex3",O_RDWR|O_CREAT,0666)) == -1) { 
      perror("Error open()"); 
      return -1; 
   }
   
   if((pid = fork()) < 0){
      printf("Erro ao abrir fork\n");
      exit(1);
   
   }
   
   else if (fork() == 0) { 
      dup2(fd[1],1);
      printf("ls\n");
      char *argv[] = {"ls", NULL};
      execv("/bin/ls",argv);
      printf("Falha ao executar ls\n");
      exit(1);
   } 
   
   else {
      int aux;  
      close(fd[1]);
      waitpid(pid,&aux,0);
      close(0);
      dup2(fd[0],0);
      dup2(entrada,1);
      printf("wc\n");
      char *argv[] = {"wc", NULL};
      execv("/usr/bin/wc",argv);
      printf("Falha ao executar wc\n");
      exit(1);
   }
   
   close(fd[0]);
   close(fd[1]);
   return 0;
}
