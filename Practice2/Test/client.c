
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
        sleep(2);
        showMedicalRecord(position);

        free(reg);
        


    }
    
    free(header);
    //goback();

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
        //createRegister();
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