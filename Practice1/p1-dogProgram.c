#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//# names in petnames.txt
#define N_PETNAMES 1716
//#indexes in hashtable
#define N_INDEXES 10
//File to Write 10M structs
#define FILE_DATADOGS "dataDogs.txt"
//File with names
#define FILE_NAMES "petnames.txt"


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

//"convert" an integer (i) to string (b[])
char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}


//read petnames.txt and return k-th line.
char* get_name(int k){
	FILE *file;
	char * line=NULL;
	int counter;
	size_t len = 0;
	ssize_t read;

	file =fopen(FILE_NAMES,"r");
	for (counter=0;counter<k;counter++){
		read= getline(&line,&len,file);
	}
	char *ptr = malloc(sizeof(char)*read);
	line[read-1]= '\0';
	ptr =line;
	fclose(file);
	return ptr;
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

//return header (pointer to struct)
void* readHeader(){
	FILE *file;
	struct fileHeader *header;
	file =fopen(FILE_DATADOGS,"rb");
	header = malloc(sizeof(struct fileHeader));
	fread(header,sizeof(struct fileHeader),1,file);
	fclose(file);
	return header;
}

//return register (pointer to dogType struct)
void* readRegister(long position){
	FILE *file;
	struct fileHeader *reg;
	file =fopen(FILE_DATADOGS,"rb");
	reg = malloc(sizeof(struct dogType));
	fseek(file,position,SEEK_SET);
	fread(reg,sizeof(struct dogType),1,file);
	fclose(file);
	return reg;
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
	header = readHeader();

	struct dogType *dt;
	dt = reg;
	long position;
	unsigned long hashed;
	hashed = hash(dt->name , N_INDEXES);
	if(header->head_pos[hashed] == 0 ){ //List is empty

		//write register, head position and counter total_registers updated.
		position = writeRegister(dt);
		header->head_pos[hashed]=position;
		header->total_registers = header->total_registers+1;
		updateHeader(header);
	}else{ //list has one or more elements

		struct dogType *currentReg;
		currentReg = readRegister(header->head_pos[hashed]);
		long tailPosition;
		//get the tail in list and its position.
		tailPosition = header->head_pos[hashed];
		while(currentReg->next_struct != 0){
			tailPosition = currentReg->next_struct;
			currentReg = readRegister(currentReg->next_struct);
		}
		//add new tail
		long newTailPosition;
		newTailPosition = writeRegister(dt);
		header->total_registers = header->total_registers+1;
		//update old tail, now will point to new tail
		//and update header (counter total_registers )
		currentReg->next_struct = newTailPosition;
		updateHeader(header);
		updateRegister(currentReg, tailPosition);


	}

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
void createRegister(){

	long i;
	int antirepeat = 151; //arbitrary prime number
    char atype[32] = "12345678901234567890123456789012";
    char arace[16] = "1234567890123456";


	//build and fill dogType Struct with scanf
	struct dogType *reg;
	reg = malloc(sizeof(struct dogType));
	printf("%s","---------------------------------\n");
	printf("%s","*** Creating New Register ***\n");
	printf("%s","---------------------------------\n\n");
	printf("%s","Please insert pet name (max 32 chars):\n");
	scanf("%s", reg->name);
	printf("%s","Please insert pet age (years)/int:\n");
	scanf("%d", &reg->age);
	printf("%s","Please insert pet weight (kg)/float:\n");
	scanf("%f", &reg->weight);
	printf("%s","Please insert pet height (cm)/int:\n");
	scanf("%d", &reg->height);
	printf("%s","Please insert animal(dog,cat...)(max 32 chars):\n");
	scanf("%s", reg->animal_type);
	printf("%s","Please insert race (max 16 chars):\n");
	scanf("%s", reg->race);
	printf("%s","Please insert pet gender (M/F) :\n");
	scanf(" %c", &reg->gender);
	reg->next_struct = 0; //long

	//here we have reg ready to write into file.
	addRegister(reg);
	free(reg);	
	printf("%s","Register added succesfully!\n");

}

void deleteRegister(void *reg,long position){
	struct dogType *dt;
	dt=reg;
	//is head in list
}

//print menu to show a specific register.
void seeRegister(){
	struct fileHeader *header;
	header = readHeader();
	printf("%s","---------------------\n");
	printf("%s","*** Show Register ***\n");
	printf("%s","----------------------\n\n");
	printf("%s%d\n","Number of existing registers : ",header->total_registers);
	printf("%s%d\n","insert a number between 1 - ",header->total_registers);
	int regNum;
	scanf("%d",&regNum);
	if(regNum<1 || regNum>header->total_registers){ 
		printf("%s","Invalid register number (not in range!).\n");
		seeRegister();
	}else{
		
		long position = sizeof(struct fileHeader) + (regNum-1)*sizeof(struct dogType); 
		struct dogType *reg;
		reg = readRegister(position);

		printf("%s%d%s","------ Register #",regNum," ------\n");
		printf("%s%s\n","Pet name : ",reg->name);
		printf("%s%d\n","Pet age (years): ",reg->age);
		printf("%s%f\n","Pet weight (Kg): ",reg->weight);
		printf("%s%d\n","Pet height (cm): ",reg->height);
		printf("%s%s\n","Animal : ",reg->animal_type);
		printf("%s%s\n","Pet race : ",reg->race);
		printf("%s%c\n","Pet gender (F/M): ",reg->gender);
		printf("%s","------Opening File(Medical Record)------\n");
		//here stuff to open medical record

	}

}

void menuDelRegister(){

	struct fileHeader *header;
	header = readHeader();
	printf("%s","---------------------\n");
	printf("%s","*** Delete Register ***\n");
	printf("%s","----------------------\n\n");
	printf("%s%d\n","Number of existing registers : ",header->total_registers);
	printf("%s%d\n","insert a number between 1 - ",header->total_registers);
	int regNum;
	scanf("%d",&regNum);
	if(regNum<1 || regNum>header->total_registers){ 
		printf("%s","Invalid register number (not in range!).\n");
		seeRegister();
	}else{
		
		long position = sizeof(struct fileHeader) + (regNum-1)*sizeof(struct dogType); 
		struct dogType *reg;
		reg = readRegister(position);

		printf("%s%d%s","------ Register #",regNum," ------\n");
		printf("%s%s\n","Pet name : ",reg->name);
		printf("%s%d\n","Pet age (years): ",reg->age);
		printf("%s%f\n","Pet weight (Kg): ",reg->weight);
		printf("%s%d\n","Pet height (cm): ",reg->height);
		printf("%s%s\n","Animal : ",reg->animal_type);
		printf("%s%s\n","Pet race : ",reg->race);
		printf("%s%c\n","Pet gender (F/M): ",reg->gender);
		printf("%s","--------------------------------\n\n");
		printf("%s","----Do you want to delete this register? (y/n)---\n");
		char ans;
		scanf(" %c",&ans);
		if(ans=='y'){
			//delete register
			deleteRegister(reg,position);


		}else if(ans=='n'){
			menuDelRegister();
		}else{
			printf("%s","----Invalid answer!!!---\n");
			menuDelRegister();
		}

	}

}


void menuSearchRegister(){
	struct fileHeader *header;
	header = readHeader();
	printf("%s","---------------------\n");
	printf("%s","*** Search Register ***\n");
	printf("%s","----------------------\n\n");
	printf("%s","Enter pet name (Max 31 chars)\n");
	
	char regName[32];
	scanf("%s", regName);
	unsigned long hashed;
	hashed = hash(regName,N_INDEXES);


	if(header->head_pos[hashed] == 0 ){ //List is empty

		//write register, head position and counter total_registers updated.
		printf("%s","No registers found!.\n");
	}else{ //list has one or more elements

		//edit here
		struct dogType *currentReg;
		currentReg = readRegister(header->head_pos[hashed]);
		long tailPosition;
		//get the tail in list and its position.
		tailPosition = header->head_pos[hashed];
		while(currentReg->next_struct != 0){
			tailPosition = currentReg->next_struct;
			currentReg = readRegister(currentReg->next_struct);
		}
		//add new tail
		long newTailPosition;
		newTailPosition = writeRegister(dt);
		header->total_registers = header->total_registers+1;
		//update old tail, now will point to new tail
		//and update header (counter total_registers )
		currentReg->next_struct = newTailPosition;
		updateHeader(header);
		updateRegister(currentReg, tailPosition);


	}


	// if(false){ //change me to something useful (error validating)
	// 	printf("%s","Invalid register number (not in range!).\n");
	// 	seeRegister();
	// }else{
		
	// 	long position = sizeof(struct fileHeader) + (regNum-1)*sizeof(struct dogType); 
	// 	struct dogType *reg;
	// 	reg = readRegister(position);

	// 	printf("%s%d%s","------ Register #",regNum," ------\n");
	// 	printf("%s%s\n","Pet name : ",reg->name);
	// 	printf("%s%d\n","Pet age (years): ",reg->age);
	// 	printf("%s%f\n","Pet weight (Kg): ",reg->weight);
	// 	printf("%s%d\n","Pet height (cm): ",reg->height);
	// 	printf("%s%s\n","Animal : ",reg->animal_type);
	// 	printf("%s%s\n","Pet race : ",reg->race);
	// 	printf("%s%c\n","Pet gender (F/M): ",reg->gender);
	// 	printf("%s","------Opening File(Medical Record)------\n");
		

	// 	//here stuff to open medical record

	// }

}



void bye(){
	//Exit
	printf("%s","\n Good Bye.\n");
	exit(0);
}

void menu(){
	//main menu
	printf("%s","---------------------------------\n");
	printf("%s","*** Welcome to Animal-Vet-Care ***\n");
	printf("%s","---------------------------------\n\n");
	printf("%s","Please select one option and press Enter:\n");
	printf("%s","1. Enter a new register.\n");
	printf("%s","2. Show an existing register.\n");
	printf("%s","3. Delete an existing register.\n");
	printf("%s","4. Look for an existing register.\n");
	printf("%s","5. Exit.\n");
	int option;
	scanf("%d",&option);
	if(option == 1){
		createRegister();
	}else if(option ==2){
		seeRegister();
	}else if(option ==3){
		//menuDelRegister();
	}else if(option ==4){
		menuSearchRegister();
	}else if(option ==5){
		bye();
	}else{
		printf("%s","Please enter a valid option!!\n" );
		menu();
	}


}


int main(){

	menu();
	return 0;

}