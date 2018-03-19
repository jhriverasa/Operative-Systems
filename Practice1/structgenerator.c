#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


struct dogType{
	int age;
	float weight;
	char gender;
	int height;  //cm
	char name[32];
	char animal_type[32];
	char race[16];
};

//# names in petnames.txt
const int n= 1716;


//read petnames.txt and return k-th line.
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


//return a random integer between a and b
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

//return a random float between min and max
float randfloat( float min, float max ){
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );     
}


void writeStruct(void *reg){
	FILE *file;
	struct dogType *dt;
	dt = reg;

	file = fopen("dataDogs.txt","wb+");
	fwrite(&dt,sizeof(struct dogType),1,file);
	fclose(file);
	
}

//return Dan Bernstein Hash Function with modulo operation.
unsigned long hash(unsigned char *str, int mod){

	unsigned long hash = 5381;
	int c;

	while(c= *str++){
		hash = ((hash << 5) + hash) + c;
	}
	return hash % mod;

}






int main(){

	long i;
	FILE *file;
	file = fopen("dataDogs.txt","w+");

	int antirepeat = 151; //arbitrary prime number
    char atype[32] = "12345678901234567890123456789012";
    char arace[16] = "1234567890123456";


    for(i=0;i<100;i++){ // put it equals 10 M (now 100 for testing)

    	//build and fill dogType Struct with pseudo-random data
    	struct dogType *reg;
    	reg = malloc(sizeof(struct dogType));
    	reg->age = randint(0,20);
		reg->weight = randfloat(1.0,5.0);
		reg->height = randint(30,200);
		antirepeat =  (antirepeat*19+i) % n +1; 
		strcpy(reg->name,get_name(antirepeat));
		strcpy(reg->animal_type, atype);
		strcpy(reg->race, "1234567890123456");

    	
		//here we have reg ready to write into file.
		

		//printf("%lu\n", hash(reg->name,10) );
		delme[hash(reg->name,10)]++; 

		if(i==0){
			// printf("%d\n",reg->age );
			// printf("%f\n",reg->weight );
			// printf("%d\n",reg->height );
			// printf("%s\n",reg->name );
			// printf("%s\n",reg->animal_type );
			// printf("%s\n",reg->race );
			writeStruct(reg);	
		}


    	
    }


    
    return 0;
}
