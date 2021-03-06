#include <ansi.h>
#include <minix/ipc.h>
#include <stdio.h>

extern int errno;

int mgetpid(void){
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
	return(ptr->m_type);
}

int main(int argc, char * argv[]){
	int pid= mgetpid();

	printf("my pid:%d\n",pid);
	return 0;
}
