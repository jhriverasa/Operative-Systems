#include <unistd.h>
#include <stdio.h>


int main(){
	int pipefd[2];
	pid_t pid;
	char buffer[13];
	char hello[12] = "Hello world\n";

	int r = pipe(pipefd);
	if(r == -1)printf("%s\n", "Error creating pipe");
	pid = fork();
	double pimid=0.0;
	if (pid == 0){//child
		close(pipefd[0]);//close read filedescriptor 
		write(pipefd[1], hello, sizeof(hello));
		close(pipefd[1]);
	}else{//parent
		close(pipefd[1]);//close write filedescriptor
		r = read(pipefd[0], buffer,sizeof(hello));
		close(pipefd[0]);
		buffer[12] = '\0'; //end of string
		printf("%s",buffer);
	}


	return 0;
}