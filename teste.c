#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h> 

//gcc -w wf.c -o wf && ./wf

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
		printf("ERROR SEM MEMORIA...\n");
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

	memoryList* current = head, *max = NULL;
	current = current->next;
	do {//aq eh lista circular current != head
		if(!(current->alloc) && (!max || current->tam > max->tam) ) {
			max = current;
		}
		current = current->next;
	} while(current != head); 

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

void mymem() {
  void *a, *b, *c, *d, *e;
	tamMem=520;
  startUpMem(tamMem);
	print_memory();
  a = alloc(100);
	print_memory();
  b = alloc(200);
	print_memory();
  c = alloc(300);
	print_memory();
	printf("Livrando..\n");
  letfree(b);
	print_memory();
	compact();
	print_memory();
  d = alloc(700);
	print_memory();
	printf("Livrando..\n");
  letfree(a);
	print_memory();
	compact();
	print_memory();
  e = alloc(800);
	print_memory();
	
}
/*
void pth( int pID ){ //cria uma thread generica.
	void *a;
	a=alloc(pID);
	printf("MEMORIA %i\n",pID);
	print_memory();
	sleep(5); 
	printf("FREE %i\n",pID);
	letfree(a);
	printf("COMPACT %i\n",pID);
	compact();
	print_memory();
	printf("FIM %i\n",pID);
}

void listProcess(){
	pthread_t th0, th1, th2, th3,th5,th6,th7,th8,th9,th4;

	printf("Thread \"Main\n");
	  
	if( pthread_create( &th0, NULL, (void *) pth,(void *) 0 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 0.\n");
	    exit(1);
	}

	if( pthread_create( &th1, NULL, (void *) pth,(void *) 10 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 1.\n");
	    exit(1);
	}
	if( pthread_create( &th2, NULL, (void *) pth,(void *) 200 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 2.\n");
	    exit(1);
	}

	if( pthread_create( &th3, NULL, (void *) pth,(void *) 30 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 3.\n");
	    exit(1);
	}

	if( pthread_create( &th4, NULL, (void *) pth,(void *) 400 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 4.\n");
	    exit(1);
	}
	if( pthread_create( &th5, NULL, (void *) pth,(void *) 4 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 5.\n");
	    exit(1);
	}
	if( pthread_create( &th6, NULL, (void *) pth,(void *) 40 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 6.\n");
	    exit(1);
	}
	if( pthread_create( &th7, NULL, (void *) pth,(void *) 100 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 7.\n");
	    exit(1);
	}
	if( pthread_create( &th8, NULL, (void *) pth,(void *) 800 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 8.\n");
	    exit(1);
	}
	if( pthread_create( &th9, NULL, (void *) pth,(void *) 4 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 9.\n");
	    exit(1);
	}


	printf("Thread \"Main\": Sincroniza termino com Threads 0 a 9.\n");


	if(pthread_join( th0, NULL)) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}

 	if(pthread_join( th1, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}

	if(pthread_join( th2, NULL)) {

		fprintf(stderr, "Error joining thread\n");
		return 2;
	}

	if(pthread_join( th3, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}

	if(pthread_join( th4, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}
	if(pthread_join( th5, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}
	if(pthread_join( th6, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}
	if(pthread_join( th7, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}
	if(pthread_join( th8, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}
	if(pthread_join( th9, NULL )) {

	 	fprintf(stderr, "Error joining thread\n");
		return 2;
	}
	printf("Thread \"Main\": Termina.\n");
	exit(0);

}
*/
int main(){
	//startUpMem(520);
	//listProcess();
	mymem();
	return 0;

}
