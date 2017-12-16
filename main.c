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

inline void affiche(const char* str, int filedes){
	if( write(filedes,str,strlen(str)) == -1 ){
		perror("Affichage : ");
		exit(EXIT_FAILURE);
	}
}


int main()
{
	const char *l1="\t--------------------------------------------\n";
	const char *l2="\t|                     Enseah                |\n";
	const char *l3="\t|       Bienvenu dans le Shell Ensearque    |\n";
	const char *l4="\t|       Pour quiter, taper exit             |\n";
	const char *l5="\t--------------------------------------------\n";
	int status;

	// On clean la console.
	if( fork() == 0){
		execlp("clear","clear",NULL);
		exit(EXIT_FAILURE)
	}else{
		wait(&status);
	}

	affiche(l1,STDOUT_FILENO); affiche(l2,STDOUT_FILENO);
	affiche(l3,STDOUT_FILENO); affiche(l4,STDOUT_FILENO);
	affiche(l5,STDOUT_FILENO);


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

			int argc=1;
			char** argv = malloc(ARG_SIZE*sizeof(char*));
			argv[0]=strtok(buffer,&delim);

			//write(STDOUT_FILENO,argv[0],strlen(argv[0]));
			//write(STDOUT_FILENO,"\n",2);

			while((token=strtok(NULL,&delim)) != NULL && argc < ARG_SIZE -1  ){
				argv[argc]=token;
				//write(STDOUT_FILENO,token,strlen(token));
				//write(STDOUT_FILENO,"\n",2);
				argc+=1;
			}
			argv[argc]=NULL;

			clock_gettime(CLOCK_REALTIME,&time0);
			int pid=fork();
			if(pid > 0){
				int status;
				//waitpid(pid,&status,WEXITED | WSTOPPED | WUNTRACED );
				wait(&status);
				clock_gettime(CLOCK_REALTIME,&time1);
				long d=1000*((long)time1.tv_sec -(long)time0.tv_sec) + 0.000001*(time1.tv_nsec - time0.tv_nsec);

				if (WIFEXITED(status)) {
					sprintf(buffer,"[F%d,%ld ms] >",status,d);
				}else if (WIFSIGNALED(status)) {
					sprintf(buffer,"[E%d,%ld ms] >",WTERMSIG(status),d);
				}else if (WIFSTOPPED(status)) {
					sprintf(buffer,"[S%d,%ld ms] >",WSTOPSIG(status),d);
				}
				affiche(buffer,STDOUT_FILENO);
				//write(STDOUT_FILENO,buffer,strlen(buffer));
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
