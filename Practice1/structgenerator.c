#include <stdio.h>
#include <stdlib.h>
#include <time.h>


struct dogType{
	int age;
	float weight;
	char gender;
	float height;
	char name[32];
	char animal_type[32];
	char race[16];
};


const int n= 1716;
//read petnames.txt and return an array of names.
char* get_name(int k){
	FILE *file;
	char * line=NULL;
	int counter;
	size_t len = 0;
	ssize_t read;

	file =fopen("petnames.txt","r");
	for (counter=0;counter<k;counter++){
		read= getline(&line,&len,file);
	}
	char *ptr = malloc(sizeof(char)*read);
	ptr =line;
	fclose(file);
	return ptr;
}

int randint(int a,int b){
	int r;

    srand(time(0));
    if (a > b)
        r=((rand() % (a-b+1)) + b);
    else if (b > a)
        r=((rand() % (b-a+1)) + a);
    else
        r= a;

	return r;

}

float randfloat( float min, float max ){
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );     
}

char* randname(char** names){
    return names[randint(0,n-1)];
}


int main(){

	long i;
	char *k = malloc(64);
	FILE *file;
	file = fopen("dataDogs.txt","w+");
	int antirepeat = 151;
	
    int a=2;
    int b=16;
    float min= 5;
    float max= 50;
   
    printf("Random int %d\n", randint(a,b));
    printf("Random float %f\n", randfloat(min,max));
    for(i=0;i<10000;i++){ // put it equals 10 M (now 1M)
    	antirepeat =  (antirepeat*19+i) % n +1;
    	fputs(get_name(antirepeat),file);
   		

    }

    
    return 0;
}
