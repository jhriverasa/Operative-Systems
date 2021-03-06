#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


//# names in petnames.txt
#define N_PETNAMES 1716
//#indexes in hashtable
#define N_INDEXES 1000
//File to Write 10M structs
#define FILE_DATADOGS "dataDogs.dat"
//File with names
#define FILE_NAMES "petnames.txt"

//headers to avoid warnings
void bye();
void goback();
void menu();

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

//"convert" a long integer (i) to string (b[])
char* itoa(long i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    long shifter = i;
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

		//write register, head position and counter total registers updated.
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
		//and update header (counter total registers )
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

//Create and/or open Medical record in default text editor
void showMedicalRecord(long position){
	
		FILE *medrec;
		char f_name[10];
		itoa(position, f_name);
		char command[23];
		command[0]='x';
		command[1]='d';
		command[2]='g';
		command[3]='-';
		command[4]='o';
		command[5]='p';
		command[6]='e';
		command[7]='n';
		command[8]=' ';
		int z;
		for(z=9;z<23;z++)command[z]=f_name[z-9];

		//printf("%s\n", f_name);
		medrec = fopen(f_name, "w");
		if(medrec != NULL){
			fputs("-----Medical Record ---- \n #Edit me",medrec);
		}else{
			medrec = fopen(f_name, "a");
			fputs("-----Medical Record ---- \n #Edit me",medrec);
		}

		fclose(medrec);
		system(command);
}

void goback(){
	printf("%s","----Go back to main menu? (y/n)---\n");
	char ans;
	scanf(" %c",&ans);
	if(ans=='y'){
		//delete register
		menu();
	}else if(ans=='n'){
		bye();
	}else{
		printf("%s","----Invalid answer!!!---\n");
		
	}
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
	goback();

}

//swap the register in position with the last one, updating pointers in list.
void swapRegister(void *reg, long position){
	struct dogType *RegA;
	RegA=reg;
	long PosA =position;
	int optionA;

	struct fileHeader *header;
	header = malloc(sizeof(struct fileHeader));
	readHeader(header);

	struct dogType *RegB;
	long PosB = sizeof(struct fileHeader) + (header->total_registers-1)*sizeof(struct dogType);
	RegB = malloc(sizeof(struct dogType));
	readRegister(PosB , RegB);
	
	int optionB;

	struct dogType *currentRegA;
	currentRegA = malloc(sizeof(struct dogType));
	long currentPosA;

	struct dogType *currentRegB;
	currentRegB = malloc(sizeof(struct dogType));
	long currentPosB;

	// updates for A
	unsigned long hashedA;
	hashedA = hash(RegA->name,N_INDEXES);
	
	

	if(header->head_pos[hashedA] == PosA){//is head ?
		optionA=1;
		
		
	}else{ //isn't head

		//update previous register in list
		
		currentPosA=header->head_pos[hashedA];
		readRegister(header->head_pos[hashedA] , currentRegA);

		while(currentRegA->next_struct != PosA){
			currentPosA=currentRegA->next_struct;
			readRegister(currentRegA->next_struct, currentRegA);
		}
		currentRegA->next_struct = PosB;
		optionA=2;
		
	}


	//updates for B
	unsigned long hashedB;
	hashedB = hash(RegB->name,N_INDEXES);
	
	
	if(header->head_pos[hashedB] == PosB){//is head ?
		optionB=1;
		
	}else{ //isn't head

		//update previous register in list
		currentPosB=header->head_pos[hashedB];
		readRegister(header->head_pos[hashedB] , currentRegB);

		while(currentRegB->next_struct != PosB){
			currentPosB=currentRegB->next_struct;
			readRegister(currentRegB->next_struct, currentRegB);
		}
		currentRegB->next_struct = PosA;
		optionB=2;
		
	}

	//write updates in file and swap positions.
	if(optionA==1)header->head_pos[hashedA]= PosB;
	if(optionA==2)updateRegister(currentRegA,currentPosA);
	if(optionB==1)header->head_pos[hashedB]= PosA;
	if(optionB==2)updateRegister(currentRegB,currentPosB);
	updateHeader(header);

	updateRegister(RegA,PosB);
	updateRegister(RegB,PosA);

}



void deleteRegister(void *reg,long delPosition){
	struct dogType *delReg;
	delReg=reg;

	swapRegister(delReg, delPosition);
	struct fileHeader *header;
	header = malloc(sizeof(struct fileHeader));
	readHeader(header);
	delPosition = sizeof(struct fileHeader) + (header->total_registers-1)*sizeof(struct dogType);

	unsigned long hashed;
	hashed = hash(delReg->name,N_INDEXES);
	
	if (delReg->next_struct == 0){//is tail?
		if(header->head_pos[hashed] == delPosition){//is head too? (list with a single element)
			
			header->head_pos[hashed]= 0;
			header->total_registers = header->total_registers-1;
			updateHeader(header);


		}else{ //is tail and not head

			//look for register which is pointing to it and will be the tail now.
			struct dogType *currentReg;
			currentReg = malloc(sizeof(struct dogType));
			long currentPos=header->head_pos[hashed];
			readRegister(header->head_pos[hashed] , currentReg);

			while(currentReg->next_struct != delPosition){
				currentPos=currentReg->next_struct;
				readRegister(currentReg->next_struct, currentReg);
			}
			currentReg->next_struct = 0;

			updateRegister(currentReg,currentPos);
			header->total_registers = header->total_registers-1;
			updateHeader(header);


		}


	}else{ //not tail 
		if(header->head_pos[hashed] == delPosition){ // is head and not the only one in list
			header->head_pos[hashed]= delReg->next_struct;

			header->total_registers = header->total_registers-1;
			updateHeader(header);

		}
		else{ //is not tail or head

			struct dogType *currentReg;
			currentReg = malloc(sizeof(struct dogType));

			long currentPos=header->head_pos[hashed];
			long nextPos = delReg->next_struct;
			readRegister(header->head_pos[hashed] , currentReg);
			

			while(currentReg->next_struct != delPosition){
				currentPos=currentReg->next_struct;
				readRegister(currentReg->next_struct, currentReg);
			}
			currentReg->next_struct= nextPos;

			updateRegister(currentReg,currentPos);
			header->total_registers = header->total_registers-1;
			updateHeader(header);


		}

	}

	long length = sizeof(struct fileHeader) + (header->total_registers)*sizeof(struct dogType); 
	truncate(FILE_DATADOGS,length);

}

//print menu to show a specific register.
void seeRegister(){
	struct fileHeader *header;
	header = malloc(sizeof(struct fileHeader));
	readHeader(header);
	printf("%s","---------------------\n");
	printf("%s","*** Show Register ***\n");
	printf("%s","----------------------\n\n");
	printf("%s%d\n","Number of existing registers : ",header->total_registers);
	printf("%s%d\n","insert a number between 1 and ",header->total_registers);
	int regNum;
	scanf("%d",&regNum);
	if(regNum < 1 || regNum > header->total_registers){ 
		printf("%s","Invalid register number (not in range!).\n");
		seeRegister();
	}else{
		
		//show register in position regNum
		long position = sizeof(struct fileHeader) + (regNum-1)*sizeof(struct dogType); 
		struct dogType *reg;
		reg= malloc(sizeof(struct dogType));
		readRegister(position,reg);

		printf("%s%d%s","------ Register #",regNum," ------\n");
		printf("%s%s\n","Pet name : ",reg->name);
		printf("%s%d\n","Pet age (years): ",reg->age);
		printf("%s%f\n","Pet weight (Kg): ",reg->weight);
		printf("%s%d\n","Pet height (cm): ",reg->height);
		printf("%s%s\n","Animal : ",reg->animal_type);
		printf("%s%s\n","Pet race : ",reg->race);
		printf("%s%c\n","Pet gender (F/M): ",reg->gender);
		printf("%s","------Opening File(Medical Record)------\n");

		//medical record
		showMedicalRecord(position);

		free(reg);
		


	}
	
	free(header);
	goback();

}

void menuDelRegister(){

	struct fileHeader *header;
	header = malloc(sizeof(struct fileHeader));
	readHeader(header);
	printf("%s","---------------------\n");
	printf("%s","*** Delete Register ***\n");
	printf("%s","----------------------\n\n");
	printf("%s%d\n","Number of existing registers : ",header->total_registers);
	printf("%s%d\n","insert a number between 1 - ",header->total_registers);
	int regNum;
	scanf("%d",&regNum);
	if(regNum<1 || regNum>header->total_registers){ 
		printf("%s","Invalid register number (not in range!).\n");
		menuDelRegister();
	}else{
		
		long position = sizeof(struct fileHeader) + (regNum-1)*sizeof(struct dogType); 
		struct dogType *reg;
		reg= malloc(sizeof(struct dogType));
		readRegister(position,reg);

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
			printf("%s","---File deleted succesfully---\n");
			goback();

		}else if(ans=='n'){
			menu();
		}else{
			printf("%s","----Invalid answer!!!---\n");
			menuDelRegister();
		}


	}


}


