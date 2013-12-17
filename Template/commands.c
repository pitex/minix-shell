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

command_pair dispatch_table[]={
    {"echo", &echo},
    {"exit", &m_exit},
    {"cd", &m_cd},
    {"kill", &m_kill},
    {"lenv", &m_lenv},
    {"lls", &m_ls},
    {NULL,NULL}
};

int echo(argv)
char * argv[];
{
    int i =1;
    if (argv[i]) printf("%s", argv[i++]);
    while  (argv[i])
        printf(" %s", argv[i++]);
    printf("\n");
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
            printf("mshell: exit: %s: numeric argument required\n",argv[0]);
            exit(255);
        }
        
        id = to_int(argv[1]);
        if (id<0)
        {
            printf("mshell: exit: Illegal number: %d\n",id);
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
    if (argv[1])
    {
        chdir(argv[1]);
    }
    else
    {
        chdir(getenv("HOME"));
    }
    return 0;
}

int m_kill(argv)
char * argv[];
{
    if (!argv[1])
    {
        printf("kill: usage: kill [-signal] pid");
        return -1;
    }
    if (!argv[1])
    {
        kill(to_int(argv[1]),0);
    }
    else
    {
        kill(to_int(argv[2]),to_int(argv[1]));
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
        printf("%s\n",*current);
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
                printf("%s\n",file->d_name);
            }
        }
        closedir(cur_dir);
    }
    fflush(stdout);
}
