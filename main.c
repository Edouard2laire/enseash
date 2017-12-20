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

const char *l1="\t\t--------------------------------------------\n";
const char *l2="\t\t|                     Enseah                |\n";
const char *l3="\t\t|       Bienvenue dans le Shell Ensearque   |\n";
const char *l4="\t\t|              Pour quitter, taper exit     |\n";
const char *l5="\t\t--------------------------------------------\n>";


#define DEBUG 0


typedef struct process {
	int pid;

	char** argv;
	int argc;

	int fd_out;
	int fd_in;
	int fc;

	int bloquant;


	int exitStatus;
	struct timespec time0,time1;

}process;

typedef struct process_list{
	process* current;
	struct process_list* next;
}process_list;

inline void affiche(const char* str, int filedes,size_t size){
	if( write(filedes,str,size) == -1 ){
		perror("Affichage : ");
		exit(EXIT_FAILURE);
	}
}


process* initProcess(){
	process *p= malloc(sizeof(process));
	if( p == NULL ){
		return NULL;
	}
	p->pid=-1;
	p->argc=0;
	p->fd_out=STDOUT_FILENO;
	p->fd_in=STDIN_FILENO;

	p->exitStatus=0;
	p->bloquant=1;
	p->fc=-1;



	p->argv= malloc(ARG_SIZE*sizeof(char*));
	if(p->argv==NULL){
		free(p);
		return NULL;
	}

	return p;
}

