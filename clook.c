#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN 0
#define MAX 199

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
void printHead(){
	printf("    head start\n");
	printf("        |\n");
	printf("        V\n");
	printf("... --> ");
}
void printFoot(){
	printf("...\n");
}


void look(int *req, int req_tam, int head){
	int max, min;
	int headloc; //location of disk head in the arr
	req[req_tam] = head;
	req_tam++;	
	int lenArr=req_tam;

	//sort the array
	qsort(req, req_tam, sizeof(int), cmpfunc);
	min=req[0];
	max=req[req_tam-1];

	//locate head in the req
	for(int i=0; i<req_tam; i++){
		if(head == req[i]){
			headloc = i;
			break;
		}
	}

	//print that indicate the located head
	printHead();
	int j;

	for(j=headloc; j<req_tam && lenArr>0; j++){
		if(j==req_tam-1){
			printf("%d --> ",req[j]);
			j=-1; // como o j é incrementado dps de executar o corpo do loop eh so mandar ele pra antes do  0
		}		
		else {
			printf("%d --> ",req[j]);
        	}
		lenArr--;
	}
	printFoot();
	int lastPos=req[j-1];
	int movimentos = (max-head)+(max-min)+(lastPos-min);
	printf("\nmovement of total cylinders %d\n", movimentos);

}


int main(){
	int req_tam=8;
	int req[req_tam];
	int head=63;
	srand (time(NULL));
	for(int i=0; i<req_tam; i++){
		req[i]=rand() % MAX;

		printf("%i\n",req[i]);
	}

	look(req, req_tam, head);
	
	return 0;
}

