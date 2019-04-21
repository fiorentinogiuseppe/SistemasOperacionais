//RESUMOS
//Least Recently Used (LRU) page replacement algorithm works on the concept that the pages that are heavily used in previous instructions are likely
//to be used heavily in next instructions. And the page that are used very less are likely to be used less in future. Whenever a page fault occurs,
//the page that is least recently used is removed from the memory frames. Page fault occurs when a referenced page in not found in the memory frames.

#include <stdio.h> 
#include <stdlib.h> 
  
/*************************/
// Struct noh da fila que tem sucessor e antecessor.
// O tamanho máximo da fila será o tamanho do cache.
// As páginas usadas mais recentemente estarão próximas da extremidade inicial.
// As páginas usadas menos recentemente estarão próximas da extremidade final.
typedef struct QNode 
{ 
	struct QNode *prev;
	struct QNode *next; 
	unsigned pageNumber; //Numero da pagina 
} QNode; 
  
// A propria fila pra auxiliar ao pegar o primeiro e ultimo e o numero total e totalaté agora além de hits e miss
//Ela eh diferente, pois as alterações sao feitas ao contrario. Quem sai eh o fim da fila e quem entra eh no inicio da fila, caso a fila esteja cheia
typedef struct Queue 
{ 
	unsigned count;  
	unsigned numberOfFrames;
	unsigned hit;
	unsigned miss;
	QNode *front, *rear; 
} Queue; 
  
//Structure que representa a tabelha hash que ira ser as paginas
//No hash o número da página sera a chave e endereço do nó da fila correspondente eh o valor da hash.
typedef struct Hash 
{ 
	int capacity; // Total de paginas
	QNode* *array; // Cada pagina tem um array que representa a posicao da memoria
} Hash; 
  
/** Funcoes utilitarias para criacao, remocao e outras utilidades de filas e hash**/
//Cria um novo no e retorna ele
QNode* newQNode( unsigned pageNumber ) 
{ 
	// Aloca memoria para criar o noh 
	QNode* temp = (QNode *)malloc( sizeof( QNode ) ); 
	temp->pageNumber = pageNumber; //numero da pagina
  
	// Initialize prev and next as NULL 
	//No criado sem estar anexado a ninguem
	temp->next = NULL;
	temp->prev = temp->next; 

  
	return temp; 
} 

//Cria a fila e retorna ela
Queue* createQueue( int numberOfFrames ) 
{ 
	//Aloca memoria para criar a fila
	Queue* queue = (Queue *)malloc( sizeof( Queue ) ); 
  
	// A fila eh criada vazia sem miss e hit
	queue->count = 0; 
	queue->hit = 0; 
	queue->miss = 0; 
	queue->front = queue->rear = NULL; 
  
	// Numero total de frames por paginas
	queue->numberOfFrames = numberOfFrames; 
  
    	return queue; 
} 

//Cria a hash e retorna ela
Hash* createHash( int capacity ) 
{ 
	// Aloca memoria para a hash
	Hash* hash = (Hash *) malloc( sizeof( Hash ) ); 
	//Tamanho total da hash ou seja o total de paginas
	hash->capacity = capacity; 
  
	// Crea o array que representa os fremas de cada pagina
	hash->array = (QNode **) malloc( hash->capacity * sizeof( QNode* ) ); 
  
	// Inicializa a hash vazia
	for( int i = 0; i < hash->capacity; ++i ) hash->array[i] = NULL;   
	return hash; 
} 

//Verifica se a fila esta vazia
int isQueueEmpty( Queue* queue ) 
{ 
	return queue->rear == NULL; 
} 

//Remove o utlimo elemento da fila
void deQueue( Queue* queue ) 
{ 
	//Um elemento só pode ser deletado quando houver pelo menos um elemento para deletar
	if( isQueueEmpty( queue ) ) return; //fila vazia n faz nada so retorna
  
	// Se ela eh o unico elemento remove o front dela e coloca null e isto faz a fila ficar vazia novamente sem front e rear
	if (queue->front == queue->rear) queue->front = NULL; 
  
	// Criar um noh temporario para salvar o ultimo elemento da fila
	QNode* temp = queue->rear; 
	//Retorna quem vem antes dele na fila e coloca no final, teoricamente eh feito um shift removendo o
	queue->rear = queue->rear->prev; 
  
	//Como removeu o "rabo" anterior tem q colocar um novo e tromover o ponteiro para o anterior
	if (queue->rear) 
	queue->rear->next = NULL; 
  	
	//liberar o no criado, ou seja matou o antigo rabo
	free( temp ); 
  
	// como tirou 1 da fila eh decrementado
	queue->count--; 
} 

// Verifica se o frame esta cheio para remover o rabo
int AreAllFramesFull( Queue* queue ) 
{ 
    return queue->count == queue->numberOfFrames; 
} 
  

