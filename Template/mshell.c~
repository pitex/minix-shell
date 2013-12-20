#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

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
int status[MAX_COMMANDS+1];
int bckg_pid[MAX_COMMANDS+1];
int background_saved;
int in_foreground;
int left_in_foreground;
struct sigaction def_sigint_act;
struct sigaction def_sigchld_act;
sigset_t wait_mask;
sigset_t block_mask;

/*  Checks if input is special character input	*/
void check_input_type() {
	struct stat file_stat;

	fstat(0,&file_stat);
	
	if (S_ISCHR(file_stat.st_mode)) {
		print_prompt = true;
	}
}

/*  Displays prompt	*/
int display_prompt() {
	int i;

	if (print_prompt) {
		for (i=0; i<background_saved; i++) {
			write_string("Process ");
			write_int(bckg_pid[i]);
			if (WIFSIGNALED(status[i])) {
				write_string(" killed by signal ");
				write_int(WTERMSIG(status[i]));
				write_string("\n");
			} else {
				write_string(" ended with status ");
				write_int(status[i]);
				write_string("\n");
			}
		}
		background_saved = 0;
		return write(1,PROMPT,PROMPT_LENGTH);
	}
	else {
		return true;
	}
}

/*  Reads input	*/
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

/*  Compares two strings	*/
int equals(char * a, char * b) {
	int i;
	int size_a,size_b;

	size_a = strlen(a);
	size_b = strlen(b);

	if (size_a!=size_b) return false;
	
	for (i=0; i<size_a; i++) {
		if (a[i]!=b[i]) return false;
	}
	
	return true;
}

/*  Replace current file descriptor on STDIN_FILENO with path file	*/
void change_input(char * path) {
	close(STDIN_FILENO);
	
	open(path,O_RDONLY);
}

/*  Replace current file descriptor on STDOUT_FILENO with path file	*/
void change_output(char * path, int append) {
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	close(STDOUT_FILENO);
	
	if (append) {
		open(path, O_RDWR | O_CREAT | O_APPEND, mode);
	} else {
		open(path, O_RDWR | O_CREAT | O_TRUNC, mode);
	}
}

/*  Check if command is shell command	*/
int shell_command(command_s command) {
	int i;
	int stdout_copy;

	/*  Go through dispatch table	*/
	for (i=0; dispatch_table[i].name != NULL; i++) {
		if (equals(dispatch_table[i].name,command.argv[0])) {
			/*  If lenv has redirected output, do it	*/
			if (equals(command.argv[0],"lenv") && command.out_file_name) {
				stdout_copy = dup(STDOUT_FILENO);
				change_output(command.out_file_name,command.append_mode);
			}

			dispatch_table[i].fun(command.argv);

			/*  Clean up after redirecting output	*/
			if (equals(command.argv[0],"lenv") && command.out_file_name) {
				close(STDOUT_FILENO);
				dup2(stdout_copy,STDOUT_FILENO);
				close(stdout_copy);
			}

			return true;
		}
	}
	
	return false;
}

