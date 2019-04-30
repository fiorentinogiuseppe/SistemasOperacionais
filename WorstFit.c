#include<unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h> 

//gcc -w -pthread wf.c -o wf && ./wf

typedef struct candidatos
{
  int tam;
  int flag;                      
  void *ptr;           
}candidatos;

typedef struct memoryList
{
  // doubly-linked list
  struct memoryList *last;
  struct memoryList *next;

  unsigned tam;
  unsigned alloc;                      
  void *ptr;           
}memoryList;

unsigned tamMem;
void *myMemory = NULL;

static memoryList *head;
static memoryList *next;


memoryList* worst_block(unsigned req) {

	memoryList* current = head, *max = NULL;

	do {//aq eh lista circular current != head
		if(!(current->alloc) && (!max || current->tam > max->tam) ) {
			max = current;
		}
		current = current->next;
	}while(current != head); 
	if(max->tam >= req) {
		return max;
	} else {
		return NULL;
	}
}

void startUpMem(unsigned sz) {
  
	tamMem = sz;

	/* Initialize memory management structures */
	myMemory = malloc(sz);
	head = malloc(sizeof(memoryList));
	head->tam = sz;
	head->alloc = 0;
	head->ptr = myMemory;
	next = head;

	head->last = head;
	head->next = head;
}


void *alloc(unsigned req) {
	memoryList* block = worst_block(req);

	if(!block) {
		printf("ERROR SEM MEMORIA REAL...\n");
		return NULL;
	}

	if(block->tam > req) {
		// Concontainer for unallocated remainder of this block 
		memoryList* remainder = malloc(sizeof(memoryList));
    	
		// Insert into linked list 

		remainder->next = block->next;



		remainder->next->last = remainder;

		remainder->last = block;

		block->next = remainder;
    
		// Divide up allocated memory 
		remainder->tam = block->tam - req;
		remainder->alloc = 0;
		remainder->ptr = block->ptr + req;
		block->tam = req;
		next = remainder;
	} else {
		next = block->next;
	}
  
	block->alloc = 1;
  
	//Return pointer to the allocated block 
	return block->ptr;

}


void contiguous_block_prev( memoryList* current ){
	if(current != head && !(current->last->alloc)) {
		memoryList* prev = current->last;
		prev->next = current->next;
		prev->next->last = prev;
		prev->tam += current->tam;
    
		if(next == current) {
			next = prev;
		}
    
		free(current);
		current = prev;
	}
}


void contiguous_block_next( memoryList* current ){
	  if(current->next != head && !(current->next->alloc)) {
		struct memoryList* second = current->next;
		current->next = second->next;
		current->next->last = current;
		current->tam += second->tam;
    
    		if(next == second) {
			next = current;
		}
    
	    	free(second);
	}

}

/* Frees a block of memory previously allocated by alloc. */
void letfree(void* block) {
	/* Iterate over memory list, searching for the target block's container */
	printf("LIBERANDO BLOCO %p...\n",block);
	struct memoryList* current = head;
	do {
		printf("VERIFICANDO BLOCO %p...\n",block);
		if(current->ptr == block) {
			printf("ACHADO O BLOCO %p...\n",block);
			break;
		}
	} while((current = current->next) != head);

	/* Flag this block as freed */
	current->alloc = 0;

	contiguous_block_prev(current);
	contiguous_block_next(current);
}




int length() {
	int length = 0;
	memoryList *current = head;
	
	do{
		length++;
		current = current->next;
	} while(current != head); 
	
   return length;
}
void sort() {
   int i, j, k, tempAlloc, tempTam;
   int *tempPtr;
   memoryList *current;
   memoryList *next;
	
   int size = length();
   k = size ;	
	
   for ( i = 0 ; i < size - 1 ; i++, k-- ) {
      current = head;
      next = head->next;
		
      for ( j = 1 ; j < k ; j++ ) { 
         if ( current->alloc > next->alloc ) {
            tempAlloc = current->alloc;
            current->alloc = next->alloc;
            next->alloc = tempAlloc;

            tempTam = current->tam;
            current->tam = next->tam;
            next->tam = tempTam;
	    
	    tempPtr = current->ptr;
            current->ptr = next->ptr;
            next->ptr = tempPtr;

         }
			
         current = current->next;
         next = next->next;
	 
      }
   } 
	  
}

void compact(){
	memoryList *current = head;
	sort();
	do{
		contiguous_block_next(current);		
		current = current->next;
	} while(current != head); 

}


/* Use this function to print out the current contents of memory. */
void print_memory() {
  printf("Memory List {\n");
  /* Iterate over memory list */
  struct memoryList* index = head;
  do {
    printf("\tBlock %p,\tsize %d,\t%s\n",
           index->ptr,
           index->tam,
           (index->alloc ? "[ALLOCATED]" : "[FREE]"));
  } while((index = index->next) != head);
  printf("}\n");
}
int somatorio(int  arr[], int n){
	int sum=0;
	for (int i = 0; i < n; i++)
	 {
	    sum = sum + arr[i];
	 }
	return sum;
}


void mymem() {
	memoryList *a=NULL;
	memoryList *arr[5]; //array de ponteiros des elementos

	for (int i = 0; i < 5; i++) {
		arr[i] = a;
	}

	int pF, vF; //variaveis aleatorias para acessar o array
	int tam;
	int candidatos[] ={50,200,300,400,500,600};
	int flag[]={0,0,0,0,0,0};
	int numCandidatos=6;
	int cont=0; //contador
	int memRemain=0, memInit; //quanto de memoria ainda tem q usar usar 
	
	memRemain= somatorio(candidatos,numCandidatos);
	memInit=memRemain;
	tamMem=520;
	startUpMem(tamMem);
	while(memRemain>0){

		print_memory();
		tam=candidatos[cont];
		printf("CONT %i\n\n\n", cont);
		printf("TAMANHO %i\n\n\n",length());
		if(tam>tamMem){
			printf("ERROR TAMANHO %i MUITO GRANDE PARA A MEMORIA. NAO CONSEGUIRA SER ALOCADO\n",tam);
			candidatos[cont]=0;
		}
		else if(tam<=0)
			printf("ERROR VALOR %i INVALIDO ou 0\n",tam);
		else{
			if(tam>0){
				printf("TENTANDO ALOCAR %i...\n",tam);
				arr[cont] = alloc(tam);
				if(arr[cont]!=NULL){
					printf("ALOCADO %i...\n",tam);
					candidatos[cont]=0;
					flag[cont]=1;
					print_memory();
				}
			}
		}
		if(memRemain<memInit){
			pF=rand()%2;
			printf("PF = %i\n",pF);
			if(pF == 1){
				vF=rand() % numCandidatos;
				printf("VF = %i \n",vF);
				printf("Flag = %i\n", flag[vF]);
				printf("candidato: %i\n", candidatos[vF]);
				if(candidatos[vF]==0 && flag[vF]==1){
					printf("LIBERANDO %p...\n", arr[vF]);
					letfree(arr[vF]);
					compact();
					flag[vF]=0;
					print_memory();
				}
			}
		}
		memRemain=somatorio(candidatos,numCandidatos);
		cont++;
		if(cont>=numCandidatos)	cont=0;
		sleep(3);
	}
}

int main(){
	mymem();
	return 0;

}
