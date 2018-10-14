#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define N 20


int main(void) {

   int entrada, saida;
   /* descritor a ser duplicado  */
   int retorno;
   /* valor de retorno de dup  */
   int retorno2;
   /* valor de retorno de dup2 */
   char str[N];
   
   if ((entrada=open("entrada",O_RDWR|O_CREAT,0666)) == -1) { 
      perror("Error open()"); 
      return -1; 
   }
   
   
   if ((saida=open("saida",O_RDWR|O_CREAT|O_TRUNC,0666)) == -1) { 
      perror("Error open()"); 
      return -1; 
   } 
   
   close(0);
   /* fechamento da entrada stdin */
   
   if ((retorno = dup(entrada)) == -1) {
      /* duplicacao de stdin (menor descritor fechado) */
      perror("Error dup()"); 
      return -2; 
   }
   
   scanf(" %20s",str);
   close(0);
    
   if ((retorno2 = dup2(saida,1)) == -1) {
      /* duplicacao de stdout */
      perror("Error dup2()"); 
      return -3; 
   }
   
   printf("%s \n",str);
   
   return 0; 
} 
