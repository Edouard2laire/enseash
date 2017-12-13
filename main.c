#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <string.h>

#define BUFFER_SIZE 1024

int main() // int argc, char *argv[]
{

	const char *msg="Enseah \n Bienvenu dans le Shell Ensearque \n pour quiter, taper exit \n";

	if( write(STDOUT_FILENO,msg,strlen(msg)) == -1 ){
		perror("erreur d\'ecriture");
	}

	char* buffer= malloc(BUFFER_SIZE*sizeof(char));
	if( buffer != NULL ){
		while(1){
		size_t size=read(STDIN_FILENO,buffer,BUFFER_SIZE*sizeof(char));

		if( size > 0 ){
			buffer[size -1]='\0';

			if(strcmp(buffer,"exit")==0){
				exit(1);
			}

			int pid=fork();
			int status;
			if(pid == 0 ){
				status=execlp(buffer,buffer,NULL);
				if(status==-1){
					perror("Commande introuvable");
					exit(-1);
				}
			}
		}
		}


	}





	exit(1);
}