// Adicionar um novo elemento a fila
void Enqueue( Queue* queue, Hash* hash, unsigned pageNumber ) 
{ 
	//Fila cheia
	// Fila cheia remove o "rabo"
	if ( AreAllFramesFull ( queue ) ) 
	{ 
		// remove page from hash 
		hash->array[ queue->rear->pageNumber ] = NULL; 
		deQueue( queue ); 
	} 
	  
	// Cria um novo no para adicionar na frente  
	QNode* temp = newQNode( pageNumber ); 
	temp->next = queue->front; 
   
	// Fila vazia
	//o rabo e o inicio ambos estao apontando para um mesmo local
	if ( isQueueEmpty( queue ) ){ 
		queue->front = temp; 
		queue->rear = queue->front;
	}
	else  // Caso a fila nao esteja vazia eh só adicionar no incio e teoricamente dar um shift em todos os restos
	{ 
		queue->front->prev = temp; 
		queue->front = temp; 
	} 
  
    // Adicionar a nova entrada ao hash
    hash->array[ pageNumber ] = temp; 
  
    // Adicionou mais um? Aumenta a conta
    queue->count++; 
} 

void printQueue(Queue* q) {
	struct QNode*ptr = q->front;
	//start from the beginning
	while(ptr != NULL) {
		printf("|%d|",ptr->pageNumber);
		ptr = ptr->next;
		if(ptr!= NULL) printf("<==");
	}
	printf("\n");
	
}
  

/**Fim das funções utilitarias**/

/**Funcoes para o LRU**/

// Se Frame não está lá na memória, trazemos na memória e adicionamos na frente da fila - miss
// O frame está lá na memória, nós movemos o frame para frente da fila - hit
// Sempre movos para o inicio da fila, assim temos que desvincular este no da sua posição para movermos para o inicio da fila

void ReferencePage( Queue* queue, Hash* hash, unsigned pageNumber ) 
{ 
	//Cria um noh para repersentar a pagina requisitada
	QNode* reqPage = hash->array[ pageNumber ]; 
  
	// Miss, pois a page nao esta e ele vai buscar	
	if ( reqPage == NULL ){ 
		//Teve um miss na queue
		queue->miss++;
		//Vai colocar na fila, mas usa os algoritmos para verificar que esta cheio ou nao, como já foi explicado
	        Enqueue( queue, hash, pageNumber );
	} 
  	//Hit
	else if (reqPage != queue->front) 
	{ 
		//Teve um hit na queue
		queue->hit++;

		//Desvincular a página solicitada de sua localização atual na fila.
		//Nesses dois comandos ele apenas tira da fila o noh requisitado
		reqPage->prev->next = reqPage->next; 
		if (reqPage->next) 
		reqPage->next->prev = reqPage->prev; 
  

		// Caso esse no seja o ultimo vai para o inicio
		if (reqPage == queue->rear) 
		{ 
			queue->rear = reqPage->prev; 
			queue->rear->next = NULL; 
		} 
  
		// Colocando o no no inicio do noh inicial 
		reqPage->next = queue->front; 
		reqPage->prev = NULL; 
	  
		// Faz com que o antigo primeiro agr aponte para o novo primeiro 
		reqPage->next->prev = reqPage; 
	  
		// modifica a front para a pagina requisitada
		queue->front = reqPage; 

    	} 
} 

/**Fim das funcoes para trabalhar com LRU**/
//When a page is referenced, the required page may be in the memory. If it is in the memory, we need to detach the node of the list and bring it to
//the front of the queue.
//If the required page is not in the memory, we bring that in memory. In simple words, we add a new node to the front of the queue and update the
//corresponding node address in the hash. If the queue is full, i.e. all the frames are full, we remove a node from the rear of queue, and add the new
//node to the front of queue.
  
/**Funcao main onde sera chamado tudo**/
int main() 
{ 


	// Cache com 4 pages 
	Queue* q = createQueue( 3 ); 
  
	// 10 paginas diferentes que podem ser referenciadas
	Hash* hash = createHash( 10 ); 
	

	// Sequencia de paginas 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5

	ReferencePage( q, hash, 1); 
	ReferencePage( q, hash, 2); 
	ReferencePage( q, hash, 3); 
	ReferencePage( q, hash, 4); 
	ReferencePage( q, hash, 1); 
	ReferencePage( q, hash, 2); 
	ReferencePage( q, hash, 5); 
	ReferencePage( q, hash, 1); 
	ReferencePage( q, hash, 2); 
	ReferencePage( q, hash, 3); 
	ReferencePage( q, hash, 4); 
	ReferencePage( q, hash, 5); 
	

	// So os prints 
	printQueue(q);
	printf("Total de hits : %d\n", q->hit);
	printf("Total de miss : %d\n", q->miss);
	
	return 0; 
} 
