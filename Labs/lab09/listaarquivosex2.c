#include <sys/types.h> 
#include <sys/dir.h> 
#include <sys/param.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FALSE 0 
#define TRUE 1


extern  int alphasort(); 
char pathname[MAXPATHLEN];


int file_select(struct direct *entry) 
{ 
    if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))  
          return (FALSE); 
    else 
          return (TRUE); 
} 

int main(void)  {  
   int count, i; 
   struct direct **files; 
   int file_select();
   struct stat *buf;
   char pathnameaux[MAXPATHLEN];
   
   if (getcwd(pathname,MAXPATHLEN) == NULL )  {  printf("Error getting path\n"); exit(0); } 
   printf("Current Working Directory = %s\n",pathname); 
   count = scandir( pathname, &files, file_select, alphasort); 
   
   /* If no files found, make a non-selectable menu item */ 
   if (count <= 0) {
      printf("No files in this directory\n"); exit(0); 
   } 
   printf("Number of files = %d\n",count);
    
   for (i=1;i<count+1;++i) {
      strcpy(pathnameaux,pathname);
      printf("%s\t", files[i-1]->d_name);
      strcat(pathnameaux,"/");
      strcat(pathnameaux,files[i-1]->d_name);
      if (stat(pathnameaux,buf) != 0) printf("Erro.\n");
      else{
         printf("inode: %d\t",buf->st_ino);
         printf("size: %d\t",buf->st_size);
         printf("age: %d days\t",(time(NULL) - (buf->st_mtime))/86400);
         // 86400 segundos em 1 dia
         printf("n links: %d\n",buf->st_nlink);
      }
   }
   
   printf("\n"); /* flush buffer */
   return 0; 
}

