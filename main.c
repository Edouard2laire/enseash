#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() // int argc, char *argv[]
{

	const char *msg="Enseah \n Bienvenu dans le Shell Ensearque \n pour quiter, taper exit \n";

	if( write(STDOUT_FILENO,msg,strlen(msg)) == -1 ){
		perror("erreur d\'ecriture");
	}

	char* buffer= malloc(BUFFER_SIZE*sizeof(char));
	struct timespec time0,time1;
	if( buffer != NULL ){
		while(1){
		size_t size=read(STDIN_FILENO,buffer,BUFFER_SIZE*sizeof(char));

		if( size > 0 ){
			buffer[size -1]='\0';

			if(strcmp(buffer,"exit")==0){
				exit(1);
			}

			clock_gettime(CLOCK_REALTIME,&time0);
			int pid=fork();
			int status;
			if(pid > 0){
				int *status=NULL;
				waitpid(pid,status,WEXITED | WSTOPPED | WUNTRACED );
				clock_gettime(CLOCK_REALTIME,&time1);
				long d=(time1.tv_nsec - time0.tv_nsec);

				char *etaBuffer=malloc(100);
				if (WIFEXITED(status)) {
					sprintf(etaBuffer,"[F%d,%ld ns]",WEXITSTATUS(status),d);
				}else if (WIFSIGNALED(status)) {
					sprintf(etaBuffer,"[E%d,%ld ns]",WTERMSIG(status),d);
				}else if (WIFSTOPPED(status)) {
					sprintf(etaBuffer,"[S%d,%ld ns]",WSTOPSIG(status),d);
				}
				write(STDOUT_FILENO,etaBuffer,strlen(etaBuffer));
			}
			if(pid == 0 ){
				status=execlp(buffer,buffer,NULL);
				perror("Commande introuvable");
				exit(status);
			}
		}
		}


	}

	exit(1);
}
