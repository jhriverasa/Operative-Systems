#include <stdio.h>
#include <stdlib.h>

const int n= 1717;
//read petnames.txt and return an array of names.
char** load_names(){
FILE *file;
	 //Constant number of lines.
	FILE *wfile;
	char *line=NULL;
	size_t len =0;
	ssize_t read;
	int counter=0;
	char *names[n]; 
	
	//open petnames.txt | mode :read
	file =fopen("petnames.txt","r");
	if(file==NULL){
		exit(1);
	}
	
	//read names (line by line) and load into memory.
	while((read = getline(&line,&len,file)) != -1 ){
		names[counter]=line;
//		printf("Len %zu :\n", read);
//		printf("%s\n", line);
		counter++;

	}

	//close stream
	fclose(file);
	if (line){
		free(line);
	}

	char** r = names;
	return r;
}

int main(){
	


	return 0;
}
