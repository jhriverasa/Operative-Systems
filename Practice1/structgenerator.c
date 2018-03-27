#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//# names in petnames.txt
#define N_PETNAMES 1716
//#indexes in hashtable
#define N_INDEXES 100
//File to Write 10M structs
#define FILE_DATADOGS "dataDogs.txt"
//File with names
#define FILE_NAMES "petnames.txt"
//Num of registers to write
#define N_TOTALREGISTERS 1000


struct dogType{
	int age;
	float weight;
	char gender;
	int height;  //cm
	char name[32];
	char animal_type[32];
	char race[16];
	long next_struct;
};

struct fileHeader{
	int total_registers;
	long head_pos[N_INDEXES];
};




//read petnames.txt and load names in a an array of strings (ptr).
void load_names(char ptr[N_PETNAMES][32]){
	FILE *file;
	char * line=NULL;
	int counter;
	size_t len = 0;
	ssize_t read;
	int c;

	file =fopen(FILE_NAMES,"r");
	for (counter=0;counter<N_PETNAMES;counter++){
		read= getline(&line,&len,file);
		line[read-1]= '\0';

		for(c=0;c<read;c++){
			ptr[counter][c]= line[c];
		}

	}
	
	
	fclose(file);

}


//return a random integer between a and b
int randint(int a,int b){
	int r;

    srand(time(0));
    if (a > b){
        r=((rand() % (a-b+1)) + b);
    }else if (b > a){
        r=((rand() % (b-a+1)) + a);
    }else{
        r= a;
    }

	return r;

}

//return a random float between min and max
float randfloat( float min, float max ){
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );     
}

//append a register to dataDogs.txt/.bin
//and return position (offset) in file.
long writeRegister(void *regdt){
	FILE *file;
	struct dogType *dt;
	dt = regdt;

	long position;

	file = fopen(FILE_DATADOGS,"ab");
	position= ftell(file);
	
	fwrite(dt,sizeof(struct dogType),1,file);
	fclose(file);
	return position;
}

//return header in a given pointer
void readHeader(struct fileHeader *header){
	FILE *file;
	file =fopen(FILE_DATADOGS,"rb");
	fread(header,sizeof(struct fileHeader),1,file);
	fclose(file);
}

//return register (pointer to dogType struct) in a given pointer
void readRegister(long position, struct dogType *reg){
	FILE *file;
	file =fopen(FILE_DATADOGS,"rb");
	fseek(file,position,SEEK_SET);
	fread(reg,sizeof(struct dogType),1,file);
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

//update Header in dataDogs.txt/bin
void updateHeader(void *newHeader){
	FILE *file;
	//overwrite header bytes.
	file = fopen(FILE_DATADOGS,"rb+");

	struct fileHeader *header;
	header = newHeader;

	fseek(file,0,SEEK_SET);
	fwrite(header,sizeof(struct fileHeader),1,file);
	fclose(file);


}

//update register in dataDogs.txt/bin
void updateRegister(void *newRegister,long position){
	FILE *file;
	//overwrite register bytes.
	file = fopen(FILE_DATADOGS,"rb+");

	struct dogType *dt;
	dt = newRegister;

	fseek(file,position,SEEK_SET);
	fwrite(dt,sizeof(struct dogType),1,file);
	fclose(file);


}



// Add register (dogType) to file (dataDogs.txt/.bin) and update header
void addRegister(void *reg){
	struct fileHeader *header;
	header = malloc(sizeof(struct fileHeader));
	readHeader(header);

	struct dogType *dt;
	dt = reg;
	long position;
	unsigned long hashed;
	hashed = hash(dt->name , N_INDEXES);
	if(header->head_pos[hashed] == 0 ){ //List is empty

		//write register, head position and counter totalregisters updated.
		position = writeRegister(dt);
		header->head_pos[hashed]=position;
		header->total_registers = header->total_registers+1;
		updateHeader(header);
	}else{ //list has one or more elements

		struct dogType *currentReg;
		currentReg = malloc(sizeof(struct dogType));
		readRegister(header->head_pos[hashed] , currentReg);
		long tailPosition;
		//get the tail in list and its position.
		tailPosition = header->head_pos[hashed];
		while(currentReg->next_struct != 0){
			tailPosition = currentReg->next_struct;
			readRegister(currentReg->next_struct , currentReg);
		}
		//add new tail
		long newTailPosition;
		newTailPosition = writeRegister(dt);
		header->total_registers = header->total_registers+1;
		//update old tail, now will point to new tail
		//and update header (counter totalregisters )
		currentReg->next_struct = newTailPosition;
		updateHeader(header);
		updateRegister(currentReg, tailPosition);
		free(currentReg);

	}
	free(header);

}



//create file and write header
void createHeader(){
	FILE *file;
	file = fopen(FILE_DATADOGS,"wb+");
	struct fileHeader *header;
	header = malloc(sizeof(struct fileHeader));
	header->total_registers = 0;
	int i;
	for(i=0;i<N_INDEXES;i++){
		header->head_pos[i]=0;
	}
	fwrite(header,sizeof(struct fileHeader),1,file);
	free(header);
	fclose(file);
}



//generate 100/10M registers in dataDogs.txt/.bin
void createRegisters(){

	long i;
	int antirepeat = 151; //arbitrary prime number
    //char atype[32] = "12345678901234567890123456789012";
    //char arace[16] = "1234567890123456";
	char names[N_PETNAMES][32];
	load_names(names); 


    for(i=0;i<N_TOTALREGISTERS;i++){ // put it equals 10 M (now 100 for testing)

    	//build and fill dogType Struct with pseudo-random data
    	struct dogType *reg;
    	reg = malloc(sizeof(struct dogType));
    	reg->age = randint(0,20);
		reg->weight = randfloat(1.0,5.0);
		reg->height = randint(30,200);
		antirepeat =  (antirepeat*19+i) % N_PETNAMES;
		strcpy(reg->name, names[antirepeat]);
		strcpy(reg->animal_type, "1234567890123456789012345678901\0");
		strcpy(reg->race, "123456789012345\0");
		reg->next_struct = 0; //long
		if(i%2 == 0){
			reg->gender = 'H';
		}else{
			reg->gender = 'M';
		}

		//here we have reg ready to write into file.
		addRegister(reg);
		free(reg);

    }

}



int main(){
	createHeader();
	createRegisters();
    return 0;
}