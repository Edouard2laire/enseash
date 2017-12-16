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


inline void affiche(const char* str, int filedes){
	if( write(filedes,str,strlen(str)) == -1 ){
		perror("Affichage : ");
		exit(EXIT_FAILURE);
	}
}
inline void accueil(){
	affiche(l1,STDOUT_FILENO); affiche(l2,STDOUT_FILENO);
	affiche(l3,STDOUT_FILENO); affiche(l4,STDOUT_FILENO);
	affiche(l5,STDOUT_FILENO);
}

int main()
{

	int status;
	struct timespec time0,time1;
	int tube[2];


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
			char* token;
			char delim=' ';

			int argc=1;
			argv[0]=strtok(buffer,&delim);

			while((token=strtok(NULL,&delim)) != NULL && argc < ARG_SIZE -1  ){
				argv[argc]=token;
				argc+=1;

			}
			argv[argc]=NULL;


			// on ouvre un tube pour communiquer avec le fils
			// Pere <- FILS
			if( pipe(tube) != 0){
					perror("tube");
					exit(-1);
			}else{
			}
			clock_gettime(CLOCK_REALTIME,&time0);
			int pid=fork();
			if(pid == 0 ){
				// Fils, on ferme le canal venant du pere P->F
				// On connecte STDOUT_FILENO avec Out F -> P
				close(tube[0]);
				dup2(tube[1],STDOUT_FILENO);

				//status=execlp(argv[0],argv[0],NULL);
				status=execvp(argv[0],argv);

				free(argv);
				free(buffer);

				perror("Commande introuvable");
				exit(status);
			}
			else if(pid > 0){
				// Pere : on ferne le canal vers le fils : P -> F
				close(tube[1]);
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
				affiche(buffer,STDOUT_FILENO);

				int fd=creat("123.txt", O_RDWR | S_IRWXU);

				while( read(tube[0],buffer,BUFFER_SIZE*sizeof(char)) >0 ){
					affiche(buffer,fd);
					affiche(buffer,STDOUT_FILENO);
				}
				close(fd);
				affiche("\n>",STDOUT_FILENO);
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
