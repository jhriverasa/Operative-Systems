#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>

#define PORT 6112
#define MAX_CLIENTS 4


//# names in petnames.txt
#define N_PETNAMES 1716
//#indexes in hashtable
#define N_INDEXES 10000
//File to Write 10M structs
#define FILE_DATADOGS "dataDogs.dat"
//File with names
#define FILE_NAMES "petnames.txt"




//constants for server connection
#define READ_HEADER 1
#define READ_REGISTER 2
#define SHOW_MEDICAL_RECORD 3
#define CREATE_REGISTER 4
#define FIND_REGISTER 5
#define READ_FOUND_REGISTER 6
#define DELETE_REGISTER 7
#define EXIT_CLIENT 8


// Threads array
pthread_t thread[MAX_CLIENTS];
int pos;

//Mutex 
pthread_mutex_t lock;

//Semaphore
sem_t sema;


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


//Look for a free slot for a new client
int free_slot() {
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(thread[i] == (pthread_t)NULL)
            return i;
    return -1;
}



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



void menuSearchRegister(void *name, long index[50000]){
    struct fileHeader *header;
    header = malloc(sizeof(struct fileHeader));
    readHeader(header);

    
    //receive name and calculate its hash.
    char regName[32];
    strcpy(regName,name);
    unsigned long hashed;
    hashed = hash(regName,N_INDEXES);
    int counter;
    counter=0;

    if(header->head_pos[hashed] == 0 ){ //List is empty

        printf("%d%s",counter," registers found!.\n");
        index[49999]=0;
        
    
    }else{ //list has one or more elements

        //read every element in list and compare if their names are equal to asked name.
        struct dogType *currentReg;
        currentReg = malloc(sizeof(struct dogType));

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
        index[49999]=counter; //last index = total registers found
        
    }
}



void bye(){
    //Exit
    printf("%s","\n Good Bye.\n");
    exit(0);
}

//obtain ip from client
char* get_ip(int client_fd) {
    struct sockaddr_in addr;
    char* ip;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int r = getpeername(client_fd, (struct sockaddr *)&addr, &addr_size);
    return inet_ntoa(addr.sin_addr);
}


//(over)writes log
void logWrite(char* instruction, char* regChar, int client_fd) {
    FILE* logFile;
    logFile = fopen("serverDogs.log", "a");
    

    // time to string
    time_t timeNow = time(NULL);
    char time[80];
    struct tm *tmPtr;
    tmPtr = localtime(&timeNow);
    strftime(time, 80, "%Y-%m-%d | %H:%M:%S", tmPtr);
    // write and close log
    char* ip = get_ip(client_fd);
    char spacesBeforeIP[15-strlen(ip)];
    for(int i = 0; i < 15-strlen(ip); i++)
        spacesBeforeIP[i] = ' ';
    if(logFile != NULL){
        fprintf(logFile, "Date/time: %s | Client: %s%s\t| Instruction: %s | Parameter: %s\n", time, spacesBeforeIP, ip, instruction, regChar);
    }else{
        logFile = fopen("serverDogs.log", "a");
        fprintf(logFile, "Date/time: %s | Client: %s%s\t| Instruction: %s | Parameter: %s\n", time, spacesBeforeIP, ip, instruction, regChar);
    }

    fclose(logFile);
}

