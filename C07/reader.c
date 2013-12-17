#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#define PATH "mojpathdofifo"
#define PATH2 "mojpathdofilea"

int length(int a)
{
	int l = 0;
	while (a)
	{
		l++;
		a/=10;
	}
	
	return l;
}

void create_fifo()
{
	int ppid = getppid();
	int x = length(ppid);
	
	char fifo_path[13+x+1];
	
	{
		int i;
		for (i = 0; i<13; i++)
		{
			fifo_path[i] = PATH[i];
		}
		for (i=13; i<13+x; i++)
		{
			fifo_path[i] = ppid%10;
			ppid/=10;
		}
		fifo_path[13+x] = '\0';
	}
	

	mkfifo(fifo_path,S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);

	close(1);
	open(fifo_path, O_WRONLY);
}

void say_hello()
{
	int ppid = getppid();
	int x = length(ppid);
	
	char file_path[14+x+1];
	
	{
		int i;
		for (i = 0; i<14; i++)
		{
			file_path[i] = PATH2[i];
		}
		for (i=14; i<14+x; i++)
		{
			file_path[i] = ppid%10;
			ppid/=10;
		}
		file_path[14+x] = '\0';
	}
	
	int fd = open(file_path,O_WRONLY | O_CREAT | O_APPEND,S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
	write(fd, "w", 1);
}

int main(int argc, char * argv[])
{
	
}
