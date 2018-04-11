#include<stdio.h>
#include<stdlib.h>

struct data{
	int age;
	char gender;
	float height;
	char name[32];	
};

void receive(void *pointer){
	struct data *dat;
	dat = pointer;
	printf("%s\n","Enter your age:" );
	scanf("%d" ,&dat->age);
	printf("%s\n","Enter your gender:" );
	scanf(" %c" ,&dat->gender);
	printf("%s\n","Enter your height:" );
	scanf("%f" ,&dat->height);
	printf("%s\n","Enter your name:" );
	scanf("%s" ,dat->name);
}

void print(void *pointer){
	struct data *dat;
	dat = pointer;
	printf("Name:\n%s\n", dat->name);
	printf("Gender:\n%c\n", dat->gender);
	printf("Height:\n%f\n", dat->height);
	printf("Age:\n%d\n", dat->age);

}

int main(){
	struct data *pointer;
	pointer = malloc(sizeof(struct data));
	receive(pointer);
	print(pointer);
	FILE *file = fopen("file.txt","w+");
	size_t bytes;
	bytes = sizeof(struct data);
	size_t written = fwrite(pointer,bytes,1,file);
	free(pointer);
	return 0;
}