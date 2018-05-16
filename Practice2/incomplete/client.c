
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
#define N_INDEXES 1000

//commands constants for server connection
#define READ_HEADER 1
#define READ_REGISTER 2
#define SHOW_MEDICAL_RECORD 3
#define CREATE_REGISTER 4
#define REGISTERS_FOUND 5


//headers to avoid warnings
void bye();
void goback();
void menu();


//int main(int argc, char const *argv[])


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



int sendMsg(int command, void *header,void *reg, long position)
{

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    int *msg = malloc(sizeof(int));
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
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


    //Request Type
    *msg = command;
    printf("%d\n", *msg);
    send(sock , msg , sizeof(int) , 0 );


    if(*msg==READ_HEADER){
        valread = recv(sock,header,sizeof(struct fileHeader),0 );
        close(sock);
       
    }

    if(*msg==READ_REGISTER){
        long *positionP=malloc(sizeof(long));
        *positionP= position;
        send(sock, positionP, sizeof(long), 0);
        valread = recv(sock , reg ,sizeof(struct dogType),0 );
        close(sock);
        
    }

    if(*msg==SHOW_MEDICAL_RECORD){

        long *positionP=malloc(sizeof(long));
        *positionP= position; 
        send(sock, positionP, sizeof(long), 0);
        close(sock);
        
    }

    if(*msg==CREATE_REGISTER){
        send(sock, reg , sizeof(struct dogType) , 0);
        close(sock);
        
    }

    if(*msg==REGISTERS_FOUND){
        long *hashedP=malloc(sizeof(long));
        *hashedP= position;  //position is hashed
        send(sock, hashedP , sizeof(long) , 0);
        close(sock);
        
    }

    return 0;
}





void bye(){
    //Exit
    printf("%s","\n Good Bye.\n");
    exit(0);
}

//return header in a given pointer
void readHeader(struct fileHeader *header){
    sendMsg(READ_HEADER,header,NULL,0);
}

void readRegister(long position, struct dogType *reg){
    sendMsg(READ_REGISTER,NULL,reg,position);
}

void showMedicalRecord(long position){
    sendMsg(SHOW_MEDICAL_RECORD,NULL,NULL,0);
}
void addRegister(void *reg){
    sendMsg(CREATE_REGISTER,NULL,reg,0);
}

void registersFound(long hashed){
    sendMsg(REGISTERS_FOUND,NULL,NULL,hashed);
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
        //sleep(2);
        //showMedicalRecord(position);

        free(reg);
        


    }
    
    free(header);
    //goback();

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

        printf("%d%s",counter," registers found!.\n");
        goback();
    
    }else{ //list has one or more elements

        //read every element in list and compare if their names are equal to asked name.
        registersFound(*hashed);

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
        //menuSearchRegister();
    }else if(option ==5){
        bye();
    }else{
        printf("%s","Please enter a valid option!!\n" );
        menu();
    }


}

int main (int argc, char const *argv[]){
    menu();
    return 0;
}