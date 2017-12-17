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

typedef enum {ARG, OUT, IN, PIPE} arg_stat;


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

	int tube_out[2];
	int tube_in[2];

	char* buffer= malloc(BUFFER_SIZE*sizeof(char));
	if(buffer == NULL ){
		exit(EXIT_FAILURE);
	}
	char** argv = malloc(ARG_SIZE*sizeof(char*));
	if(argv == NULL){
		free(buffer);
		exit(EXIT_FAILURE);
	}

	// On clean la console.
	if( fork() == 0){
		execlp("clear","clear",NULL);
		free(buffer); free(argv);
		exit(EXIT_FAILURE);
	}else{
		wait(&status);
	}

	accueil();


	while(1){
		size_t size=read(STDIN_FILENO,buffer,BUFFER_SIZE*sizeof(char));

		if( size > 0 ){
			buffer[size -1]='\0';

			if(strcmp(buffer,"exit")==0){
				free(buffer); free(argv);
				exit(1);
			}

			// separation des tokens
			char* token=NULL;
			char delim=' ';

			// strucure du fichier de sortie >
			char* fout=NULL;
			struct stat s_fout;
			int fd_out=-1;

			// strucure du fichier d'entré <
			char* fin=NULL;
			struct stat s_in;
			int fd_in=-1;

			int argc=1;
			argv[0]=strtok(buffer,&delim);

			while((token=strtok(NULL,&delim)) != NULL && argc < ARG_SIZE -1  ){
				if(strcmp(token,">")==0){
					fout=strtok(NULL,&delim);
					//affiche(fout,STDOUT_FILENO,strlen(fout));
				}else if(strcmp(token,"<")==0){
					fin=strtok(NULL,&delim);
				}else{
					argv[argc]=token;
					argc+=1;
				}
			}
			argv[argc]=NULL;


			// on ouvre un tube pour communiquer avec le fils
			// Pere <- FILS
			if( pipe(tube_out) != 0 || pipe(tube_in)){
					perror("tube");
					exit(-1);
			}
			clock_gettime(CLOCK_REALTIME,&time0);


			int pid=fork();
			if(pid == 0 ){
				// Fils, on ferme le canal venant du pere P->F
				// On connecte STDOUT_FILENO avec Out F -> P
				close(tube_out[0]);

				if(fin !=NULL){
					if( (fd_in=open(fin,O_RDONLY,0)) != -1 ){
							dup2(fd_in,STDIN_FILENO);
							close(fd_in);
					}
				}

				if(fout !=NULL){
					if( stat(fout,&s_fout) == -1){
						fd_out=creat(fout,O_WRONLY | S_IRWXU  );
					}else{
						fd_out=open(fout,O_WRONLY|O_TRUNC);
					}
					dup2(fd_out,STDOUT_FILENO);
					close(fd_out);

				}


				//status=execlp(argv[0],argv[0],NULL);
				status=execvp(argv[0],argv);

				free(argv);
				free(buffer);

				perror("Commande introuvable");
				exit(status);
			}
			else if(pid > 0){
				// Pere : on ferne le canal vers le fils : P -> F
				close(tube_out[1]);

				wait(&status);

				clock_gettime(CLOCK_REALTIME,&time1);
				long d=1000*((long)time1.tv_sec -(long)time0.tv_sec) + 0.000001*(time1.tv_nsec - time0.tv_nsec);

				/* if(fout !=NULL){
					if( stat(fout,&s_fout) == -1){
						fd_out=creat(fout,O_WRONLY | S_IRWXU  );
					}else{
						fd_out=open(fout,O_WRONLY|O_TRUNC);
					}
				} */


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
				if(fd_out != -1){
					close(fd_out);
					fout=NULL;
				}
				affiche("\n>",STDOUT_FILENO,2);
				//write(STDOUT_FILENO,buffer,strlen(buffer));
			}
			else {
				// Fork n'a pas marché
				free(argv);
				free(buffer);

				perror("Fork");
				exit(EXIT_FAILURE);
			}

		}

	}

	exit(1);
}
