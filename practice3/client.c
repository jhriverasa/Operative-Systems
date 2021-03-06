#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>
#define PORT 6112


//#indexes in hashtable
#define N_INDEXES 10000

//commands constants for server connection
#define READ_HEADER 1
#define READ_REGISTER 2
#define SHOW_MEDICAL_RECORD 3
#define CREATE_REGISTER 4
#define FIND_REGISTER 5
#define READ_FOUND_REGISTER 6
#define DELETE_REGISTER 7
#define EXIT_CLIENT 8

//Local variable
int sock;

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


//return Dan Bernstein Hash Function with modulo operation.
unsigned long hash(unsigned char *str, int mod){
    unsigned long hash = 5381;
    int c;
    while(c= *str++){
        hash = ((hash << 5) + hash) + c;
    }
    return hash % mod;

}



int sendMsg(int command, void *header,void *reg, long position,struct dogType *dt)
{

    int valread;
    int *msg = malloc(sizeof(int));


    //Request Type
    *msg = command;
    //printf("%d\n", *msg);
    send(sock , msg , sizeof(int) , 0 );


    if(*msg==READ_HEADER){
        valread = recv(sock,header,sizeof(struct fileHeader),0 );
        if (valread==-1)printf("VALHAR VERGHULIS\n");
       
    }

    if(*msg==READ_REGISTER){
        long *positionP=malloc(sizeof(long));
        *positionP= position;
        send(sock, positionP, sizeof(long), 0);
        valread = recv(sock , reg ,sizeof(struct dogType),0 );
        if (valread==-1)printf("VALHAR VERGHULIS\n");
        
    }

    if(*msg==SHOW_MEDICAL_RECORD){

        long *positionP=malloc(sizeof(long));
        *positionP= position; 
        send(sock, positionP, sizeof(long), 0);
        
    }

    if(*msg==CREATE_REGISTER){
        send(sock, reg , sizeof(struct dogType) , 0);
        
    }

    if(*msg==FIND_REGISTER){
        int length = position;
        char *nameP=malloc(length);
        int *sizelen = malloc(sizeof(int));
        *sizelen=length;
        send(sock, sizelen , sizeof(int) , 0);
        nameP= header;  //header is name
        send(sock, nameP , length , 0);

        valread= recv(sock,reg,sizeof(int),0);
        if (valread==-1)printf("VALHAR VERGHULIS\n");
        
    }

    if(*msg==READ_FOUND_REGISTER){
        int length = position;
        char *nameP=malloc(length);
        int *sizelen = malloc(sizeof(int));
        *sizelen=length;
        send(sock, sizelen , sizeof(int) , 0);
        nameP= header;  //header is name
        send(sock, nameP , length , 0);

        //reg is numreg 
        send(sock,reg,sizeof(long),0);

        //dt = dogtype asked
        valread= recv(sock,dt,sizeof(struct dogType),0);
        if (valread==-1)printf("VALHAR VERGHULIS\n");
        
    }

    if(*msg==DELETE_REGISTER){
        long *positionP=malloc(sizeof(long));
        *positionP= position;
        send(sock, positionP, sizeof(long), 0);
        send(sock , reg ,sizeof(struct dogType),0 );
       
        
    }

    if(*msg==EXIT_CLIENT){
        //do nothing
        close(sock);
        
    }



    return 0;
}





void bye(){
    //Exit
    sendMsg(EXIT_CLIENT,NULL,NULL,0,NULL);
    printf("%s","\n Good Bye.\n");
    exit(0);
}

//return header in a given pointer
void readHeader(struct fileHeader *header){
    sendMsg(READ_HEADER,header,NULL,0,NULL);
}

void readRegister(long position, struct dogType *reg){
    sendMsg(READ_REGISTER,NULL,reg,position,NULL);
}

void deleteRegister(void *reg,long delPosition){
    sendMsg(DELETE_REGISTER,NULL,reg,delPosition,NULL);
}

void showMedicalRecord(long position){
    sendMsg(SHOW_MEDICAL_RECORD,NULL,NULL,position,NULL);
}
void addRegister(void *reg){
    sendMsg(CREATE_REGISTER,NULL,reg,0,NULL);
}

long findRegister(void *name,long len){
    long *foundRegs = malloc(sizeof(long));
    sendMsg(FIND_REGISTER,name,foundRegs,len,NULL);
    return *(long*)foundRegs;
}

struct dogType * readFoundRegister(void *name, void *numreg,long len){
    struct dogType *data=malloc(sizeof(struct dogType));
    sendMsg(READ_FOUND_REGISTER,name,numreg,len,data);
    return  data;
}

void goback(){
    printf("%s","----Go back to main menu? (y/n)---\n");
    char ans;
    scanf(" %c",&ans);
    if(ans=='y'){
        menu();
    }else if(ans=='n'){
        bye();
    }else{
        printf("%s","----Invalid answer!!!---\n");
        
    }
}


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

    if(header->head_pos[hashed] == 0 ){ //List is empty

        printf("%d%s",0," registers found!.\n");
        goback();
    
    }else{ //list has one or more elements
        
        
        //read every element in list and compare if their names are equal to asked name.
        int len = strlen(regName);
        int counter =findRegister(regName,len);


        //here we have all registers found in Counter and all their indexes in index[x]
        printf("\n%d%s",counter," registers found!.\n");

        //now ask for one of them (if we found 1 or more).
        if(counter==0)goback();

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
            
            // show register.

            struct dogType *reg;
            reg= malloc(sizeof(struct dogType));
            len = strlen(regName);
            int *numreg = malloc(sizeof(int)); 
            *numreg = regNum;
            reg = readFoundRegister(regName,numreg,len);

            

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

int main (int argc, char const *argv[]){
    struct sockaddr_in address;
    int valread;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form

    //argv[1]= ip
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    //wait aprox 7 s for connection
    int synRetries=2;
    setsockopt(sock,IPPROTO_TCP,TCP_SYNCNT,&synRetries,sizeof(synRetries));
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }


    menu();
    return 0;
}