/* ************************************************** Includes */
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <time.h>



/* ************************************************** Defines */
#define TRUE 1
#define FALSE 0 
#define N 5


/* ************************************************** Global Varialbles */
int flag[N]; // interesse
int turn; //vez
int compart;//variavel compartilhada

/* ************************************************** Peterson Algorithm */

// Executed before entering critical section 
void enter_region (int process){ //0 ou 1
	//int other; //outro processo

	// give the other thread the chance to acquire lock 
	//other= 1-process; 

	/* Prepara-se para ENTRAR da Regiao Critica */
	flag[process] = TRUE; // Set TRUE saying you want to acquire lock 
	turn = 1-process; // troca o turn

	// Wait until the other thread looses the desire 
	// to acquire lock or it is your turn to get the lock. 
	while( flag[1-process]==TRUE & turn ==(1-process));
}

// Executed after leaving critical section 
void leave_region(int process){ // quem estiver saindo
	/* Prepara-se para SAIR da Regiao Critica */
	// You do not desire to acquire lock in future. 
	// This will allow the other thread to acquire 
	// the lock. 
	flag[process]=FALSE;
}

/* ************************************************** Thread */
void pth( int pID ){ //cria uma thread generica.
	int i, j, k;
	/* Prepara-se para ENTRAR da Regiao Critica */
	enter_region(pID);

	/* Processo dentro da Regiao Critica */
	printf("  Thread %i: ... Entrou na Regiao Critica ... \n",pID);

	// Critical section (Only one thread 
    	// can enter here at a time) 
	compart=pID;
	printf("  Compart: %i\n",compart);	
	sleep(10); 
	/* Prepara-se para SAIR da Regiao Critica */
	printf("  Thread %i: ... Saindo da Regiao Critica ... \n",pID);
	leave_region( pID);

}


/* ************************************************** Main Program */
int main( int argc, char* argv[] )
{
	pthread_t th0, th1, th2, th3,th4;
	void * r_th0;
	void * r_th1;
	void * r_th2;
	void * r_th3;
	void * r_th4;

/* ************************************************** Lock Init */
  // Initialize lock by reseting the desire of 
  // both the threads to acquire the locks. 
  // And, giving turn to one of them. 
	turn = 0;
 	flag[0] = flag[1] = flag[2] = flag[3] = flag[4] = FALSE;

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
