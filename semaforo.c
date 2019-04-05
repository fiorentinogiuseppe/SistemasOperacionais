#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
  
sem_t mutex; 
  
void pth( int pID )
{ 
    //wait 
printf("  Thread %i  ... Entrando da Regiao Critica ... \n",pID);
    sem_wait(&mutex); 

  
    //critical section 
printf("  Thread %i ... Regiao Critica ... \n",pID);
    sleep(4); 
 
    //signal 
printf("  Thread %i ... Saindo da Regiao Critica ... \n",pID);
    sem_post(&mutex); 

} 
  
  
int main() 
{ 
    sem_init(&mutex, 0, 1); 
    pthread_t t1,t2; 
    pthread_create(&t1,NULL,(void *) pth,(void *) 0 ); 
    sleep(2); 
    pthread_create(&t2,NULL,(void *) pth,(void *) 1 ); 
    pthread_join(t1,NULL); 
    pthread_join(t2,NULL); 
    sem_destroy(&mutex); 
    return 0; 
} 
