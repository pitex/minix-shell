#include <stdio.h>
#include <ansi.h>
#include <minix/ipc.h>
#include <stdio.h>

extern int errno;

int store(int n) {
	message msg;
	register message * ptr = &msg;
	int status;

	ptr->m_type = 49;
	ptr->m1_i1 = n;
	status = _sendrec(0, ptr);
	if (status != 0) {
		ptr->m_type = status;
	}
	if (ptr->m_type < 0) {
		errno = -ptr->m_type;
		return(-1);
	}
	return(ptr->m_type);
}

int retrieve(int *pn) {
	message msg;
	register message * ptr = &msg;
	int status;

	ptr->m_type = 50;
	status = _sendrec(0, ptr);
	if (status != 0) {
		ptr->m_type = status;
	}
	if (ptr->m_type < 0) {
		errno = -ptr->m_type;
		return(-1);
	}
	(*pn) = ptr->m1_i1;
	return(ptr->m_type);
}

int main() {
	int x = store(42);
	printf("%d\n",x);
	int a;
	x = retrieve(&a);
	printf("%d %d\n",x,a);
}