//Listen and execute the selected option (Option selected in client)
void *cliente(void *new_socketP){

    int new_socket=*(int*)new_socketP;
    int valread;
    int id = pos;
    int *buffer = malloc(sizeof(int));
    
    while( valread = recv( new_socket , buffer, sizeof(int),0) !=-1){
        printf("%d\n",*buffer );
        if(*buffer == READ_HEADER){
            printf("%s\n","READ HEADER DETECTED!" );
            struct fileHeader *tempHeader = malloc(sizeof(struct fileHeader));
            readHeader(tempHeader);
            send(new_socket, tempHeader , sizeof(struct fileHeader) , 0);
            free(tempHeader);

        }

        if(*buffer == READ_REGISTER){
            printf("%s\n","READ REGISTER DETECTED!" );

            struct dogType *tempReg = malloc(sizeof(struct dogType));
            long *tempPosition = malloc(sizeof(long));
            valread = recv(new_socket ,tempPosition ,sizeof(long),0 );  
            readRegister(*tempPosition,tempReg);    
            send(new_socket, tempReg , sizeof(struct dogType) , 0);


            char regCh[10];
            logWrite("See-Register",itoa(*tempPosition,regCh),new_socket);

            free(tempReg);
            free(tempPosition);

        }

        if(*buffer == SHOW_MEDICAL_RECORD){
            printf("%s\n","SHOW_MEDICAL_RECORD DETECTED!" );
            long *tempPosition = malloc(sizeof(long));
            valread = recv(new_socket ,tempPosition ,sizeof(long),0 );
            //showMedicalRecord(*tempPosition);


            free(tempPosition);

        }

        if(*buffer == CREATE_REGISTER){
            printf("%s\n","CREATE_REGISTER DETECTED!" );

            //--->CRITICAL SECTION<-----
            pthread_mutex_lock(&lock);
            //sem_wait(&sema);**

            struct dogType *tempReg = malloc(sizeof(struct dogType));
            valread = recv(new_socket ,tempReg ,sizeof(struct dogType),0 );  
            addRegister(tempReg);

            struct fileHeader *tempHeader = malloc(sizeof(struct fileHeader));
            readHeader(tempHeader);

            long tempPosition = sizeof(struct fileHeader) + (tempHeader->total_registers)*sizeof(struct dogType);
            char regCh[10];
            logWrite("Create-Register",itoa(tempPosition,regCh),new_socket);

            pthread_mutex_unlock(&lock);
            //sem_post(&sema);**
            //---->END CRITICAL SECTION<-----


            free(tempReg);
            free(tempHeader);

        }

        if(*buffer == FIND_REGISTER){
            printf("%s\n","FIND_REGISTER DETECTED!" );
            int *length=malloc(sizeof(int));
            valread = recv(new_socket , length ,sizeof(int),0 ); 
            char tempName[32];
            int i;
            for(i =0 ;i<32;i++){
                tempName[i]= '\0';
            }

            valread = recv(new_socket , tempName ,*length,0 ); 
            long positions[50000];
            menuSearchRegister(tempName, positions); 
            
            int *numregs=malloc(sizeof(int));
            *numregs = positions[49999];
            send(new_socket, numregs , sizeof(long) , 0);



            free(numregs);
            free(length);

        }

        if(*buffer == READ_FOUND_REGISTER){
            printf("%s\n","READ_FOUND_REGISTER DETECTED!" );
            int *length=malloc(sizeof(int));
            valread = recv(new_socket , length ,sizeof(int),0 ); 
            char tempName[32];
            valread = recv(new_socket , tempName ,*length,0 ); 
            long positions[50000];
            menuSearchRegister(tempName, positions); 
            
            int *regtoread=malloc(sizeof(int));
            valread = recv(new_socket, regtoread,sizeof(int),0);


            long lPos;
            lPos = positions[*regtoread-1];

            struct dogType *tempReg;
            tempReg= malloc(sizeof(struct dogType));
            readRegister(lPos,tempReg);

            logWrite("Search-register",tempName,new_socket);

            send(new_socket,tempReg, sizeof(struct dogType),0 );



            free(tempReg);

        }



        if(*buffer == DELETE_REGISTER){
            printf("%s\n","DELETE_REGISTER DETECTED!" );

             //--->CRITICAL SECTION<-----
            pthread_mutex_lock(&lock);
            //sem_wait(&sema);**

            struct dogType *tempReg = malloc(sizeof(struct dogType));
            long *tempPosition = malloc(sizeof(long));
            valread = recv(new_socket ,tempPosition ,sizeof(long),0 );  
            valread = recv(new_socket, tempReg, sizeof(struct dogType),0);
            deleteRegister(tempReg,*tempPosition);

            char regCh[10];
            logWrite("Delete-Register",itoa(*tempPosition,regCh),new_socket);


            pthread_mutex_unlock(&lock);
            //sem_post(&sema);**
            //---->END CRITICAL SECTION<-----

            free(tempReg);
            free(tempPosition);

        }


        if(*buffer == EXIT_CLIENT){
            printf("%s\n","EXIT_CLIENT DETECTED!" );

            //dummy long data
            long *tempPosition = malloc(sizeof(long));
            valread = recv(new_socket ,tempPosition ,sizeof(long),0 );

            close(new_socket);
            pthread_join(thread[id], NULL);
            thread[id] = (pthread_t) NULL;

            free(tempPosition);
        }
    }

}

int main(int argc, char const *argv[])
{   
    
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int *buffer = malloc(sizeof(int));
   
     
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
     
    // Forcefully attaching socket to the port 3535
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
     
    // Forcefully attaching socket to the port 
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //mutex added
    if(pthread_mutex_init(&lock, NULL) != 0){
        printf("\n mutex init failed \n");
        return 1;
    }

    //Sempaphore added
    if(sem_init(&sema, 0, 1) != 0){
        printf("\n semaphore init failed \n");
        return 1;
    }

    //endless bucle
    while(1==1){
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                           (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        //--------------------------
        do
            pos = free_slot();

        while (pos == -1);
        printf("Client connected: %d\n", pos+1);

        // Create new thread and listen new client
        int *new_socketP=malloc(sizeof(int));
        *new_socketP=new_socket;
        pthread_create(&thread[pos], NULL, (void *)cliente, new_socketP);
    
        //-----------------------------

        

    }

    return 0;
}
