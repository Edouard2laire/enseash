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

	p->fc_in=-1;
	p->fc_out=-1;

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
int isValid(process *p){
	return (p !=NULL && (p->argc) >= 1);
}

process_list* initProcessList(){
	process_list* p_list=malloc(sizeof(process_list) );
	p_list->current=NULL;
	p_list->next=NULL;

	return p_list;
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

	char* debug= malloc(BUFFER_SIZE*sizeof(char));
	if(buffer == NULL ){
		exit(EXIT_FAILURE);
	}

	process_list *p_list;
	process_list *p_list_current;
	process_list* temp;
	process *p;

	int tube[2];

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
		affiche("\n>",STDOUT_FILENO,2);
		size_t size=read(STDIN_FILENO,buffer,BUFFER_SIZE*sizeof(char));
		if( size > 1 ){
			buffer[size -1]='\0';

			if(strcmp(buffer,"exit")==0){
				free(buffer);
				exit(1);
			}

			// separation des tokens
			char* token=NULL;
			char delim=' ';
			struct stat s_fout;

			if( (p_list=initProcessList()) == NULL){
				exit(-1);
			}
			p_list_current=p_list;
			if( (p_list->current = initProcess()) == NULL){
				free(p_list);
				free(buffer);
				exit(EXIT_FAILURE);
			}
			token=strtok(buffer,&delim);

			p=p_list_current->current;
			p->argc=1;
			p->argv[0]=token;

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
				}else if(strcmp(token,"|")==0){
					pipe(tube);

					sprintf(debug,"[T0: %d, T1 : %d]",tube[0],tube[1]);
					affiche(debug,STDOUT_FILENO,strlen(debug));

					p->fd_out=tube[1];// dup(tube[1]); // Writing
					p->fc_out=tube[0];//dup(tube[0]); // Reading



					p->argv[p->argc]=NULL;

					temp=initProcessList();
					temp->current = initProcess();

					temp->current->fd_in=p->fc_out; // Reading
					temp->current->fc_in=p->fd_out; // Writing

					p_list_current->next=temp;
					p_list_current=temp;
					p=temp->current;

					//close(tube[0]);
					//close(tube[1]);

				}else{
					p->argv[p->argc]=token;
					p->argc+=1;


				}
			}
			p->argv[p->argc]=NULL;


			p_list_current=p_list;
			p=p_list->current;

			while(p != NULL ){
			if( isValid(p)){

				clock_gettime(CLOCK_REALTIME,&time0);
				int pid=fork();
				if(pid == 0 ){
					sprintf(debug,"Processus %s\n",p->argv[0]);
					affiche(debug,STDOUT_FILENO,strlen(debug));

					if(p->fc_in > -1){

						sprintf(debug,"Tube Entrée: Ferme %d DUP %d->STDIN \n",p->fc_in,p->fd_in);
						affiche(debug,STDOUT_FILENO,strlen(debug));
						close(p->fc_in);
						if( dup2(p->fd_in,STDIN_FILENO) == -1){
							perror("dup entrée");
						}
						close(p->fd_in);
					}
					else if(p->fd_in != STDIN_FILENO){
						dup2(p->fd_in,STDIN_FILENO);
						close(p->fd_in);
					}


					if(p->fc_out > -1){
						sprintf(debug,"Tube Sortie :ferme %d DUP %d->STDOUT \n",p->fc_out,p->fd_out);
						affiche(debug,STDOUT_FILENO,strlen(debug));
						close(p->fc_out);
						if( dup2(p->fd_out,STDOUT_FILENO) == -1){
							perror("dup sortie");
						}
						close(p->fd_out);
					}
					else if(p->fd_out !=STDOUT_FILENO){
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

					if(p->fc_in > -1){
						close(p->fd_in);
						close(p->fc_in);
					}
					if(p->fc_out > -1){
						close(p->fd_in);
						close(p->fc_in);
					}
					lib(p);
					temp=p_list_current;
					p_list_current=p_list_current->next;
					free(temp);
					if(p_list_current==NULL){
						p=NULL;
					}else{
						p=p_list_current->current;
					}

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

		}



	}

	exit(1);
}
