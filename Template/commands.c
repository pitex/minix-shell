#define _POSIX_SOURCE 1

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>

#include "commands.h"

int echo(char*[]);
int m_exit(char*[]);
int m_cd(char*[]);
int m_kill(char*[]);
int m_lenv(char*[]);
int m_ls(char*[]);

command_pair dispatch_table[]={
	{"echo", &echo},
	{"exit", &m_exit},
	{"cd", &m_cd},
	{"kill", &m_kill},
	{"lenv", &m_lenv},
	{"lls", &m_ls},
	{NULL,NULL}
};

//	Checks if string can be converted to int
int is_int(char *a)
{
	int i;
	int length = strlen(a);
	int beg = 0;
	int result = 0;
	beg+=(a[0]=='-');
	
	for (i=beg; i<length; i++)
	{
		if (a[i]>'9' || a[i]<'0') return 0;
	}
	
	return 1;
}

//	Converts string to int
int to_int(char *a)
{
	int i;
	int length = strlen(a);
	int beg = 0;
	int result = 0;
	beg+=(a[0]=='-');
	
	for (i=beg; i<length; i++)
	{
		result*=10;
		result+=a[i]-'0';
	}
	return beg?-result:result;
}

void write_string(char * string) {
	int temp;
	int printed = 0;
	int size = strlen(string);
	char * to_print = string;

	while (printed<size) {
		temp = write(STDOUT_FILENO,to_print,size-printed);
		if (temp == -1) {
			exit(EXIT_FAILURE);
		}
		printed+=temp;
	}
}

void write_int(int a) {
	int p;
	char c[2];

	p = 1;
	c[1] = '\0';

	if (a < 0) {
		c[0] = '-';
		write_string(c);
		a*=-1;
	}

	while (p * 10 <= a) {
		p *= 10;
	}

	while (p) {
		c[0] = '0' + (a/p)%10;
		write_string(c);
		p/=10;
	}
}

int echo(argv)
char * argv[];
{
	int i =1;
	if (argv[i]) write_string(argv[i++]);//printf("%s", argv[i++]);
	while  (argv[i]){
		write_string(" ");
		write_string(argv[i++]);
		// printf(" %s", argv[i++]);
	}
	write_string("\n");
	// printf("\n");
	fflush(stdout);
	return 0;
}

int m_exit(argv)
char * argv[];
{
	int i;
	int id = 0;

	if (argv[1])
	{
		if (!is_int(argv[1]))
		{
			write_string("mshell: exit: ");
			write_string(argv[0]);
			write_string(": numeric argument required\n");
			// printf("mshell: exit: %s: numeric argument required\n",argv[0]);
			exit(255);
		}
		
		id = to_int(argv[1]);
		if (id<0)
		{
			write_string("mshell: exit: Illegal number: ");
			write_string(argv[1]);
			write_string("\n");
			// printf("mshell: exit: Illegal number: %d\n",id);
			return -1;
		}
		exit(id);
	}
	else
	{
		exit(EXIT_SUCCESS);
	}
	
	return -1;
}

int m_cd(argv)
char * argv[];
{
	char path[1024];

	getcwd(path,1024);

	if (argv[1]) {
		//	~ case:
		if (argv[1][0] == '~') {
			chdir(getenv("HOME"));
			if (strlen(argv[1])>1) {
				//	if there is someting more than ~
				if (argv[1][1] != '/' || (strlen(argv[1])>2 && chdir(argv[1]+2) != 0)) {
					//	if next char after ~ is not / or there is no such path
					write_string("mshell: cd: ");
					write_string(argv[1]);
					write_string(": No such file or directory\n");
					chdir(path);
				}
			}
		} else if (chdir(argv[1])!=0) {
			write_string("mshell: cd: ");
			write_string(argv[1]);
			write_string(": No such file or directory\n");
		}
	} else if (chdir(getenv("HOME")) != 0) {
		write_string("mshell: cd: ");
		write_string(argv[1]);
		write_string(": No such file or directory\n");
	}
	return 0;
}

int m_kill(argv)
char * argv[];
{
	if (!argv[1])
	{
		write_string("kill: usage: kill [-signal] pid\n");
		// printf("kill: usage: kill [-signal] pid");
		return -1;
	}
	if (!argv[2])
	{
		kill(to_int(argv[1]),15);
	}
	else
	{
		kill(to_int(argv[2]),-1*to_int(argv[1]));
	}
	fflush(stdout);
	
	return 0;
}

extern char **environ;

int m_lenv(argv)
char * argv[];
{
	char **current;
	for(current = environ; *current; current++) {
		write_string(*current);
		write_string("\n");
		// printf("%s\n",*current);
	}
	fflush(stdout);
}

int m_ls(argv)
char * argv[];
{
	char dir_name[1024];
	struct dirent *file;

	getcwd(dir_name,1024);

	DIR* cur_dir = opendir(dir_name);
	if (cur_dir=opendir(dir_name))
	{
		while (file = readdir(cur_dir))
		{
			if (file->d_name[0]!='.')
			{
				write_string(file->d_name);
				write_string("\n");
				// printf("%s\n",file->d_name);
			}
		}
		closedir(cur_dir);
	}
	fflush(stdout);
}