void menuSearchRegister(){
	struct fileHeader *header;
	header = malloc(sizeof(struct fileHeader));
	readHeader(header);
	printf("%s","---------------------\n");
	printf("%s","*** Search Register ***\n");
	printf("%s","----------------------\n\n");
	printf("%s","Enter pet name (Max 31 chars)\n");
	
	//receive name and calculate its hash.
	char regName[32];
	scanf("%s", regName);
	unsigned long hashed;
	hashed = hash(regName,N_INDEXES);
	int counter;
	counter=0;

	if(header->head_pos[hashed] == 0 ){ //List is empty

		printf("%d%s",counter," registers found!.\n");
		goback();
	
	}else{ //list has one or more elements

		//read every element in list and compare if their names are equal to asked name.
		struct dogType *currentReg;
		currentReg = malloc(sizeof(struct dogType));
		long index[50000];

		readRegister(header->head_pos[hashed],currentReg);
		if( strcasecmp(currentReg->name , regName) == 0 ){
			index[counter]=header->head_pos[hashed];
			counter++;
		}

		long tailPosition;
		tailPosition = header->head_pos[hashed];

		//Count every register found and write its positions in index[x]
		while(currentReg->next_struct != 0){
			tailPosition = currentReg->next_struct;
			readRegister(currentReg->next_struct, currentReg);
			if( strcasecmp(currentReg->name , regName) == 0 ){
				index[counter]=tailPosition;
				counter++;
			}
		}

		//here we have all registers found in Counter and all their indexes in index[x]
		printf("\n%d%s",counter," registers found!.\n");

		//now ask for one of them (if we found 1 or more).
		if(counter==0)menuSearchRegister();

		printf("%s","------------------\n");
		printf("%s","*** Which one? ***\n");
		printf("%s","------------------\n");
		printf("%s%d\n","Insert a number between 1 and ",counter);
		int regNum;
		scanf("%d",&regNum);
		if(regNum<1 || regNum > counter){ 
			printf("%s","Invalid register number (not in range!) :( .\n");
			menuSearchRegister();
		}else{
			
			//look in index, read and show register.
			long position;
			position = index[regNum-1];

			struct dogType *reg;
			reg= malloc(sizeof(struct dogType));
			readRegister(position,reg);

			printf("\n%s%d%s","------ Register found #",regNum," ------\n");
			printf("%s%s\n","Pet name : ",reg->name);
			printf("%s%d\n","Pet age (years): ",reg->age);
			printf("%s%f\n","Pet weight (Kg): ",reg->weight);
			printf("%s%d\n","Pet height (cm): ",reg->height);
			printf("%s%s\n","Animal : ",reg->animal_type);
			printf("%s%s\n","Pet race : ",reg->race);
			printf("%s%c\n","Pet gender (F/M): ",reg->gender);
			printf("%s","-------------------------------------\n");
			free(reg);
			goback();

		}
	}
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
		menuDelRegister();
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