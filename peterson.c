
/*http://www.facom.ufu.br/~faina/MCC_Crs/MC104-1S2008/Prjt_SO/thread-peterson.c
https://timmurphy.org/2010/05/04/pthreads-in-c-a-minimal-working-example/
*/

/* ************************************************** Includes */
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>

/* ************************************************** Defines */
#define TRUE 1
#define FALSE 0 
#define N 2 //numero de processos


/* ************************************************** Global Varialbles */
int flag[N]; // interesse
int turn; //vez
int tNumber; //Numero da thread

/* ************************************************** Peterson Algorithm */

void enter_region (int process){ //0 ou 1
	int other; //outro processo

	other= 1-process; //oposto do processo
	/* Prepara-se para ENTRAR da Regiao Critica */
	flag[process] = TRUE; //Interesse
	turn = process; // troca o turn

	while( flag[other]==TRUE & turn == process ) {
		printf("  Thread %i: ... Ocupado, espere ... \n",process);
	}
}

void leave_region(int process){ // quem estiver saindo
	/* Prepara-se para SAIR da Regiao Critica */
	flag[process]=FALSE; //saindo da regian critica
}

/* ************************************************** Thread */
void pth( int pID ) //cria uma thread generica. Só precisa entrar com o numero da thread. 0 ou 1, por enquando tepois vai ate 5
{
  int i, j, k;
  
  for( i=0; i<N; i++ ) {
    /* Prepara-se para ENTRAR da Regiao Critica */

    enter_region ( pID);

    /* Processo dentro da Regiao Critica */
    printf("  Thread %i: ... Entrou na Regiao Critica ... \n",pID);
    /* Prepara-se para SAIR da Regiao Critica */
    leave_region( pID);
    printf("  Thread %i: ... Saiu da Regiao Critica ... \n",pID);

    /* Simula um Processamento qualquer */
    for( j=0; j<16384; j++ )
      for( k=0; k<8192; k++) ;
  }
}


/* ************************************************** Main Program */
int main( int argc, char* argv[] )
{
  pthread_t th0, th1;
  void * r_th0;
  void * r_th1;
  turn = 0;
  
  flag[0] = flag[1] = FALSE;

  printf("Thread \"Main\": Algoritmo de \"Peterson\" \n");
  
  if( pthread_create( &th0, NULL, (void *) pth,(void *) 0 ) != 0 ) {
    printf("Error \"pthread_create\" p/ Thread 0.\n");
    exit(1);
  }


  if( pthread_create( &th1, NULL, (void *) pth,(void *) 1 ) != 0 ) {
    printf("Error \"pthread_create\" p/ Thread 1.\n");
    exit(1);
  }

  /* Sincroniza o termino da Thread "Main" com as Threads "th0" e "th1" */

  printf("Thread \"Main\": Sincroniza termino com Threads 0 e 1.\n");


 if(pthread_join( th0, (void *) &r_th0 )) {

 	fprintf(stderr, "Error joining thread\n");
	return 2;

 }

 if(pthread_join( th1, (void *) &r_th1 )) {

 	fprintf(stderr, "Error joining thread\n");
	return 2;

 }

  printf("Thread \"Main\": Termina.\n");
  exit(0);

}
