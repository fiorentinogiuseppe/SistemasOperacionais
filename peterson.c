
/* ************************************************** Includes */
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <time.h>
#include<stdbool.h>
// RODAR SINGLE CORE taskset -c 0 ./peterson


/* ************************************************** Defines */
#define TRUE 1
#define FALSE -1 
#define N 5


/* ************************************************** Global Varialbles */
int flag[N]; // interesse
int turn[N]; //vez
int compart;//variavel compartilhada

/* ************************************************** Peterson Algorithm */
//https://stackoverflow.com/questions/26701942/trying-to-understand-the-petersons-n-process-algorithm
//https://www.cs.rice.edu/~vs3/comp422/lecture-notes/comp422-lec19-s08-v1.pdf

// Executed before entering critical section 
// that position 0 is the tail (back) of the queue, position N-1 is the head (front) of the queue, position N-2 is second from the front, etc... , and position -1 means you're not in the queue at all 

//Filter lock: direct generalization of Peterson’s lock

bool sameOrHigher(int i, int j) { 
	for(int k = 0; k < N; k++) 
		if (k != i && flag[k] >= j) 
			return true; 
	return false;
}
void enter_region (int process){
	printf("  Thread %i: ... Entrando na Regiao Critica ... \n",process);
	
	for (int count = 1; count < N; count++) {
		flag[process] = count;                 // I think I'm in position "count" in the queue
		turn[count] = process;                  // and I'm the most recent process to think I'm in position "count"
		while (sameOrHigher(process,count) && turn[count] == process);
		
                                      // now I can update my estimated position to "count"+1 

	}                                    // now I'm at the head of the queue so I can start my critical section          
	sleep(2);
}



// Executed after leaving critical section 
void leave_region(int process){ // quem estiver saindo
	
	printf("  Thread %i: ... Saindo da Regiao Critica ... \n",process);
	flag[process]=FALSE;

}

/* ************************************************** Thread */
void pth( int pID ){ //cria uma thread generica.
	int i, j, k;
	/* Prepara-se para ENTRAR da Regiao Critica */
	//printf("  Thread %i: ... Entrando na Regiao Critica ... \n",pID);
	enter_region(pID);

	/* Processo dentro da Regiao Critica */
	printf("  Thread %i: ... Entrou na Regiao Critica ... \n",pID);

	// Critical section (Only one thread 
    	// can enter here at a time) 
	compart=pID;
	printf("  Compart: %i\n",compart);
	
	/* Prepara-se para SAIR da Regiao Critica */
	leave_region( pID);
	printf("  Thread %i: ... Saiu da Regiao Critica ... \n",pID);
}


/* ************************************************** Main Program */
int main( int argc, char* argv[] )
{
	pthread_t th0, th1, th2, th3,th4;

/* ************************************************** Lock Init */
  // Initialize lock by reseting the desire of 
  // both the threads to acquire the locks. 
  // And, giving turn to one of them. 

	for(int i=0; i<N;i++) {
		flag[0] = FALSE;
		turn[0] = FALSE;
	}

/* ************************************************** 5 Proccess */
	printf("Thread \"Main\": Algoritmo de \"Peterson\" \n");
	  
	if( pthread_create( &th0, NULL, (void *) pth,(void *) 0 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 0.\n");
	    exit(1);
	}

	if( pthread_create( &th1, NULL, (void *) pth,(void *) 1 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 1.\n");
	    exit(1);
	}
	if( pthread_create( &th2, NULL, (void *) pth,(void *) 2 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 2.\n");
	    exit(1);
	}

	if( pthread_create( &th3, NULL, (void *) pth,(void *) 3 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 3.\n");
	    exit(1);
	}

	if( pthread_create( &th4, NULL, (void *) pth,(void *) 4 ) != 0 ) {
	    printf("Error \"pthread_create\" p/ Thread 4.\n");
	    exit(1);
	}

/* ************************************************** Wait the end */
	printf("Thread \"Main\": Sincroniza termino com Threads 0 a 4.\n");


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

	printf("Thread \"Main\": Termina.\n");
	exit(0);
}
