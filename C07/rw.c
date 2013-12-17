#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define READER "reader"
#define WRITER "writer"
#define TEST_WRITER "test_writer"

void produce(int n);

void producer(int fd[], int n);
void wait_for(int n);

int main(int argc, char * argv[]){
	int i;	

	if ((argc!=3)&&(argc!=4)) {
		printf("Usage:%s numer_of_readers number_of_writers [--test]\n", argv[0]);
		exit(1);
	}

	int readers= atoi(argv[1]);
	int writers= atoi(argv[2]);

	if ((readers<=0) || (writers<=0)){
		printf("Invalid numbers.\n", argv[0]);
		exit(1);
	}
	
	int test=0;
	if ((argc==4)&&(strncmp("--test", argv[3],7)==0)){
		test=1;
	}

	// generate producers and writers

	close(3); close(4);
	fcntl(0,F_DUPFD,3);
	fcntl(0,F_DUPFD,4);

	for (i=0; i< writers; i++){
		int fd1[2];
		int fd2[2];

		if (pipe(fd1)==-1) exit(1);
		if (pipe(fd2)==-1) exit(1);

		if (!fork()){
			close(fd2[0]); close(fd2[1]);
			producer(fd1,2*i);
		}

		if (!fork()){
			close(fd1[0]); close(fd1[1]);
			producer(fd2,2*i+1);
		}

		if (!fork()){
			close(3);
			close(4);
			fcntl(fd1[0],F_DUPFD,3);
			fcntl(fd2[0],F_DUPFD,4);
			close(0); 
			close(fd1[0]); close(fd1[1]);
			close(fd2[0]); close(fd2[1]);
			if (!test){
				close(1); 
				int er=execl(WRITER,WRITER,argv[1],argv[2],NULL);
			    fprintf(stderr,"writer exec failed with: %s\n",strerror(errno));
			} else {
				int er=execl(TEST_WRITER,TEST_WRITER,NULL);
			    fprintf(stderr,"test writer exec failed with: %s\n",strerror(errno));
			}

			exit(1);
		}	
		close(fd1[0]); close(fd1[1]);
		close(fd2[0]); close(fd2[1]);		
	}

	if (test) wait_for(3*writers);

	// generate readers

    printf("readers\n");

	char buf[20];
	int o=	sprintf(buf,"%s.out.",READER);
	for (i=0; i< readers; i++){
        printf("readersi loop %d\n",1);
		if (!fork()){
			sprintf(buf+o, "%d",i);
			close(1);
			int fd=open( buf, O_CREAT|O_TRUNC|O_WRONLY,S_IWUSR|S_IRUSR);
			
			fprintf(stderr,"reader open at: %d\n",fd);

			close(0);
			int er= execl(READER,READER,argv[1],argv[2],NULL);
			fprintf(stderr,"reader exec failed with: %s\n",strerror(errno));
			exit(1);
		}		
	}
	wait_for(3*writers+readers);
}

void wait_for(int n){
	while (n>0){
		if (wait(NULL)>0) n--;
	}
	exit(0);
}

void producer(int fd[], int n){
			close(0);
			close(1);
			close(fd[0]);
			fcntl(fd[1],F_DUPFD,1);
			close(fd[1]);
			produce(n);
			exit(0);
}
