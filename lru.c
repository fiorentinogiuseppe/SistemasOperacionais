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
	printf("\nCaso tenha erro verificar aq\n");
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
  
	// A fila eh criada vazia
	queue->count = 0; 
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
  

/**Fim das funções utilitarias**/
