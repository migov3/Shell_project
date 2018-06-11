/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA
Nombre y apellidos: Miguel Gómez

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell
	(then type ^D to exit program)
//Tarea 1, 2, 3, 4, mask, time-out, respawnable.
**/

#include "job_control.h"   // remember to compile with module job_control.c
#include <string.h>
#include <stdio.h>
#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */
#define ROJO "\x1b[31;1;1m"
#define NEGRO "\x1b[0m"
#define VERDE "\x1b[32;1;1m"
#define AZUL "\x1b[34;1;1m"
#define CIAN "\x1b[36;1;1m"
#define MARRON "\x1b[33;1;1m"
#define PURPURA "\x1b[35;1;1m"

	job * lista;
	int ret,alarma;
	int masks[20];
	char path[MAX_LINE/2];

/*
void children(){
	char pathC[MAX_LINE/2];
	int nchildren=0, nthreads=0;
	char command[20];
	getcwd(pathC,sizeof(path));
	char buffer[100];
	int res;
	char * pid=args[1];
	chdir("/proc/");
	ret=chdir(pid);
	if(ret!=0){
		printf("Error: pid: %s not found",pid);
	}else{
		FILE * fp = fopen("stat","r");
		fseek(fp, 0, SEEK_SET); //al leer stat, PPID: 4, offset: sizeof(int)+sizeof(char[])+
		fread(buffer, sizeof(char), 48, fp);
		printf("%s\n", buffer);
		nchildren=buffer[20];
		printf("#CHILDREN: %i\n",nchildren/32);
		fflush(stdout);
		fclose(fp);
	}
	fflush(stdout);
	chdir(pathC);
}
*/

/*
void pipes(char * args[]){
	{
		int descf[2], anterior[2], i;
		pid_t pid;
		int cont=0;
		int status;

	        if (2>args){
			printf("\tError: Uso: %s prog1 prog2 ...\n", args[0]);
		}

		for (i=1;i<args;i++)
		cont++;
		{
			if (i<args-1)
			{
				pipe(descf); //se crea un pipe
			}
			if (-1==(pid=fork()))
			{
				printf("Error: invocando fork\n");
			}
			if (0==pid)
			{
				// el proceso hijo cambia su salida estandar
				//al pipe nuevo cerrando la entrada del pipe
				if (i<args-1) // si no es el ultimo
				{
					dup2(descf[1],fileno(stdout));
					close(descf[0]);
				}
				// el proceso hijo cambia su entrada estandar
				//al pipe anterior cerrando la entrada del pipe
				if (i>1) // si no es el primero
				{
					dup2(anterior[0],fileno(stdin));
					close(anterior[1]);
				}
				// ahora ejecuta el programa indicado en argv
				execlp(args[i],args[i],NULL);
				// si llega aqui es que exec fallo
				exit(-1);
			}
			if (i>1)
			{
				close(anterior[0]);
				close(anterior[1]);
			}
			anterior[0]=descf[0];
			anterior[1]=descf[1];
		}
		waitpid(pid,&status,0);
	}
}
*/

void redirecIzq(char * args[]){
			FILE *inputfile;
			static int fd;
			static fpos_t pos;
			int status;
			int fnum1,fnum2;
			int cont = 0;
			while(args[cont]!=0){
				if(strcmp(args[cont],"<")==0){
					args[cont] = strdup(args[cont+1]);
					args[cont+1] = NULL;
				}
					cont++;
			}
			cont --;
		        //Requiere 3 argumentos incluido el nombre del propio ejecutable
			if(cont<1){
				printf("Not enough arguments.\n");
			}
		  else if (args[1]==NULL){
				printf("\tError: bad parameters.\n");
			}
			if (NULL==(inputfile=fopen(args[cont],"r")))
			{
				printf("\tError: opening: %s\n",args[cont]);
			}else{
				//

			fflush(stdin);
			fgetpos(stdin,&pos);
			fd=dup(fileno(stdin));
			freopen(args[cont], "r", stdin);
			//
			args[cont]=NULL;
			block_SIGCHLD();
			int pid_fork = fork();
			if(pid_fork>0){ // es padre
				new_process_group(pid_fork);
				set_terminal(pid_fork);
				waitpid(pid_fork, &status, WUNTRACED);
				set_terminal(getpid());
				}else{// es hijo
					new_process_group(getpid()); //se asigna un gpid diferente al del padre
					restore_terminal_signals();
					fclose(inputfile);
					execvp(args[0], args);
					exit(-1);
				}

			}
			fflush(stdin);
			dup2(fd,fileno(stdin));
			close(fd);
			clearerr(stdin);
			fsetpos(stdin,&pos);
		}