void lib(process *p){

	if( p != NULL){
		free(p->argv);
		free(p);
	}

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

void print_p(process *p,char* debug, int fd){
	sprintf(debug,"Processus %s\n=============\n",p->argv[0]);
	affiche(debug,fd,strlen(debug));
	sprintf(debug,"Entree: %d: Sortie : %d, fermee: %d\n",p->fd_in,p->fd_out,p->fc);
	affiche(debug,fd,strlen(debug));

	sprintf(debug,"Arguments %d\n=============\n",p->argc);
	affiche(debug,fd,strlen(debug));
	int i=0;
	for(i=0; i < p->argc ; ++i){
		sprintf(debug,"[%d]%s\n",i,p->argv[i]);
		affiche(debug,fd,strlen(debug));
	}


}

void lib_l(process_list* plist){

	process_list *tmp=plist;
	process_list *tmp0;

	while(tmp != NULL ){
		lib(tmp->current);
		tmp0=tmp->next;

		free(tmp);
		tmp=tmp0;
	}
}

void add(process_list* plist, process* p ){
	process_list* tmp=plist;

	if(tmp->current == NULL){
		tmp->current=p;
	}else{
		process_list* nw=initProcessList();
		nw->current=p;
		while( tmp->next != NULL ){
		    	 tmp=tmp->next;
		}
		tmp->next=nw;
	}

}

void showExit(process*p, char* buffer, int fd){
	long d=1000*((long)p->time1.tv_sec -(long)p->time0.tv_sec) + 0.000001*(&p->time1.tv_nsec - &p->time0.tv_nsec);


	if (WIFEXITED(p->exitStatus)) {
		sprintf(buffer,"%s terminé [%d,%ld ms]\n",p->argv[0],p->exitStatus,d);
	}else if (WIFSIGNALED(p->exitStatus)) {
		sprintf(buffer,"%s a recu le signal : %d .( %ld ms)\n",p->argv[0],p->exitStatus,d);
	}else if (WIFSTOPPED(p->exitStatus)) {
		sprintf(buffer,"%s a été stoppé : %d .( %ld ms)\n",p->argv[0],WSTOPSIG(p->exitStatus),d);
	}
	affiche(buffer,fd,strlen(buffer));
}


inline void accueil(){
	affiche(l1,STDOUT_FILENO,strlen(l1)); affiche(l2,STDOUT_FILENO,strlen(l2));
	affiche(l3,STDOUT_FILENO,strlen(l3)); affiche(l4,STDOUT_FILENO,strlen(l4));
	affiche(l5,STDOUT_FILENO,strlen(l5));
}

void showRunning(process_list* running,char *buffer, int fd){
	process_list* tmp=running;

	if(tmp->current == NULL){
		sprintf(buffer,"No process running \n");
		affiche(buffer,fd,strlen(buffer));

	}else{
		sprintf(buffer,"Process running : [ ");
		affiche(buffer,fd,strlen(buffer));

		while(tmp != NULL){
			if( tmp -> current != NULL) {
				sprintf(buffer," ( %d , %s ) ", tmp->current->pid, tmp->current->argv[0]);
				affiche(buffer,fd,strlen(buffer));
			}
			tmp=tmp->next;
		}
		sprintf(buffer,"] \n");
		affiche(buffer,fd,strlen(buffer));


	}


}
int main()
{

	int status;



	char* bufferIn= malloc(BUFFER_SIZE*sizeof(char));
	char* bufferOut= malloc(BUFFER_SIZE*sizeof(char));
	char* debug;

	if(DEBUG){ debug= malloc(BUFFER_SIZE*sizeof(char)); }

	if(bufferIn == NULL  || bufferOut == NULL){
		exit(EXIT_FAILURE);
	}

	process_list *p_list;
	process_list *p_list_current;

	process_list* running;

	process_list* temp;
	process *p;

	int tube[2];

	// On clean la console.
	if( fork() == 0){
		free(bufferIn);
		free(bufferOut);
		if(DEBUG){ free(debug);}

		execlp("clear","clear",NULL);

		exit(EXIT_FAILURE);
	}else{
		wait(&status);
	}

	accueil();


	running=initProcessList();

	while(1){

		affiche("\n>",STDOUT_FILENO,2);
		size_t size=read(STDIN_FILENO,bufferIn,BUFFER_SIZE*sizeof(char));
		if( size > 1 ){
			bufferIn[size -1]='\0';

			if(strcmp(bufferIn,"exit")==0){
				free(bufferOut);
				free(bufferIn);
				if(DEBUG){ free(debug); }
				exit(1);
			}

			// separation des tokens
			char* token=NULL;
			char delim=' ';
			struct stat s_fout;

			if( (p_list=initProcessList()) == NULL){
				free(bufferOut);free(bufferIn);
				if(DEBUG){ free(debug); }
				exit(-1);
			}
			p_list_current=p_list;
			if( (p_list->current = initProcess()) == NULL){
				free(p_list);free(bufferIn);
				if(DEBUG){ free(debug);}
				exit(EXIT_FAILURE);
			}

			token=strtok(bufferIn,&delim);

			p=p_list_current->current;
			p->argc=1;
			p->argv[0]=token;


			while((token=strtok(NULL,&delim)) != NULL && p->argc < ARG_SIZE -1  ){
				// STDOUT >
				if(strcmp(token,">")==0){
					token=strtok(NULL,&delim);
					if( stat(token,&s_fout) == -1){
						p->fd_out=creat(token,O_WRONLY | S_IRWXU  );
					}else{
						p->fd_out=open(token,O_WRONLY|O_TRUNC);
					}
				}
				//STDIN <
				else if(strcmp(token,"<")==0){
					token=strtok(NULL,&delim);
					p->fd_in=open(token,O_RDONLY,0);
				}
				// PIPE |
				else if(strcmp(token,"|")==0){
					token=strtok(NULL,&delim);
					pipe(tube);

					if(DEBUG){
						sprintf(debug,"[T0: %d, T1 : %d],%s\n",tube[0],tube[1],token);
						affiche(debug,STDOUT_FILENO,strlen(debug));
					}
					p->fd_out= dup(tube[1]); // Writing
					p->fc= dup(tube[0]); // Reading

					p->argv[p->argc]=NULL;

					temp=initProcessList();
					temp->current = initProcess();
					if(temp==NULL || temp->current==NULL){
						exit(-1);
					}
					temp->current->argc=1;
					temp->current->argv[0]=token;

					temp->current->fd_in=p->fc; // Reading
					temp->current->fc=p->fd_out; // Writing

					p_list_current->next=temp;
					p_list_current=temp;
					p=p_list_current->current;

					close(tube[0]);
					close(tube[1]);

				}
				else if(strcmp(token,"&")==0){
					p->bloquant=0;
				}
				else{
					p->argv[p->argc]=token;
					p->argc+=1;
				}
			}
			p->argv[p->argc]=NULL;

			p_list_current=p_list;

			do{
				p=p_list_current->current;
				clock_gettime(CLOCK_REALTIME,&p->time0);
				int pid=fork();
				if(pid == 0 ){
					if(DEBUG){print_p(p,debug,STDOUT_FILENO);}

					if(p->fc > -1){
						close(p->fc);
					}

					if(p->fd_in != STDIN_FILENO){
						if( dup2(p->fd_in,STDIN_FILENO) == -1){
							perror("dup entrée");
							exit(-1);
						}
						close(p->fd_in);
					}

					if(p->fd_out !=STDOUT_FILENO){
						if( dup2(p->fd_out,STDOUT_FILENO) == -1){
							perror("dup sortie");
							exit(-1);
						}
						close(p->fd_out);

					}

					status=execvp(p->argv[0],p->argv);

					lib_l(p_list_current);

					free(bufferIn);
					free(bufferOut);
					if(DEBUG){ free(debug);}

					perror("Commande introuvable");
					exit(status);
				}
				else if((p->pid=pid) > 0){
					struct timespec tf;
					if(p->bloquant){
						waitpid(p->pid,&status,WUNTRACED|WCONTINUED);
						//pid=wait(&status);
						clock_gettime(CLOCK_REALTIME,&tf);

						p->time1=tf;
						p->exitStatus=status;
						showExit(p, bufferOut, STDOUT_FILENO );
						lib(p);

					}else{
						add(running,p);
						showRunning(running,bufferOut,STDOUT_FILENO);
					}

					temp=p_list_current;
					p_list_current = p_list_current->next;
					free(temp);
				}
				else { // Fork n'a pas marché
					lib_l(p_list); free(bufferIn); free(bufferOut);if(DEBUG){ free(debug); }
					perror("Fork"); exit(EXIT_FAILURE);
				}
			}while( p_list_current != NULL );

		}
		temp=running;
		struct timespec tf;
		while(temp != NULL ){
			if( temp -> current != NULL){
				if( waitpid(temp -> current->pid,&status,WNOHANG|WUNTRACED|WCONTINUED) > 0){
					clock_gettime(CLOCK_REALTIME,&tf);

					temp -> current->time1=tf;
					temp -> current->exitStatus=status;
					showExit(temp -> current, bufferOut, STDOUT_FILENO );
					lib(temp->current);
					if(temp->next !=NULL ){
							temp-> current = temp->next->current;
							temp->next = temp->next->next;
					}else{
						temp->current=NULL;
						temp->next =NULL;
					}

				}else{
					temp = temp ->next ;
				}
			}else{
				temp = temp ->next ;
			}

		}

	}

	exit(1);
}
