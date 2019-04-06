#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
 
/*Compile the code by linking with -lpthread -lrt*/

//To define a semaphore object
sem_t mutex; 
int compart;//variavel compartilhada
  
void pth( int pID )
{ 
	//Proccess ID
	int var=pID;

	//Decrement or lock a semaphore 
	//To wait on a semaphore
	printf("  Thread %i  ... Entrando da Regiao Critica ... \n",pID);
	sem_wait(&mutex); 
  
	//critical section 
	printf("  Thread %i ... Regiao Critica ... \n",pID);
	compart=pID;
	printf("  Compart: %i\n",compart);	
	sleep(5); 
 
	//Increment or unlock a semaphore  
	printf("  Thread %i ... Saindo da Regiao Critica ... \n",pID);
    	sem_post(&mutex); 
} 
  
  
int main() 
{ 
	//initialize a private semaphore
	int pshared = 0; // This argument specifies whether or not the newly initialized semaphore is shared between processes or between threads. A 
			//  non-zero value means the semaphore is shared between processes and a value of zero means it is shared between threads.

	//   Specifies the value to assign to the newly initialized semaphore.
	int value = 1; 
	
	//To initialize a semaphore
	//returns 0 on success; on error, -1 is returned, and errno is set to indicate the error.
	int returno = sem_init(&mutex, pshared, value); 
	
	if(returno==0){
		
		pthread_t t1,t2,t3,t4,t5; 

		pthread_create(&t1,NULL,(void *) pth,(void *) 1 ); 
		sleep(2); 
		pthread_create(&t2,NULL,(void *) pth,(void *) 2 ); 
		sleep(2); 
		pthread_create(&t3,NULL,(void *) pth,(void *) 3 ); 
		sleep(2); 
		pthread_create(&t4,NULL,(void *) pth,(void *) 4 ); 
		sleep(2); 
		pthread_create(&t5,NULL,(void *) pth,(void *) 5 ); 
		
		//Suspends execution of the calling thread until the target thread terminates, unless the target thread has already terminated
		//Second argument - Pass in the address of a pointer-variable pointing to where the value passed to pthread_exit() should be written.
		pthread_join(t1,NULL); 
		pthread_join(t2,NULL); 
		pthread_join(t3,NULL); 
		pthread_join(t4,NULL); 
		pthread_join(t5,NULL); 

		//the prototype of pthread_join is:
		//int pthread_join(pthread_t tid, void **ret);
		//and the prototype of pthread_exit is:
		//void pthread_exit(void *ret);
	
		// In pthread_exit, ret is an input parameter. You are simply passing the address of a variable to the function.
		//In pthread_join, ret is an output parameter. You get back a value from the function. Such value can, for example, be set to NUL

		//To destroy a semaphore, 
		sem_destroy(&mutex); 
	
		return 0; 
	}
	else{
		printf("ERROR...\nTry Again Later!\n");
		return returno;
	}
} 	
