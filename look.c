#include <stdio.h>
#include <stdlib.h>

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
void printHead(){
	printf("    head start\n");
	printf("        |\n");
	printf("        V\n");
}
void look(int *req, int req_tam, int head){
	int max, min;
	int headloc; //location of disk head in the arr
	req[req_tam] = head;
	req_tam++;	

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

	//look algorithm
	for(int j=headloc; j>=0; j--){
		if(j==headloc){
			printf("... --> %d --> ",req[j]);
		}
		else{
			printf("%d --> ",req[j]);
		}     
	}		
	for(int j=headloc+1; j<req_tam; j++){
		if(j==req_tam-1){
			printf("%d --> ...",req[j]);
		}		
		else {
			printf("%d --> ",req[j]);
        	}
	}
	int movimentos = (max-head)+(max-min);
	printf("\nmovement of total cylinders %d\n", movimentos);

}


int main(){
	int req_tam=8;
	int req[req_tam];
	int head=53;

	req[0]=98;
	req[1]=183;
	req[2]=37;
	req[3]=122;
	req[4]=14;
	req[5]=124;
	req[6]=65;
	req[7]=67;

	look(req, req_tam, head);
	
	return 0;
}
