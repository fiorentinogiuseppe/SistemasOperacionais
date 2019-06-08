#include <stdio.h>
#include <stdlib.h>

#define MIN 0
#define MAX 199


int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
void printHead(){
	printf("    head start\n");
	printf("        |\n");
	printf("        V\n");
}
void scan(int *req, int req_tam, int head){
	int max, min;
	int headloc; //location of disk head in the arr
	req[req_tam] = head;
	req[req_tam+1]=MIN;
	req[req_tam+2]=MAX;
	req_tam+=3;

	//sort the array
	qsort(req, req_tam, sizeof(int), cmpfunc);
	
	//locate head in the req
	for(int i=0; i<req_tam; i++){
		if(head == req[i]){
			headloc = i;
			break;
		}
	}

	for(int j=headloc+1; j<req_tam; j++){
		if(req[j]==MAX){
       			printf("%d --> ",req[j]);
		}
		else if(j==headloc+1){
			printf("... --> %d --> ",req[j]);
		}
		else{
			printf("%d --> ",req[j]);
		}     
	}	
	for(int j=headloc; j>=0; j--){
		if(req[j]==MIN){
			printf("%d --> ...",req[j]);
		}		
		else {
			printf("%d --> ",req[j]);
        	}
	}

	int movimentos = (MAX-head)+(MAX-req[1]);
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

	scan(req, req_tam, head);
	
	return 0;
}
