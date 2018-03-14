#include <stdio.h>
#include <stdlib.h>

struct dogType{
	int age;
	float weight;
	char gender;
	float height;
	char name[32];
	char animal_type[32];
	char race[16];
};

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


	}else if(option ==2){

	}else if(option ==3){

	}else if(option ==4){

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
