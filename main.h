#ifndef MAIN_H_
#define MAIN_H_

#define BUFFER_SIZE 1024
#define ARG_SIZE 10

const char *l1="\t\t--------------------------------------------\n";
const char *l2="\t\t|                     Enseah                |\n";
const char *l3="\t\t|       Bienvenue dans le Shell Ensearque   |\n";
const char *l4="\t\t|              Pour quitter, taper exit     |\n";
const char *l5="\t\t--------------------------------------------\n>";



typedef struct process {
	int pid;
	char** argv;
	int argc;

	int fd_out;
	int fc_out;

	int fd_in;
	int fc_in;

}process;

typedef struct process_list{
	process* current;
	struct process_list* next;
}process_list;

process* initProcess();
void lib(process *p);
int isValid(process *p);

process_list* initProcessList();

inline void affiche(const char* str, int filedes,size_t size);
inline void accueil();



#endif /* MAIN_H_ */