void redirecDer(char * args[]){
		FILE *outfile;
		static int fd;
		static fpos_t pos;
		int status;
		int fnum1,fnum2;
		int cont = 0;
		while(args[cont]!=0){
			if(strcmp(args[cont],">")==0){
				args[cont] = strdup(args[cont+1]);
				args[cont+1] = NULL;
			}
				cont++;
		}
		cont --;
	        //Requiere 3 argumentos incluido el nombre del propio ejecutable
		if(cont<1){
			printf("Not enough arguments.\n");
		}
	  else if (args[1]==NULL){
			printf("\tError: bad parameters.\n");
		}
		if (NULL==(outfile=fopen(args[cont],"w")))
		{
			printf("\tError: opening: %s\n",args[cont]);
		}else{
			//

		fflush(stdout);
		fgetpos(stdout,&pos);
		fd=dup(fileno(stdout));
		freopen(args[cont], "w", stdout);
		//
		args[cont]=NULL;
		block_SIGCHLD();
		int pid_fork = fork();
		if(pid_fork>0){ // es padre
			new_process_group(pid_fork);
			set_terminal(pid_fork);
			waitpid(pid_fork, &status, WUNTRACED);
			set_terminal(getpid());
			}else{// es hijo
				new_process_group(getpid()); //se asigna un gpid diferente al del padre
				restore_terminal_signals();
				fclose(outfile);
				execvp(args[0], args);
				exit(-1);
			}

		}
		fflush(stdout);
		dup2(fd,fileno(stdout));
		close(fd);
		clearerr(stdout);
		fsetpos(stdout,&pos);
	}

void manejadorSIGALRM(int signal){//manejador de SIGALRM
	if(signal<=0){
		printf("Signal error.");
	}
}

void timeOut(char * args[]){
	if (args[1]==NULL){
		printf("Error: bad parameter\n");
		fflush(stdout);
	}
	else{
		if(args[2]!=NULL){
			alarma=atoi(args[1]);
			int pos = 0;
			while (args[pos+2]!=0){
				args[pos]=strdup(args[pos+2]);
				args[pos+2]=NULL;
				pos++;
			}
			args[pos]=NULL;
		}else{
			printf("Error: bad parameter\n");
			fflush(stdout);
		}
	}
}

void mask(char * args[]){

	if (args[1]==NULL){
		printf("Error: bad parameter\n");
		fflush(stdout);
	}else{
		if(args[2]!=NULL){
			int x=1;
			while(strcmp(args[x],"-c")){
				masks[x-1]=atoi(args[x]);
				x++;
			}
			int pos=0;
			while (args[pos+x+1]!=0){
				if(x>2) args[pos]=strdup(args[pos+x+1]);
				else args[pos]=strdup(args[pos+x+1]);
				args[pos+x]=NULL;
				pos++;
			}
			while(args[pos]!=NULL){
				args[pos]=NULL;
			}
		}else{
			printf("Error: bad parameter\n");
			fflush(stdout);
		}
	}
}

void fg(char * args[]) {
	int status, pid_wait, info, status_res;
	int pos;
	if(args[1]==NULL){
		pos=1;
	} else {
		pos=atoi(args[1]);
	}
	if(empty_list(lista)!=1){
		if(list_size(lista)>=pos && pos>0){
			job * aux = get_item_bypos(lista, pos);
			set_terminal(aux->pgid);
			killpg(aux->pgid,SIGCONT);
			aux->state=FOREGROUND;
			pid_wait=waitpid(aux->pgid, &status, WUNTRACED);
			status_res = analyze_status(status, &info);

			if(pid_wait==aux->pgid){
				printf("Process %i status: %s, info: %i\n",aux->pgid,status_strings[status_res], info);
				if(status_strings[status_res]=="Suspended"){
					aux->state=STOPPED;
				}else if(status_strings[status_res]=="Signaled" || status_strings[status_res]=="Exited"){
					delete_job(lista,aux);
			}
			set_terminal(getpid());
		}

		}else printf("Error: bad parameter.\n");
	}else printf("Empty list.\n");
}

void bg(char * args[]) {
		int pos = 1;
		job * aux;
		if(empty_list(lista)!=1){
			if(args[1] != NULL && (list_size(lista)<atoi(args[1]) || 0>atoi(args[1]))){
			printf("Error: bad parameter.\n");
		}else{
			if(args[1]!=NULL){
				pos=atoi(args[1]);
			}
			aux = get_item_bypos(lista,pos);
			aux->state=BACKGROUND;
			killpg(aux->pgid, SIGCONT);
			printf("%i is running in the background.\n", aux->pgid);
		}
	}else {
		printf("Empty list.\n");
	}
}

