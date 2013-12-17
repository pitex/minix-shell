#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "config.h"
#include "commands.h"
#include "cparse.h"

#define true 1
#define false 0

int main(int, char**);

char input_buffer[BUFF_SIZE*2+3];
int print_prompt = false;
int buf_begin, buf_end;
int children_pids[MAX_COMMANDS+10];

void check_input_type()
{
    struct stat file_stat;
    fstat(0,&file_stat);
    
    if (S_ISCHR(file_stat.st_mode))
    {
        print_prompt = true;
    }
}

int write_prompt()
{
    if (print_prompt)
    {
        return write(1,PROMPT,PROMPT_LENGTH);
    }
    else
    {
        return 1;
    }
}

int read_input()
{
    if (buf_end > BUFF_SIZE)
    {
    	int i;
        for (i = buf_begin; i<buf_end; i++)
        {
            input_buffer[i-buf_begin] = input_buffer[i];
        }
        
        buf_end -= buf_begin;
        buf_begin = 0;
    }
    
    return read(0,input_buffer + buf_end,BUFF_SIZE);
}

int equals(char * a, char * b)
{
	int size_a,size_b;
	size_a = strlen(a);size_b = strlen(b);
	if (size_a!=size_b) return false;
	
	int i;
	for (i=0; i<size_a; i++)
	{
		if (a[i]!=b[i]) return false;
	}
	
	return true;
}

int shell_command(command_s command)
{
	int i=0;
	while (dispatch_table[i].name)
	{
		if (equals(dispatch_table[i].name,command.argv[0]))
		{
			dispatch_table[i].fun(command.argv);
			return 1;
		}
		i++;
	}
	
	return 0;
}

void change_input(char * path)
{
    close(0);
    
    open(path,O_RDONLY);
}

void change_output(char * path, int append)
{
    close(1);
    
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    
    if (append)
    {
    	open(path, O_RDWR | O_CREAT | O_APPEND, mode);
    }
    else
    {
    	open(path, O_RDWR | O_CREAT | O_TRUNC, mode);
    }
}

void execute_line()
{
    command_s* commands = split_commands(input_buffer+buf_begin);
    if (commands[0].argv == NULL)
    {
        return;
    }
    
    if (shell_command(commands[0]))
    {
    	return;
    }
    
    int i=0;
    int got_pipe = false;
    int pipe_des[2];
    while(commands[i].argv)
    {
    	int got_new_pipe = false;
    	int new_pipe[2];
    	if (commands[i+1].argv)
    	{
    		pipe(new_pipe);
    		got_new_pipe = true;
    	}
        
		int pid;
		if (!(pid = fork()))
		{
			if (got_pipe)
			{
				close(0);
				dup2(pipe_des[0],0);
				close(pipe_des[0]);
			}
			if (got_new_pipe)
			{
				close(1);
				dup2(new_pipe[1],1);
				close(new_pipe[1]);
			}
			
			if (commands[0].in_file_name)
			{
				change_input(commands[0].in_file_name);
			}
			
			if (commands[0].out_file_name)
			{
				change_output(commands[0].out_file_name, commands[0].append_mode);
			}
			
		    if (execvp(commands[0].argv[0], commands[0].argv)==-1)
		    {
		        exit(EXIT_FAILURE);
		    }
		}
		else if (pid > 0)
		{
		    if (got_pipe) close(pipe_des[0]);
		    if (got_new_pipe) close(new_pipe[1]);
		    
		    children_pids[i] = pid;
		}
		
		got_pipe = got_new_pipe;
		pipe_des[0] = new_pipe[0];
		pipe_des[1] = new_pipe[1];
		
		i++;
    }
    
    int j;
    for (j=0; j<i; j++)
    {
    	waitpid(children_pids[j],NULL,0);
    }
}

void handle_input(int l)
{
	int i;
    for (i=0; i<l; i++)
    {
        if (input_buffer[buf_end + i] == '\n')
        {
            input_buffer[buf_end + i] = '\0';
            
            execute_line();
            
            buf_begin = buf_end+i+1;
        }
    }
    
    buf_end += l;
}

int main(argc, argv)
int argc;
char* argv[];
{
    
    check_input_type();

    while (1)
    {
        int input_length;
        
        if (write_prompt() == -1)
        {
            continue;
        }
        if ((input_length = read_input()) == 0)
        {
            break;
        }
        if (input_length < 0)
        {
            if (errno == EINTR) continue;
            exit(EXIT_FAILURE);
        }
        
        handle_input(input_length);
    }
    
	return 0;
}

