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
#define fail -1
#define READ_END 0
#define WRITE_END 1

int main(int, char**);

char input_buffer[BUFF_SIZE*2+3];
int print_prompt = false;
int buf_begin, buf_end;
int children_pids[MAX_COMMANDS+10];

void check_input_type() {
    struct stat file_stat;
    fstat(0,&file_stat);
    
    if (S_ISCHR(file_stat.st_mode)) {
        print_prompt = true;
    }
}

int display_prompt() {
    if (print_prompt) {
        return write(1,PROMPT,PROMPT_LENGTH);
    }
    else {
        return true;
    }
}

int read_input() {
    int i;

    if (buf_end > BUFF_SIZE) {
        for (i = buf_begin; i<buf_end; i++) {
            input_buffer[i-buf_begin] = input_buffer[i];
        }
        
        buf_end -= buf_begin;
        buf_begin = 0;
    }
    
    return read(0,input_buffer + buf_end,BUFF_SIZE);
}

int equals(char * a, char * b) {
    int i;
	int size_a,size_b;
	size_a = strlen(a);size_b = strlen(b);
	if (size_a!=size_b) return false;
	
	for (i=0; i<size_a; i++) {
		if (a[i]!=b[i]) return false;
	}
	
	return true;
}

void change_input(char * path) {
    close(STDIN_FILENO);
    
    open(path,O_RDONLY);
}

void change_output(char * path, int append) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    close(STDOUT_FILENO);
    
    if (append) {
        open(path, O_RDWR | O_CREAT | O_APPEND, mode);
    } else {
        open(path, O_RDWR | O_CREAT | O_TRUNC, mode);
    }
}

int shell_command(command_s command) {
	int i=0;
    int stdout_copy;

	while (dispatch_table[i].name) {
		if (equals(dispatch_table[i].name,command.argv[0])) {
            if (equals(command.argv[0],"lenv") && command.out_file_name) {
                stdout_copy = dup(STDOUT_FILENO);
                change_output(command.out_file_name,command.append_mode);
            }

			dispatch_table[i].fun(command.argv);

            if (equals(command.argv[0],"lenv") && command.out_file_name) {
                close(STDOUT_FILENO);
                dup2(stdout_copy,STDOUT_FILENO);
                close(stdout_copy);
            }

			return true;
		}
		i++;
	}
	
	return false;
}

void execute_line() {
    int i=0;
    int j;
    int pipe_fd[2];
    int write_to = -1;
    int read_from = -1;
    int pid;
    command_s* commands = split_commands(input_buffer+buf_begin);

    if (commands[0].argv == NULL) {
        return;
    }
    
    if (shell_command(commands[0])) {
    	return;
    }
    
    while(commands[i].argv) {
    	if (commands[i+1].argv)
    	{
    		pipe(pipe_fd);
            write_to = pipe_fd[WRITE_END];
    	}
        
		if (!(pid = fork())) {
			if (write_to != -1) {
				dup2(write_to,STDOUT_FILENO);
				close(write_to);
                close(pipe_fd[READ_END]);
			}
			if (read_from != -1) {
				dup2(read_from,STDIN_FILENO);
				close(read_from);
                close(pipe_fd[WRITE_END]);
			}
			
			if (commands[i].in_file_name) {
				change_input(commands[i].in_file_name);
			}
			
			if (commands[i].out_file_name) {
				change_output(commands[i].out_file_name, commands[i].append_mode);
			}
			
		    if (execvp(commands[i].argv[0], commands[i].argv) == fail) {
		        exit(EXIT_FAILURE);
		    }
		} else if (pid > 0) {
		    if (write_to != -1) close(write_to);
		    if (read_from != -1) close(read_from);
		    
		    children_pids[i] = pid;
		}
		
        read_from = pipe_fd[READ_END];
        write_to = -1;
		
		i++;
    }

    for (j=0; j<i; j++) {
    	waitpid(children_pids[j],NULL,0);
    }
}

void handle_input(int l) {
	int i;
    for (i=0; i<l; i++) {
        if (input_buffer[buf_end + i] == '\n') {
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
    int input_length;
    
    check_input_type();

    while (true) {
        
        if (display_prompt() == fail) {
            continue;
        }
        if ((input_length = read_input()) == 0) {
            break;
        }
        if (input_length < 0) {
            if (errno == EINTR) continue;
            exit(EXIT_FAILURE);
        }
        
        handle_input(input_length);
    }
    
	return 0;
}

