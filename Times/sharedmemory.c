#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#define NUMBYTES 100//change me 


int main(){
	clock_t start,end;
	double cpu_time_used;

	pid_t pid;
	key_t key = 1234;
	int shmId;
	int *ap;
	shmId =shmget(key, sizeof(int), IPC_CREAT|0666);
	ap = shmat(shmId,NULL,0);
	pid = fork();
	start=clock();// time start
	int i;
	if(pid == 0){
		int pointer;
		for(i=0;i<NUMBYTES;i++){
			pointer = 99;
			*ap = pointer;
		}
	}else{
		
		int num = *ap;
		//printf("%d\n", num);
		for(i=0;i<NUMBYTES;i++){
			end = clock(); //time stop
			shmdt(ap);
		}
		printf("Time used: %f\n", ((double)(end - start)) / CLOCKS_PER_SEC );
		printf("#Bytes sent and read: %d\n", NUMBYTES);
	}


	return 0;
}
