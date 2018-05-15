#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#define NUMBYTES 1024*1024*100//change me


int main(){
	clock_t start,end;
	double cpu_time_used;

	pid_t pid;
	key_t key = 1234;
	int shmId;
	char *ap;
	shmId =shmget(key, sizeof(int), IPC_CREAT|0666);
	ap = shmat(shmId,NULL,0);
	pid = fork();
	start=clock();// time start
	int i;
	if(pid == 0){
		char pointer;
		pointer = 'a';
		for(i=0;i<NUMBYTES;i++){
			//writing memory
			*ap = pointer;
		}
	}else{
		
		char a;
		//a= *ap;
		//printf("%c\n", a);
		for(i=0;i<NUMBYTES;i++){
			//reading memory
			a = *ap;	
		}
		shmdt(ap);
		end = clock(); //time stop
		printf("Time used: %f\n", ((double)(end - start)) / CLOCKS_PER_SEC );
		printf("#Bytes sent and read: %d\n", NUMBYTES);
	}


	return 0;
}
