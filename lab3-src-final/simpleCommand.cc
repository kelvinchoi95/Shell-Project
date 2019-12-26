#include <cstdio>
#include <cstdlib>
#include "simpleCommand.hh"
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
int flag = 0;
std::vector<int> SimpleCommand::bpids;

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector
  	//printf("in insertArgument\n");
  	//printf("arg is: %s\n", (char *)argument->c_str());
	char * answer = tilde((char *) argument->c_str());

	if(answer != NULL) {
		//printf("tilde not null\n");
		std::string * bar = new std::string();
		bar->append(answer);
		_arguments.push_back(bar);
	}
	else {	
	answer = checkEnvExpansion((char *)argument->c_str());
	//printf("answer is: %s\n", answer);
	//printf("\n\n");
	//printf("got out of checkEnvExpansion\n");
	
	//answer = tilde((char *) argument->c_str());
	
	std::string * bar = new std::string();
	bar->append(answer);
	

	//printf("answer is: %s\n", answer);
	//printf("string is %s\n", bar.c_str());
	if(flag == 1) {
		//printf("flag is 1\n");
		std::string * first = new std::string();
		std::string * second = new std::string();
		char * realpath = "realpath";
		char * shell = "../shell";
		first->append(realpath);
		second->append(shell);
		_arguments.pop_back();
		_arguments.push_back(first);
		_arguments.push_back(second);
	}
	else if(flag == 2) {
		int pid = getpid();
		//printf("flag is 2\n");
		//printf("pid is: %d\n", pid);

		//_arguments.pop_back();
		std::string * first = new std::string();
		//std::string * second = new std::string();
		//std::string * third = new std::string();
		std::string pidstring = std::to_string(pid);
		//char * ps = "ps";
		//char * u = "-u";
		//char * v = "../shell";
		first->append(pidstring);
		//second->append(u);
		//third->append(v);
		_arguments.push_back(first);
		//_arguments.push_back(second);
		//_arguments.push_back(third);

	}
	else if(flag == 3) {
		printf("flag is 3\n");

	}
	else if(flag == 4) {
		int pid = bpids.back();
		std::string * first = new std::string();
		std::string pidstring = std::to_string(pid);
		first->append(pidstring);
		_arguments.push_back(first);
		//printf("flag is 4\n");
	}
	else {
	//printf("about to push %s\n", answer);
	//printf("here for some reason\n");
	_arguments.push_back(bar);
	}
	}
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}

char * SimpleCommand::checkEnvExpansion(char * arg) {
	//printf("in check env expansion function\n");
	//printf("arg in checkenvexpansion is: %s\n", arg);
	
	
	char * argument = strdup(arg);
	//printf("argument is: %s\n", argument);
        //char * r = (char *)malloc(sizeof(arg) + 40);	
	char * braceCheck = strchr(argument, '{');
	char * dollarCheck = strchr(argument, '$');
	char * braceCheck2 = strchr(argument, '}');


	int i = 0;
	int j = 0;
	int k = 0;
	int z = 0;
	//printf("asdfsdfsdf\n");	
	if(dollarCheck != NULL && braceCheck != NULL) {
		char * temp = (char *) malloc(sizeof(argument) + 350);
		while(argument[i] != NULL) {
			if(argument[i] == '$') {
				i++;
				if(argument[i] == '{') {
					//printf("getting stuff inside brackets\n");
					i++;
					char * env = (char *) malloc(sizeof(200));
					while(argument[i] != '}') {
						env[k] = argument[i];
						k++;
						i++;
					}
					env[k] = '\0';
					if(strcmp(env, "SHELL") == 0) {
						//printf("FOUND\n");
						flag = 1;
						break;
					}
					if(strcmp(env, "$") == 0) {
						flag = 2;
						
						break;
					}
					if(strcmp(env, "?") == 0) {
						printf("question mark found\n");
						flag = 3;
						break;
					}
					if(strcmp(env, "!") == 0) {
						//printf("exclamation mark found\n");
						flag = 4;
						break;
					}
					//printf("source env var is: %s\n", env);
					k = 0;
					char * x = getenv(env);
					//printf("env var is: %s\n", x);
					while(x[z] != '\0') {
						temp[j] = x[z];
						j++;
						z++;
					}

					z = 0;
				}
				//printf("temp after getting env var is: %s and i is: %d\n", temp, i);
			}
			else if(argument[i] != '$' && argument[i] != '{' && argument[i] != '}') {
				temp[j] = argument[i];
				//printf("*temp is: %c\n", temp[j]);
				i++;
				j++;
			}
			else {
				i++;
			}
		}
		//printf("temp for now is: %s\n", temp);
		
		temp[j] = '\0';

		//printf("arg is: %s\n", temp);
		//char * returnValue = getenv(temp);
		//printf("returnValue is: %s\n", temp);
		return temp;
		
	}
	//printf("about to return: %s\n", arg);	
	return arg;
}

char * SimpleCommand::tilde(char * arg) {
	if(arg[0] == '~') {
		if(strlen(arg) == 1) {
			arg = strdup(getenv("HOME"));
			return arg;
		}
		else {
			if(arg[1] == '/') {
				char * dir = strdup(getenv("HOME"));
				arg++;
				arg = strcat(dir, arg);
				return arg;
			}

			
			char * temp = strdup(arg);
			char * n = (char *)malloc(strlen(arg) + 100);
			char * uName = (char * ) malloc(100);
			char * user = uName;

			temp++;

			while(*temp != '/' && *temp != NULL) {
				*uName = *temp;
				uName++;
				temp++;
			}
			*uName='\0';
			if(*temp) {
				n = strcat(getpwnam(user)->pw_dir, temp);
				arg = strdup(n);
				return arg;
			}
			else {
				arg = strdup(getpwnam(user)->pw_dir);
				return arg;
			}
			
			//printf("tilde arg size is greater than 1\n"); 
			return arg;
		}

	}

	
	return NULL;
	
}
