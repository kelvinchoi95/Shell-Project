
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"
#include "command.hh"
#include <sys/types.h>
#include <unistd.h>
////////////  Start added code ///////////

extern "C" char * read_line();

int mygetc(FILE * f) {
 static char *p;
 char ch;

 if (!isatty(0)) {
 // stdin is not a tty. Call real getc
   return getc(f);
 }

 // stdin is a tty. Call our read_line.
 if (p==NULL || *p == 0) {
   char * s = read_line();
   p = s;
 }

 ch = *p;
 p++;

 return ch;
}

#undef getc
#define getc(f) mygetc(f)

/////////// End added code ///////////
static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

%}

%%
\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  	return GREAT;
}
"<" {
	return LESS;
}
">>" {
	return GREATGREAT;
}
"2>" {
	return TWOGREAT;
}

"|" {
	return PIPE;
}
"&" {
	return AMPERSAND;
}
">&" {
	return GREATAMPERSAND;
}
">>&" {
	return GREATGREATAMPERSAND;
}
"$("[^\)]*")" {
	/* backtick*/
	/*subshell*/
	char * const * args = NULL;
	char * a = strdup(yytext + 1);
	//printf("a is: %s\n", a);
	if(a[strlen(a) - 2] == ')') {
		a[strlen(a) - 2] = '\0';
	}
	//printf("a after check is: %s\n", a);
	//parent creates two pipes
	//int ret;
	int tmpin = dup(0);
	int tmpout = dup(1);

	int fdpipein[2];
	int fdpipeout[2];
	pipe(fdpipein);
	pipe(fdpipeout);

	//write to pipe
	write(fdpipein[1], a, strlen(a));
	write(fdpipein[1], "\n", 1);
	write(fdpipein[1], "exit", 4);
	write(fdpipein[1], "\n", 1);
	close(fdpipein[1]);

	dup2(fdpipein[0], 0);
	close(fdpipein[0]);
	dup2(fdpipeout[1], 1);
	close(fdpipeout[1]);
	//parent creates child process
	//printf("about to fork in subshell\n");
	//fflush(stdout);
	int ret = fork();
	//printf("ret value is: %d\n", p);	
	//in child redirect inout from pin[0] and redirect output to pout[1]. then execute shell from /proc/self/exe
	if(ret == 0) {
		//printf("about to execute in subshell\n");
		execvp("/proc/self/exe", args);
		//printf("ERROR: %s\n", strerror(errno));
		_exit(1); 
		
	}
	else if(ret < 0){
		perror("fork in subshell");
		exit(1);
	}
	/*else {
		perror("NOT GOING IN CHILD\n");
		exit(1);
	}*/

	dup2(tmpin, 0);
	dup2(tmpout, 1);
	close(tmpin);
	close(tmpout);
	
	char x;
	char * buffer = (char *) malloc(4000);
	int i = 0;

	while(read(fdpipeout[0], &x, 1)) {
		if(x == '\n') {
			buffer[i] = ' ';

		}
		else {
			buffer[i] = x;
		}
		i++;
		
	}
	buffer[i] = '\0';
	//put back into lex in reverse
	int j;
	for(j = strlen(buffer); j >= 0; j--) {
		if(buffer[j] != '\0') {
			//printf("buffer[j] is: %c\n", buffer[j]);
			unput(buffer[j]);
		}
	}
	//parent writes to pin[1]
	//the child process runs the subshell command and writes output to pout[1]
	//the parent reads the output from pout[0] and writes it into a buffer. The parent puts the characters in the buffer back into lex by calling myunoutc(in c) in reverse order	
}
["][^\n\"]*["] {
	/*printf("in quotes part\n");*/
	char * a = strdup(yytext);
	int length = 0;
	while(*a != '\0') {
		a++;
		length++;
	}
	yylval.cpp_string = new std::string(yytext, 1, length - 2);
	return WORD;	
}
[^ \t\n<>&'\|]*\\[^ \t\n]* {
	/* escape */
	//char * a = strdup(yytext);
	int count = 0;
	int i = 0;
	char * a = strdup(yytext);
	std::string temp("");
	
	for(i = 0; i < strlen(a); i++) {
		if(yytext[i] == '\\') {
			if(yytext[i+1] == '\\') {
				temp += yytext[i+1];
				i++;
				//count++;				
			}
		}
		else {
			temp += yytext[i];
			//count++;
		}
		
	}
	yylval.cpp_string = new std::string(temp.c_str());
	return WORD;
	
}
[^ \t\n|<>&][^ \t\n|<>&]*  {
  /* Assume that file names have only alpha chars */
  	yylval.cpp_string = new std::string(yytext);
  	return WORD;
}
.	{
	return NOTOKEN;
}
%%
