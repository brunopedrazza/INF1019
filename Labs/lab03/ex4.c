#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define EVER ;;

void childhandler(int signo);
void continua(int signo);
void para(int signo);
int delay;

int main (int argc,char *argv[]){
	int i,contador=0;
	int v[3];
	pid_t pid;
	signal(SIGCHLD, childhandler);
	if ((pid = fork()) < 0){
		fprintf(stderr,	"Erro ao criar filho\n");
		exit(-1);
	}

	/* parent */
	if(pid != 0){
		sscanf(argv[1],"%d", &delay);
		/* read delay from command line */
		sleep(delay);
		printf("Program %s exceeded limit of %d seconds!\n", argv[3], delay);
		kill(pid, SIGKILL);
	}	
	/* children */
	else{
		for (i=0; i<2; i++){
			if(pid == 0){
				v[0] = getppid();
				v[i+1]= getpid();
				signal(SIGCONT, KILL);			


				for(EVER);
				break;
			}
		}

	}
	return 0;
}
void childhandler(int signo)
/* Executed if child dies before parent */
{
	int status;
	pid_t pid = wait(&status);
	printf("Child %d terminated within %d seconds com estado %d.\n", pid, delay, status);
	exit(0);
}

void para(int signo){
	int status;
	pid_t pid = wait(&status);
	pause(20);
	printf("Child %d terminated within %d seconds com estado %d.\n", pid, delay, status);
	exit(0);
}

