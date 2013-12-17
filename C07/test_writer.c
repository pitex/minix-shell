#include <unistd.h>
#include <stdio.h>

int main(int argc, char * argv[]){
	int val;
	char buf[20];

	while (read(3,&val,4)==4){
		int n = sprintf(buf,"%d\n",val);
		if (n>0) write(1,buf,n);
	}
	while (read(4,&val,4)==4){
		int n = sprintf(buf,"%d\n",val);
		if (n>0) write(1,buf,n);
	}

}
