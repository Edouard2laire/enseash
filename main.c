#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <string.h>

int main() // int argc, char *argv[]
{

	const char *msg="Enseah \n Bienvenu dans le Shell Ensearque \n pour quiter, taper exit \n";

	if( write(STDOUT_FILENO,msg,strlen(msg)) == -1 ){
		perror("erreur d\'ecriture");
	}

	while(1){
	char* buffer= malloc(100*sizeof(char));
	if( buffer != NULL ){
		size_t size=read(STDIN_FILENO,buffer,100);

		buffer[size -1]='\0';

		int pid=fork();
		int status;
		if(pid > 0 ){
			status=execlp(buffer,"enseash",NULL);
			if(status==-1){
				perror("Erreur lors de l'exec");
			}
		}

	}
	}





	exit(1);
}
