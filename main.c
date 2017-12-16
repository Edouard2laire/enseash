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
#define ARG_SIZE 10

int main()
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
			char* token;
			char delim=' ';
			char* str= malloc(BUFFER_SIZE*sizeof(char));
			strcpy(str,buffer);

			int argc=1;
			char** argv = malloc(ARG_SIZE*sizeof(char*));
			argv[0]=strtok(str,&delim);

			write(STDOUT_FILENO,argv[0],strlen(argv[0]));
			write(STDOUT_FILENO,"\n",2);

			while((token=strtok(NULL,&delim)) != NULL && argc < ARG_SIZE -1  ){
				argv[argc]=token;
				write(STDOUT_FILENO,token,strlen(token));
				write(STDOUT_FILENO,"\n",2);
				argc+=1;
			}
			argv[argc]=NULL;

			clock_gettime(CLOCK_REALTIME,&time0);
			int pid=fork();
			int status;
			if(pid > 0){
				int status;
				//waitpid(pid,&status,WEXITED | WSTOPPED | WUNTRACED );
				wait(&status);
				clock_gettime(CLOCK_REALTIME,&time1);
				long d=1000*((long)time1.tv_sec -(long)time0.tv_sec) + 0.000001*(time1.tv_nsec - time0.tv_nsec);

				char *etaBuffer=malloc(100);
				if (WIFEXITED(status)) {
					sprintf(etaBuffer,"[F%d,%ld ms]",status,d);
				}else if (WIFSIGNALED(status)) {
					sprintf(etaBuffer,"[E%d,%ld ms]",WTERMSIG(status),d);
				}else if (WIFSTOPPED(status)) {
					sprintf(etaBuffer,"[S%d,%ld ms]",WSTOPSIG(status),d);
				}
				write(STDOUT_FILENO,etaBuffer,strlen(etaBuffer));
			}
			if(pid == 0 ){

				//status=execlp(argv[0],argv[0],NULL);
				status=execvp(argv[0],argv);
				perror("Commande introuvable");
				exit(status);
			}
		}
		}


	}

	exit(1);
}
