
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <string.h>
#include "command.hh"
#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN TWOGREAT GREAT NEWLINE LESS GREATGREAT GREATAMPERSAND PIPE AMPERSAND GREATGREATAMPERSAND

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"
#include <string.h>
#include "command.hh"
#include <dirent.h>
#include <regex.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
void expandWildcardsIfNecesary(char * arg);
int cmpfunc(const void * file1, const void * file2);
void yyerror(const char * s);
int yylex();

%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       | pipe_list io_modifier_list NEWLINE {
                Shell::_currentCommand.execute();
       }       
       ;

simple_command:	
  command_and_args io_modifier_list background_optional NEWLINE {
    /*printf("   Yacc: Execute command\n");*/
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
    Shell::_currentCommand._outFlag = 0;
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    /*printf("   Yacc: insert argument \"%s\"\n", $1->c_str());*/
    /*Command::_currentSimpleCommand->insertArgument( $1 );*/
    expandWildcardsIfNecesary((char *)$1->c_str());
  }
  ;

command_word:
  WORD {
    /*printf("   Yacc: insert command \"%s\"\n", $1->c_str());*/
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;
pipe_list:
	 pipe_list PIPE command_and_args
	 | command_and_args
	 ;
iomodifier_opt:
  TWOGREAT WORD {
	Shell::_currentCommand._errFile = $2;
  }
  | GREAT WORD {
    /*printf("   Yacc: insert output \"%s\"\n", $2->c_str());*/
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._outFlag++;
  }
  | GREATGREAT WORD {
	Shell::_currentCommand._outFile = $2;
	Shell::_currentCommand._append = 1;
	Shell::_currentCommand._outFlag++;
  }
  | GREATAMPERSAND WORD {
	Shell::_currentCommand._outFile = $2;
	Shell::_currentCommand._errFile = $2;
  }
  | GREATGREATAMPERSAND WORD {
	Shell::_currentCommand._outFile = $2;
        Shell::_currentCommand._errFile = $2;
        Shell::_currentCommand._append = 1;
        Shell::_currentCommand._outFlag++;
  }
  | LESS WORD {
	/*      printf("  Yacc: insert output \"%s\"\n", $2->c_str());*/
        Shell::_currentCommand._inFile= $2;
  }
  ;
io_modifier_list:
  io_modifier_list iomodifier_opt
  | iomodifier_opt
  | /* can be empty */
  ;

background_optional:
  AMPERSAND {
        Shell::_currentCommand._background = 1;
  }
  | /* can be empty */
  ;

%%

int maxEntries = 20;
int nEntries = 0;
char ** entries = (char **) malloc(maxEntries * sizeof(char*));

void expandWildcardsIfNecesary(char * arg) {
	/*return if arg does not contain '*' or '?'*/
	/*printf("in expand wildcards function\n");
	printf("arg is: %s\n", arg);	*/
	if(!strchr(arg, '*') && !strchr(arg, '?')) {
		std::string * str = new std::string(arg);
		/*printf("str is: %s\n", str);*/
		Command::_currentSimpleCommand->insertArgument(str);
		return;
	}
	/* convert wildcard to regular expression*/
	/* Convert "*" -> ".*"*/
	/* "?" -> ".", "." -> "\."*/
	else {
		char * reg = (char*)malloc(2 * strlen(arg) + 10);
		char * a = arg;
		char * r = reg;
		*r = '^';
		r++;
		while(*a) {
			if(*a == '*') {
				*r = '.';
				r++;
				*r='*';
				r++;
			}
			else if(*a == '?') {
				*r = '.';
				r++;
			}
			else if(*a == '.') {
				*r='\\';
				r++;
				*r='.';
				r++;
			}
			else {
				*r=*a;
				r++;
			}
			a++;
		}
		*r='$';
		r++;
		*r=0;

		regex_t re;
		/* 2. compile regular expression*/
		int expbuf = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
		if(expbuf == NULL) {
			perror("compile");
			return;
		}
		
		/* 3 List directory and add as arguments the entries that match the reg exp*/
		DIR * dir = opendir(".");
		if(dir == NULL) {
			perror("opendir");
			return;
		}

		struct dirent * ent;
		regmatch_t match;
		while((ent = readdir(dir)) != NULL) {
			/* Check if name matches */
			if(regexec(&re,ent->d_name, 1, &match, 0)) {
				/* add argument*/
				if(nEntries == maxEntries) {
					maxEntries *=2;
					entries = (char **)realloc(entries, maxEntries * sizeof(char*));
				}
				entries[nEntries]=strdup(ent->d_name);
				nEntries++;
				/*std::string * str = new std::string(strdup(ent->d_name));
				Command::_currentSimpleCommand->insertArgument(str);
				*/
			}
		}
		closedir(dir);
		qsort(entries, nEntries,sizeof(char *), cmpfunc);
		//add arguments
		for(int i = 0; i < nEntries; i++) {
			std::string * str = new std::string(strdup(entries[i]));	
			Command::_currentSimpleCommand->insertArgument(str);
		}
		free(entries);
	}
}

int cmpfunc (const void *file1, const void *file2) {
	const char * _file1 = *(const char **)file1;
	const char * _file2 = *(const char **)file2;
	return strcmp(_file1, _file2);
}
void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