void manejador(int signal){ //manejador de la señal SIGCHLD

	if(empty_list(lista)!=1){ //si no este vacia
		enum status status_result;
		int stat, infor, pid_aux;
		job * aux, * aux2;
		aux2=lista;
		aux=aux2->next;
		while(aux!=NULL){
			pid_aux=waitpid(aux->pgid, &stat, WNOHANG | WUNTRACED);

			if(pid_aux==aux->pgid){
				status_result=analyze_status(stat, &infor);

				if(status_result==SUSPENDED){
					printf("Proccess %i stopped.\n", aux->pgid);
					aux->state = 2; //stopped
				}
				else{
					printf("\nProccess %i finished.\n", aux->pgid);
					if(aux->state == RESPAWNABLE){
						if(infor == 255){
							delete_job(lista, aux);
							continue;
						}
						job * aux3;
						int pid_fork = fork();


						if(pid_fork>0){ // es padre
							new_process_group(getpid());
							set_terminal(getpid());
							aux3 = new_job(pid_fork,aux->argv[0],RESPAWNABLE,aux->argv);
							add_job(lista, aux3);
							printf("\nProccess %i respawns.\n", aux->pgid);
							fflush(stdout);

							}else{// es hijo
								new_process_group(getpid()); //se asigna un gpid diferente al del padre
								execvp(aux->argv[0], aux->argv);
								printf("Error, command not found: %s\n", aux->argv[0]);
								exit(-1);
							}
						}
						delete_job(lista, aux);
				}
					unblock_SIGCHLD();
			}else aux=aux->next;
		}
	}
	unblock_SIGCHLD();
	fflush(stdout);
	return;
}

void jobs(){
	if(empty_list(lista)!=1){
		print_job_list(lista);
		}else printf("Empty list.\n");
	}

void cd(char * args[]){
		if(args[1]==NULL);
		else{
			ret=chdir(args[1]);
			if(ret!=0){
				printf("No such file or directory: \"%s\"\n", args[1]);
			}else{
				getcwd(path,sizeof(path));
			}
		}
	}

// -----------------------------------------------------------------------
//                            MAIN
// -----------------------------------------------------------------------

int main(void)
{
	signal(SIGCHLD, manejador);
	signal(SIGALRM, manejadorSIGALRM);
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background, respawnable, redirec;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */
	char path[MAX_LINE/2];
	args[0]=0;
	lista = new_list("shell tasks",args);


/*typedef struct job_
{
	pid_t pgid; /* group id = process lider id
	char * command; /* program name
	enum job_state state;
	struct job_ *next; /* next job in the list
} job;
*/


	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{
		alarma=-1;
		int i=0;
		while(masks[i]>0){
			masks[i]=0;
			i++;
		}
		ignore_terminal_signals();
		getcwd(path,sizeof(path));
		printf(ROJO"%s:%s%s# "NEGRO,getenv("USER"), AZUL, path);
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background, &respawnable, &redirec);  /* get next command */
		if(args[0]==NULL) continue;   // if empty command
		if(redirec==1){redirecDer(args);
		//}else if(redirec==3){pipes(args);
		}else if(redirec==2){redirecIzq(args);
		}else if(strcmp(args[0],"jobs")==0){jobs();
		//}else if(strcmp(args[0],"children")==0){children(args);
		}else if(strcmp(args[0],"cd")==0){cd(args);
		}else if(strcmp(args[0],"fg")==0){fg(args);
		}else if(strcmp(args[0],"bg")==0){bg(args);
		}else{
			if(strcmp(args[0],"mask")==0){mask(args);}
			else if(strcmp(args[0],"time-out")==0){timeOut(args);}

			block_SIGCHLD();
			pid_fork = fork();

			if(pid_fork==0){ // es hijo
				new_process_group(getpid()); //se asigna un gpid diferente al del padre
				if(background!=1 && respawnable!=1){
					set_terminal(getpid()); //se le cede el terminal
				}
				restore_terminal_signals();
				if(alarma>0) alarm(alarma);
				if(masks[0]>0){
					int i = 0;
					while(masks[i]>0){
						block_signal(masks[i],1);
						i++;
					}
				}
				unblock_SIGCHLD();
				execvp(args[0], args);
				printf("Error, command not found: %s\n", args[0]);
				exit(-1);
				}else{ //es padre
					new_process_group(pid_fork);

				if(respawnable==1 || background==1){
						add_job(lista, new_job(pid_fork, args[0], respawnable==1 ? 3 : background, args));
						printf("Background job running... pid: %i command: %s\n", pid_fork, args[0]);
						unblock_SIGCHLD();
					}
					else {
						set_terminal(pid_fork);//se le cede al hijo
						unblock_SIGCHLD();
						pid_wait=waitpid(pid_fork, &status, WUNTRACED);
						status_res=analyze_status(status, &info);
						set_terminal(getpid());// lo recupera el padre
						if(info!=255){ //no mostrar si es error
							printf("Foreground pid: %i, command: %s, state: %s, info: %i \n", pid_fork,
				 																		args[0], status_strings[status_res], info);
																				}
						if(status_strings[status_res]=="Suspended"){
							add_job(lista, new_job(pid_fork, args[0], STOPPED, args));
							printf("Process %d suspended.\n",pid_fork);
							unblock_SIGCHLD();
						}
					}
				}

		/* the steps are:
			 (1) fork a child process using fork()
			 (2) the child process will invoke execvp()
			 (3) if background == 0, the parent will wait, otherwise continue
			 (4) Shell shows a status message for processed command
			 (5) loop returns to get_commnad() function
		*/

		} // end while
	}
}
