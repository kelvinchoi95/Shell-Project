#include <cstdio>
#include <signal.h>
#include "shell.hh"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int yyparse(void);

void Shell::prompt() {
	if(isatty(0)) {  
		printf("myshell>");
	}
  fflush(stdout);
}
extern "C" void disp(int sig) {
        printf("\n");
        Shell::prompt();
}

extern "C" void zombie(int sig) {
	//The shell should print the process ID of the child when a process in the background exits in the form "[PID] exited."
	//call waitpid to clean up zombie child
	//if(sig == SIGCHLD) {
		while(waitpid(-1, 0, WNOHANG) > 0);
		//printf("im going to die now\n");
	//}
}
int main() {
 
  //control c
    //printf("in main\n");
    struct sigaction sa;
    sa.sa_handler = disp;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGINT, &sa, NULL)){
        perror("sigaction");
        exit(2);
    }
    //printf("done\n");
    struct sigaction sa1;
    sa1.sa_handler = zombie;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa1, NULL)){
	perror("sigaction");
        //exit(2);
    }
    //printf("done again\n");
  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
