#include <ansi.h>
#include <minix/ipc.h>
#include <stdio.h>

extern int errno;

int mgetppid(void){
	message msg;
	register message * ptr = &msg;
	int status;

	ptr->m_type = 20;
	status = _sendrec(0, &msg);
	if (status != 0) {
		/* 'sendrec' itself failed. */
		/* XXX - strerror doesn't know all the codes */
		ptr->m_type = status;
	}
	if (ptr->m_type < 0) {
		errno = -ptr->m_type;
		return(-1);
	}
	
	if (ptr->m_type < 0) return ( (pid_t) -1);
	return( (pid.t) ptr->m2_i1);
}

int main(int argc, char * argv[]){
	int ppid= mgetpid();

	printf("parent pid:%d\n",pid);
	return 0;
}
