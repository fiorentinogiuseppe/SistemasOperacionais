
/* ************************************************** Includes */
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <time.h>

// RODAR SINGLE CORE taskset -c 0 ./peterson


/* ************************************************** Defines */
#define TRUE 1
#define FALSE 0 
#define N 5


/* ************************************************** Global Varialbles */
int level[N]; // interesse
int victim[N]; //vez
int compart;//variavel compartilhada

/* ************************************************** Peterson Algorithm */
//http://pages.cpsc.ucalgary.ca/~higham/Teaching/distributedComputingPapers/herlihy_ch2_ME.pdf
// 3.4 The Filter Lock pag 10
//Maurice Herlihy and Nir Shavit. “Multiprocessor
//Synchronization and Concurrent Data Structures.” Chapter 3
//“Mutual Exclusion.” Draft manuscript, 2005.

//Verifica se tem thread no msm nivel ou maior
int maior(int a, int b) { 
	for(int i = 0; i < N; i++) 
		if (i != a && level[i] >= b) 
			return TRUE; 
	return FALSE;
}

//Filter lock: direct generalization of Peterson’s lock
void enter_region (int process){
	printf("  Thread %i: ... Entrando na Regiao Critica ... \n",process);
	//Percorrer todos os processos
	for (int count = 1; count < N; count++) {
		level[process] = count;                 
		victim[count] = process;      
		//aguarda conflitos            
		while (maior(process,count) && victim[count] == process);           
	}                                  
	sleep(2);
}



// Executed after leaving critical section 
void leave_region(int process){ // quem estiver saindo
	
	printf("  Thread %i: ... Saindo da Regiao Critica ... \n",process);
	level[process]=FALSE;

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
  // And, giving victim to one of them. 

	for(int i=0; i<N;i++) {
		level[0] = FALSE;
		victim[0] = FALSE;
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
