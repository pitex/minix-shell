#include <unistd.h>
#include <stdlib.h>

#define RUNDS_MAX 1000
#define SERIES_MAX 100
#define SLEEP_MAX 10000

void produce(int n){
	srand(n);
	int turns= rand()%RUNDS_MAX;
	int i,j;

	for (i=0; i<turns; i++){
		int time = rand()%SLEEP_MAX;

		usleep(time);

		int serie = rand()%SERIES_MAX;
		for (j=0; j<serie; j++){
			int val= rand();
			write(1,&val,4);
		}
	}
}