void execute_line() {
	int i;
	int j;
	int pipe_fd[2];
	int write_to;
	int read_from;
	int pid;
	int do_in_background;
	int finished_foreground;
	command_s* commands;

	write_to = -1;
	read_from = -1;
	do_in_background  = in_background(input_buffer+buf_begin);
	commands = split_commands(input_buffer+buf_begin);

	/*  If there are no commands	*/
	if (commands[0].argv == NULL) {
		return;
	}
	
	/*  If it is shell command	*/
	if (!do_in_background && shell_command(commands[0])) {
		return;
	}
	
	/*	Block incoming SIGCHLD for now	*/
	sigprocmask(SIG_BLOCK, &block_mask, NULL);

	/*  Go through commands	*/
	for(i=0; commands[i].argv != NULL; i++) {
		/*  If there is a pipe after command, get ready to redirect output	*/
		if (commands[i+1].argv)
		{
			pipe(pipe_fd);
			write_to = pipe_fd[WRITE_END];
		}
		
		if (!(pid = fork())) {
			/*	Restore default signal handling	*/
			sigaction(SIGINT, &def_sigint_act, NULL);
			sigaction(SIGCHLD, &def_sigchld_act, NULL);

			/*	Restore default SIGCHLD handling and disable signal forwarding from parent	*/
			if (do_in_background) {
				setsid();
			} else {
				/*	We don't really need it now, do we?	*/
				sigprocmask(SIG_UNBLOCK, &block_mask, NULL);
			}

			/*  Redirect output if needed	*/
			if (write_to != -1) {
				dup2(write_to,STDOUT_FILENO);
				close(write_to);
				close(pipe_fd[READ_END]);
			}
			/*  Redirect input if needed	*/
			if (read_from != -1) {
				dup2(read_from,STDIN_FILENO);
				close(read_from);
			}
			
			/*  Redirect input if "<"	*/
			if (commands[i].in_file_name) {
				change_input(commands[i].in_file_name);
			}
			
			/*  Redirect output if ">" or ">>"	*/
			if (commands[i].out_file_name) {
				change_output(commands[i].out_file_name, commands[i].append_mode);
			}
			
			/*  Execute command	*/
			if (execvp(commands[i].argv[0], commands[i].argv) == fail) {
				exit(EXIT_FAILURE);
			}
		} else if (pid > 0) {
			/*  Close pipe	*/
			if (write_to != -1) close(write_to);
			if (read_from != -1) close(read_from);
			
			/*  Remember child pid	*/
			children_pids[i] = pid;
		}
		
		/*  Get ready to redirect input of the next command	*/
		read_from = pipe_fd[READ_END];
		write_to = -1;
	}

	/*  Wait for children to finish	*/
	if (!do_in_background) {
		in_foreground = left_in_foreground = i;

		/*	Gonna catch'em all	*/
		while (left_in_foreground) {
			sigsuspend(&wait_mask);
		}
	}
	in_foreground = left_in_foreground = 0;

	/*	BRING THEM ON!	*/
	sigprocmask(SIG_UNBLOCK, &block_mask, NULL);
}

/*  Decide what to do with read input	*/
void handle_input(int l) {
	int i;

	for (i=0; i<l; i++) {
		/*  If there is a whole line in buffer, execute it	*/
		if (input_buffer[buf_end + i] == '\n') {
			input_buffer[buf_end + i] = '\0';
			
			execute_line();
			
			buf_begin = buf_end+i+1;
		}
	}
	
	buf_end += l;
}

/*	handle SIGINT	*/
void my_sigint_handler(int a) {
	write_string("\n");
}

/*	handle SICHLD	*/
void my_sigchld_handler(int a) {
	int pid;
	int i;
	int sts;
	int is_bck;

	/*	get pid number of finished child	*/
	while ((pid = waitpid(-1, &sts, WNOHANG))>0) {
		is_bck = true;
	
		/*	chech if foreground child was finished	*/
		for (i=0; i<in_foreground; i++) {
			if (children_pids[i] == pid) {
				is_bck = false;
				left_in_foreground--;
				break;
			}
		}

		/*	save iformation about background child finished	*/
		if (is_bck && background_saved < MAX_COMMANDS) {
			status[background_saved] = sts;
			bckg_pid[background_saved++] = pid;
		}
	}
}

/*	for handling ctrl-c and SIGCHLD	*/
void set_hadlers() {
	struct sigaction new_sigint_act;
	struct sigaction new_sigchld_act;

	new_sigint_act.sa_handler = my_sigint_handler;
	new_sigchld_act.sa_handler = my_sigchld_handler;
	
	sigfillset(&new_sigint_act.sa_mask);
	sigfillset(&new_sigchld_act.sa_mask);

	sigaction(SIGINT, &new_sigint_act, &def_sigint_act);
	sigaction(SIGCHLD, &new_sigchld_act, &def_sigchld_act);
}

/*	initialize variables	*/
void init() {
	/*	prepare mask for waiting	*/
	sigprocmask(SIG_BLOCK, NULL, &wait_mask);

	/*	prepare mask for blocking sigchld	*/
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGCHLD);
}

int main(argc, argv)
int argc;
char* argv[];
{
	int input_length;
	int you_no_print_prompt;

	you_no_print_prompt = false;
	init();
	
	check_input_type();

	set_hadlers();

	while (true) {
		
		if (!you_no_print_prompt) {
			if (display_prompt() == fail) {
				continue;
			}
		}
		
		you_no_print_prompt = false;

		/*  If there is nothing more to read, finish	*/
		if ((input_length = read_input()) == 0) {
			break;
		}

		/*  If you can't read due to interruption, try again! elsewise just give up	*/
		if (input_length < 0) {
			if (errno == EINTR) {
				you_no_print_prompt = true;
				continue;
			}
			exit(EXIT_FAILURE);
		}
		/*  What to do now?	*/
		handle_input(input_length);
	}
	
	return 0;
}

