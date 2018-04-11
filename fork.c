#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

int main(){
	pid_t pid;
	pid = fork();

	if(pid == -1){
		perror("Error forking");
		exit(-1);
	}
	if (pid==0){
		printf("\nI'm your son and i wanna tell you that: \n-----------\n" );
		FILE *file;
		file =fopen("dat.dat","r");
		while(file == NULL){
			printf("%s\n","lol" );
		}
		//read data from dat.dat
		char *buffer;
		size_t result;
		buffer = malloc(sizeof(char)*11);
		result = fread(buffer,sizeof(char)*11,1,file);
		printf("%s\n", buffer);

		printf("\n-------------\n");


	}else{
		char * hello = "hello world";
		printf("%s\n","Hey Im your father" );
		FILE *file;
		file = fopen("dat.dat", "w+");
		fwrite(hello,11,sizeof(char), file);
		fclose(file);
	}


	return 0;
}

