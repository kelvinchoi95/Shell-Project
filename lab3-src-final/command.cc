/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "command.hh"
#include "shell.hh"
#include <stdio.h>
#include <string.h>
//extern int pid = 0;

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();
    numsimplecommands = 0;
    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _outFlag = 0;
    _inFlag = 0;
    _append = 0;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
    numsimplecommands++;
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    int outflag = 0;
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
	outflag = 1;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
    numsimplecommands = 0;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}
int Command::builtInFunction(int i) {
	
	//setenv A B
	if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "setenv")) {
		int error = setenv(_simpleCommands[i]->_arguments[1]->c_str(), _simpleCommands[i]->_arguments[2]->c_str(), 1);
		if(error) {
			perror("setenv");
		}
		clear();
		Shell::prompt();
		return 1;
	}
	
	//unsetenv A B
	if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "unsetenv")) {
		int error = unsetenv(_simpleCommands[i]->_arguments[1]->c_str());
		if(error) {
			perror("unsetenv");
		}
		clear();
		Shell::prompt();
		return 1;
	}
	//cd A
	if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "cd")) {
		int error;
		if(_simpleCommands[i]->_arguments.size() == 1) {
			//printf("HOME part\n");
			error = chdir(getenv("HOME"));
		}
		else {
			error = chdir(_simpleCommands[i]->_arguments[1]->c_str());
		}
		if(error) {
			perror("cd");
		}
		clear();
		Shell::prompt();
		return 1;
	}
	
	return 0;
}
void Command::execute() {
	//printf("in execute\n");
	//printf("numsimplecommands is: %d\n", numsimplecommands);
	char ** sc;
       	// Don't do anything if there are no simple commands
	//save in/out
//	print();
	int tmpin=dup(0);
	int tmpout=dup(1);
        int tmperr=dup(2);	
	//set initial input
	int fdin;
	int fderr;
	if(_outFlag > 1) {
		printf("Ambiguous output redirect.\n");
		clear();
		Shell::prompt();
		return;
	}
	if (_inFile) {
		fdin = open(_inFile->c_str(), O_RDONLY);
		if(fdin < 0) {
			perror("inputfile");
			exit(1);
		}
	}
	else {
		fdin=dup(tmpin);
	}
	
	if(_errFile) {
		if(_append == 1) {
			fderr = open(_errFile->c_str(), O_RDWR | O_CREAT | O_APPEND, 0600);
		}
		else {
			fderr = open(_errFile->c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
		}
		if(fderr < 0) {
			perror("errorfile");
			exit(1);
		}
	}
	else {
		fderr = dup(tmperr);
	}
	dup2(fderr,2);
	close(fderr);


	int ret;
	int fdout;
	//printf("initial numsimplecommands is: %d\n", numsimplecommands);
	for(int i = 0; i < numsimplecommands; i++) {
	//printf("got inside for loop\n");
		//printf("beginning i is: %d\n", i);
	//	int count = i;
		if(isatty(0)) {
			if(strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "exit")==0) {
				printf("Bye!\n");
				exit(1);
			}
		}
		if(builtInFunction(i)) {
			return;
		}
		dup2(fdin, 0);
		close(fdin);
		//setup output
		//printf("i after builtin check is: %d\n", i);
		if (i == numsimplecommands - 1) {
			if(_outFile) {
				if(_append == 1) {
					fdout=open(_outFile->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
				}
				else {
					 fdout=open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
				}
			}
			else {
				fdout=dup(tmpout);
			}
		
		//printf("i after last command check is: %d\n", i);
		}
		else {
			//not last simple command
			//create pipe
			int fdpipe[2];
			pipe(fdpipe);
			fdout=fdpipe[1];
			fdin=fdpipe[0];
			//printf("i in else is : %d\n", i);
		}
		//printf("i check: %d\n", i);
		//redirect output
		//dup2(fdout, 1);
		//printf("i in between: %d\n", i);
		//close(fdout);
		//printf("i before storing args is: %d\n", i);
		//printf("about to handle storing simple command arguments\n");
		//make string vector to store current simple command
		//printf("i is: %d\n", i);
		sc = (char **) malloc(sizeof(char *) * (_simpleCommands[i]->_arguments.size() + 1));
		//printf("created double pointer\n");
		for(int j = 0; j < (int)_simpleCommands[i]->_arguments.size(); j++) {
			//printf("inside sc storage for loop\n");
			sc[j] =const_cast<char*>(_simpleCommands[i]->_arguments[j]->c_str());
			//printf("sdfdfsc[0] is: %s\n", sc[j]);
		//printf("string is: %s\n", sc[j]);
		}
		sc[_simpleCommands[i]->_arguments.size()] = NULL;
		//printf("set null\n");
		//create child process
		dup2(fdout, 1);
		close(fdout);
		ret = fork();
		if(ret == 0) {
			//printf("in child process\n");
			//printenv
			//for(int k = 0; k < _simpleCommands[i]->_arguments.size(); k++) {
			//	printf("sd[k] inside child is: %s\n", sc[k]);
			//}
			//printf("i is: %d\n", i);
			//printf("sc[0] is: %s\n", sc[0]);
			if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv")) {
				char **p = environ;
			//	printf("in printenv\n");
				while (*p != NULL) {
					printf("%s\n", *p);
					p++;
				}
				//Shell::prompt();
				exit(0);
			}
			if(!strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "source")) {
				//if source then open file 
				//run file line by line
				//FILE * fp = fopen(_simpleCommands[i]->_arguments[1]->c_str(), "r");
				//char line [1024];
				//get each line

			}
			//printf("before execvp\n");
			//dup2(fdout, 1);
			//close(fdout);
			execvp(sc[0], sc);
			//fprintf(stderr, "sc[0]: %s\t, sc: %s\n", sc[0], sc);
			perror("execvp");
			exit(1);
		}

	}
	
	//out of for
	//restore in/out defaults
	dup2(tmpin, 0);
	dup2(tmpout, 1);
	dup2(tmperr, 2);
	close(tmpin);
	close(tmpout);
	close(tmperr);
	//printf("before background\n");	
	if(!_background) {
		//wait for last command
		waitpid(ret, NULL, 0);
	}
	if(_background == true){
	//	printf("in background\n");
		SimpleCommand::bpids.push_back(getpid());
	}



    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
	
    // Clear to prepare for next command
    clear();
    //printf("numsimplecommands is: %d\n", numsimplecommands);
    // Print new prompt
    Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
