#include<unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h> 

//gcc -w -pthread wf.c -o wf && ./wf


/*********Estruturas*********/

typedef struct candidatos
{
  int tam;
  int flag;                      
  void *ptr;           
}candidatos;


//Estrutura que representa um noh da lista circular duplamente ligada pra modelar a memoria
typedef struct memoryList
{	
	struct memoryList *last;
	struct memoryList *next;

	unsigned tam; //tamanho alocado
	unsigned alloc; //indica se esta alocado ou nao                      
	void *ptr; //ponteiro do proprio noh ele q indicara a ordem de colocacao        
}memoryList;

/*********Variaveis Globais*********/
unsigned tamMem;
void *myMemory = NULL;

//Ponteiros para o inicio e o free
static memoryList *head;
static memoryList *next;


/*********Funções que simulam o Gerenciamento de Memoria *********/

//Inicializa a memoria
void startUpMem(unsigned tam) {
  	
	//Variavel global é inicializada com o valor de inicio da memoria
	tamMem = tam;
	
	//Preenche os dados iniciais da estrutura, ou seja cria o head
	myMemory = malloc(tam);
	head = malloc(sizeof(memoryList));
	head->tam = tam;
	head->alloc = 0;
	head->ptr = myMemory;
	
	//Inicia o next
	next = head;
	//Inicia o head	
	head->last = head;
	head->next = head;
}


//Algoritmo que encontra o bloco de maior memoria
memoryList* worst_block(unsigned req) {

	//cria variavel que recebe a head = current
	//cria variavel que recebe o maior =  max
	memoryList* current = head, *max = NULL;

	//loop para andar na lista e verificar o maior bloco existente
	do {
		//Varifica se esta alocado se estiver ele ignora(uso da ! faz isso) 
		//Verifica é o maior
		if(!(current->alloc) && (!max || current->tam > max->tam) ) {
			max = current;
		}
		//proximo nó
		current = current->next;
	}while(current != head); 
	
	//Verifica se tem a memoria para aql tamanho requisitado
	if(max->tam >= req) {
		return max;
	} else {
		return NULL;
	}
}

//Adicionando valor na lista. Para esse momento chamamos de alocacao de memoria
void *alloc(unsigned req) {
	//Busca o bloco de maior tamanho
	memoryList* block = worst_block(req);
	
	//Se o maior bloco eh menor que a memoria requisitada entao	
	//ele indica que n tem memoria e retorna com null
	if(!block) {
		printf("NAO HA ESPACO PARA O TAMANHO REQUISITADO...\n");
		return NULL;
	}
	
	//Se o tamanho requisitado eh menor que o tamanho da maior memoria
	if(block->tam > req) {
		//O quanto resta de memoria
		//Quano a memoria inicia o head eh o maior e ele tem os ponteiros apontando para ele mesmo
		//Uma nova memoria vai pra frente
		//Criamos uma memoria para indicar o que resta
		//Pois o antigo bloco macio de memoria sera quebrado 
		//Novo bloco com tudo que sobrou do bloco block macico
		//Ficara entre o block e o block->next
		//Como eh lista circular nao da pra colocar no fim por isso eh colocado desta forma
		memoryList* resta = malloc(sizeof(memoryList));    		
		
		//Inicializa o que resta com os dados do block anterior livre
		//O block vai sempre pro fim do bloco macico
		//Int o rest fica entre o block e o que vinha depois do block
		resta->next = block->next;
		resta->next->last = resta;
		resta->last = block;
		block->next = resta;

		//Atualizacao de pesos. O bloco que resta tera este peso 
		resta->tam = block->tam - req;
		//Atualizacao de pesos. O bloco que resta esta livre
		resta->alloc = 0;
		//Resta ponteiro indicando q eh a memoria seguinte
		resta->ptr = block->ptr + req; 
		//O bloco anterior macico agr recebe este valor
		block->tam = req;

		//next recebe resta pois indica onde fia o free
		next = resta;
	} else {
		//Se o tamanho requisitado eh igual que o tamanho da maior memoria eh so indicar o bloco que vem antes dele no next
		next = block->next;
	}
	
  	//indica que o bloco esta alocado
	block->alloc = 1;
  
	//Returna o ponteiro da memoria alocada
	return block->ptr;

}

//Conta o tamanho da lista que representa a memoria
int length() {
	int length = 0;
	memoryList *current = head;
	
	do{
		length++;
		current = current->next;
	} while(current != head); 
	
   return length;
}

