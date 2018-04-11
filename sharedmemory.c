#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

int main(){
	pid_t pid;
	key_t key = 1234;
	int shmId;
	double *ap;
	shmId =shmget(key, sizeof(double), IPC_CREAT|0666);
	ap = shmat(shmId,NULL,0);
	pid = fork();
	if(pid == 0){
		double pointer;
		pointer = 1.0;
		*ap = pointer;
	}else{
		printf("%f\n", *ap );
		shmdt(ap);
	}


	return 0;
}
