#include <ansi.h>
#include <minix/ipc.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>

#define BSIZE 1024

int mopen(char * fname, int flags){
	va_list argp;
	message m;

	va_start(argp, flags);
	if (flags & O_CREAT) {
		{
			register const char *s = fname;

			while (*s++);
	
			m.m1_i1 = --s - fname + 1;
		}
		m.m1_i2 = flags;
		m.m1_i3 = va_arg(argp, _mnx_Mode_t);
		m.m1_p1 = (char *) fname;
	} else {
		register size_t k;
		{
			register const char *s = fname;
			
			while (*s++);	
			
			k = --s - fname + 1;
		}
		m.m3_i1 = k;
		m.m3_p1 = (char *) fname;
		if (k <= sizeof m.m3_ca1) 
		{
			register char *s1 = m.m3_ca1;

			while (*s1++ = *fname++);
		}
		m.m3_i2 = flags;
	}
	va_end(argp);
	
	int status;
	register message * ptr = &m;
	
	ptr->m_type = 5;
	status = _sendrec(1, ptr);
	if (status != 0) {
		/* 'sendrec' itself failed. */
		/* XXX - strerror doesn't know all the codes */
		ptr->m_type = status;
	}
	if (ptr->m_type < 0) {
		errno = -ptr->m_type;
		return(-1);
	}
	return(ptr->m_type);
}

int main(int argc, char * argv[]){
	char buf[BSIZE];
	int n;

	int fd;

	if (argc!=2){
		fprintf(stderr, "Wrong number of arguments.");
		exit(1);
	}

	fd = mopen(argv[1], 0);

	if (fd<0){
		fprintf(stderr, "Cannot open:%s\n",argv[1]);
		exit(1);
	}

	while (n=read(fd, buf,BSIZE)){	
		if ((n==-1) && (errno==EINTR || errno==EAGAIN)) continue;
		if (n==-1) exit(1);
		write(1, buf, n);
	}
	return 0;
}