//Ordena baseado na alocacao. Memoria livre vai pro final da lista
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
         if ( current->alloc < next->alloc ) {
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

//realiza compactacao
//Ordena primeiro e depois junta td mundo que tem memoria livre em uma unica memoria
void compact(){
	//usado para andar na lista
	memoryList *current = head;
	sort();
	do{
		//So entra se nao for a cabeca e se o anterior nao esta alocado
		//Anda ate o ultimo alocado quando chegar nele entra no if sempre ate chegar na cabeca novamente
		if(current != head && !(current->last->alloc)) {
			//cria um novo noh com a memoria livre anterior da memoria atual que eh a ultima que foi alocada
			memoryList* prev = current->last;
						
			prev->next = current->next;
			prev->next->last = prev;
			//Vai incrementando a memoria
			prev->tam += current->tam;
    	
			if(next == current) {
				next = prev;
			}
    
			free(current);
			current = prev;
		}
		current = current->next;
	} while(current != head); 	
	//Quando chegar na cabeca tera apenas uma memoria com o tamanho que eh a soma de todas as anteriores

}

//Implementacao da liberacao de memoria
void letfree(void* block) {
	memoryList* current = head;
	//vai andando na lista ate achar o bloco 
	do {
		printf("	VERIFICANDO BLOCO %p...\n",block);
		if(current->ptr == block) {
			printf("	ACHADO O BLOCO %p\n",block);
			break;//forca saida do loop, pois achou a memoria
		}
	} while((current = current->next) != head);

	//Indica que foi liberado o bloco requisitado para ser liberado
	current->alloc = 0;
	
	//Realiza compactacao
	compact();
}



//print da lista
void print_memory() {
	printf("Memory List {\n");

	memoryList* index = head;
	do {
		printf("\tBlock %p,\tsize %d,\t%s\n",
		index->ptr,
		index->tam,
		(index->alloc ? "[ALLOCATED]" : "[FREE]"));
	} while((index = index->next) != head);
	printf("}\n");
}

//Realiza somatorio da lista de memoria
int somatorio(int  arr[], int n){
	int sum=0;
	for (int i = 0; i < n; i++)
	 {
	    sum = sum + arr[i];
	 }
	return sum;
}

//Funcao que remove o elemento da lista que eh muito grande
candidatos *removeGrande(candidatos cnddRemovivel[], int len, candidatos *lista[]){
	int n=len;
	candidatos *lista2 [n];
	int i=0;
	int i2=0;
	while(i<n){
		if(lista[i]->tam!=cnddRemovivel->tam){
			if(i<n+1){
			 lista2[i2]=lista[i];
			}
		}
		else{	
			lista2[i2]=lista[i+1];
			i++;
		}
		i++;
		i2++;
	}
	//Realiza a copia da nova lista para a lista antiga	
	memcpy(lista,lista2,sizeof(lista2));
}

//Principal funcao para trabalhar a memoria
void mymem() {
	/***CRIA CANDIDATOS***/
	int cnddt[] = {600,200,300,400,500,50};
	/****INICIA CANDIDATOS****/
	int len = sizeof(cnddt)/sizeof(cnddt[0]);
	candidatos * a = NULL;
	candidatos *lista [len];
	for(int i=0; i< len; i++){
		a = malloc(sizeof(candidatos));
		a->tam = cnddt[i];
		a->ptr=NULL;
		a->flag=0;
		lista[i]=a;
	}

	/***VARIAVEIS***/	

	int pF, vF; //variaveis aleatorias para acessar o array
	int cont=0; //contador
	int memRemain=0, memInit; //quanto de memoria ainda tem q usar usar 
	int tam;
	int remo=0;
	
	memRemain= somatorio(cnddt,len);
	memInit=memRemain;
	tamMem=520;

	/***INICIANDO MEMORIA***/
	startUpMem(tamMem);


	//Firaca rodando ate que o tamanho da memoria que for requisitada seja 0
	//Toda vez que a memora for alocada sera serado seu tamanho e a flag setado para 1
	//Toda vez que for liberado sera feito compactacao e flag setada para 0
	//Sera feito um somatorio a cada vez no loop pra saber se todas as memorias ja foram usadas
	//Caso uma memoria seja muito grande e nao couber nem na memoria totalmente livre ela sera setada para zero
	//Memoria zeradas fazem nada apenas um contador de 2;
	//Memorias negativas o sistema da erro e sai;
	/***RUNNING***/
	while(memRemain>0){
		//Cada inicio de loop eh printado a memoria
		print_memory();
		//Pra ajudar na hora de escrever coloquei o tamanho para a variavel tam
		tam=lista[cont]->tam;
		
		/** Memorias Invalidas**/
		//Memoria maior que a do sistema é zerado e não faz mais nada
		if(tam>tamMem){
			printf("ERROR TAMANHO %i MUITO GRANDE PARA A MEMORIA. NAO CONSEGUIRA SER ALOCADO\nREMOVENDO...\n",tam);
			removeGrande(lista[cont],len, &lista);
			remo=1;
			len--;
		}
		//Memorias zeradas não faz nada apenas um sleep
		else if(tam==0){
			sleep(2);
		}
		//Memorias negativas dao erro e sai do sistema
		else if(tam<0)
			return -1;
		/***Memorias validas***/
		else{
			//So pra garantir que so entrara valores maiores que zero
			if(tam>0){
				printf("\nTENTANDO ALOCAR %i...\n",tam);
				//Vai ser tentado alocar
				lista[cont]->ptr = alloc(tam);
				//Sem memoria disponivel retorna null e teoricamente ele espera ate entrar
				if(lista[cont]->ptr!=NULL){
					printf("	ALOCADO %i...\n",tam);
					lista[cont]->tam=0;
					lista[cont]->flag=1;
				}
			}
		}
		//O somatorio tem que ser menor que o inicio, isso foi criado so pra controlar e no inicio ja nao remover
		if(memRemain<memInit){
			//Gera um aleatorio para dizer se vai remover ou nao
			pF=rand()%2;
			if(pF == 1){
				//Gera um aleatorio apra dizer qual sera removido
				vF=(rand() % len);
				//Verifica se o valor a ser removido ja foi alocado e se ainda esta alocao
				if(lista[vF]->tam==0 && lista[vF]->flag==1){
					printf("\nLIBERANDO %p...\n", lista[vF]->ptr);
					letfree(lista[vF]->ptr);
					compact();
					//Indica que ja foi removido
					lista[vF]->flag=0;
				}
			}
		}
		//Realiza o somatorio novamente
		memRemain=somatorio(cnddt,len);
		//Caso um valor tenha memoria muito grande e foi removido  ele segura o cont 1x pra continuar contando
		if(remo==0) cont++;		
		else remo=0;
		//Como conto atraves da lista se o valor do contador passou o tamanho da lista ele reinicia pra procurar novamente
		if(cont>=len)	cont=0;
		//sleep so pra ajudar a visualizar 
		sleep(3);
	}
}


/*********Funções Main*********/
int main(){
	
	mymem();
	return 0;

}
