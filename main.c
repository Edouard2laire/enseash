#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "main.h"



process* initProcess(){
	process *p= malloc(sizeof(process));
	if( p == NULL ){
		return NULL;
	}
	p->pid=-1;
	p->argc=0;
	p->fd_out=STDOUT_FILENO;
	p->fd_in=STDIN_FILENO;

	p->argv= malloc(ARG_SIZE*sizeof(char*));
	if(p->argv==NULL){
		free(p);
		return NULL;
	}

	return p;
}

void lib(process *p){
	free(p->argv);
	free(p);

}
inline void affiche(const char* str, int filedes,size_t size){
	if( write(filedes,str,size) == -1 ){
		perror("Affichage : ");
		exit(EXIT_FAILURE);
	}
}
inline void accueil(){
	affiche(l1,STDOUT_FILENO,strlen(l1)); affiche(l2,STDOUT_FILENO,strlen(l2));
	affiche(l3,STDOUT_FILENO,strlen(l3)); affiche(l4,STDOUT_FILENO,strlen(l4));
	affiche(l5,STDOUT_FILENO,strlen(l5));
}

int main()
{

	int status;
	struct timespec time0,time1;


	char* buffer= malloc(BUFFER_SIZE*sizeof(char));
	if(buffer == NULL ){
		exit(EXIT_FAILURE);
	}

	process *p;
	if( (p= initProcess()) == NULL){
		free(buffer);
		exit(EXIT_FAILURE);
	}


	// On clean la console.
	if( fork() == 0){
		execlp("clear","clear",NULL);
		free(buffer);
		exit(EXIT_FAILURE);
	}else{
		wait(&status);
	}

	accueil();



	while(1){
		size_t size=read(STDIN_FILENO,buffer,BUFFER_SIZE*sizeof(char));
		lib(p);

		if( (p= initProcess()) == NULL){
			free(buffer);
			exit(EXIT_FAILURE);
		}

		if( size > 0 ){
			buffer[size -1]='\0';

			if(strcmp(buffer,"exit")==0){
				free(buffer); lib(p);
				exit(1);
			}

			// separation des tokens
			char* token=NULL;
			char delim=' ';
			struct stat s_fout;




			p->argc=1;
			p->argv[0]=strtok(buffer,&delim);

			while((token=strtok(NULL,&delim)) != NULL && p->argc < ARG_SIZE -1  ){
				if(strcmp(token,">")==0){
					token=strtok(NULL,&delim);
					if( stat(token,&s_fout) == -1){
						p->fd_out=creat(token,O_WRONLY | S_IRWXU  );
					}else{
						p->fd_out=open(token,O_WRONLY|O_TRUNC);
					}
				}else if(strcmp(token,"<")==0){
					token=strtok(NULL,&delim);
					p->fd_in=open(token,O_RDONLY,0);
				}else{
					p->argv[p->argc]=token;
					p->argc+=1;
				}
			}
			p->argv[p->argc]=NULL;



			clock_gettime(CLOCK_REALTIME,&time0);


			int pid=fork();
			if(pid == 0 ){
				if(p->fd_in != STDIN_FILENO){
					dup2(p->fd_in,STDIN_FILENO);
					close(p->fd_in);
				}
				if(p->fd_out !=STDOUT_FILENO){
					dup2(p->fd_out,STDOUT_FILENO);
					close(p->fd_out);
				}


				//status=execlp(argv[0],argv[0],NULL);
				status=execvp(p->argv[0],p->argv);

				lib(p);
				free(buffer);

				perror("Commande introuvable");
				exit(status);
			}
			else if((p->pid=pid) > 0){

				wait(&status);

				clock_gettime(CLOCK_REALTIME,&time1);
				long d=1000*((long)time1.tv_sec -(long)time0.tv_sec) + 0.000001*(time1.tv_nsec - time0.tv_nsec);


				if (WIFEXITED(status)) {
					sprintf(buffer,"[F%d,%ld ms]",status,d);
				}else if (WIFSIGNALED(status)) {
					sprintf(buffer,"[E%d,%ld ms]",WTERMSIG(status),d);
				}else if (WIFSTOPPED(status)) {
					sprintf(buffer,"[S%d,%ld ms]",WSTOPSIG(status),d);
				}
				affiche(buffer,STDOUT_FILENO,strlen(buffer));

				/*
				while( (size=read(tube_out[0],buffer,BUFFER_SIZE*sizeof(char))) >0 ){
					if( fd_out !=-1 ){ affiche(buffer,fd_out,size);}
					affiche(buffer,STDOUT_FILENO,size);
				}
				close(tube_out[0]); */
				affiche("\n>",STDOUT_FILENO,2);
				//write(STDOUT_FILENO,buffer,strlen(buffer));
			}
			else {
				// Fork n'a pas marché
				lib(p);
				free(buffer);

				perror("Fork");
				exit(EXIT_FAILURE);
			}

		}

	}

	exit(1);
}
