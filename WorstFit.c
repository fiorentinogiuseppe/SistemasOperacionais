#include<unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h> 

//gcc -w -pthread wf.c -o wf && ./wf


/*********Estruturas*********/

//Estrutura que representa um noh da lista circular duplamente ligada pra modelar a memoria
typedef struct memoryList
{	
	struct memoryList *last;
	struct memoryList *next;

	unsigned tam; //tamanho alocado
	unsigned alloc; //indica se esta alocado ou nao                      
	void *ptr; //ponteiro do proprio noh           
}memoryList;

/*********Variaveis Globais*********/
unsigned tamMem;
void *myMemory = NULL;

//Ponteiros para o inicio e o proximo dps do free
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
		memoryList* resta = malloc(sizeof(memoryList));    		
		
		resta->next = block->next;
		//Atualiza o dado do bloco que vem antes do free para apontar para esse resta bloco
		resta->next->last = resta;
		//o bloco free fica apos o resta bloco
		resta->last = block;
		//o antigo next do |free| eh atualizado pra ficar apos o resta bloco
		block->next = resta;

		// 
		resta->tam = block->tam - req;
		printf("\n\n\n\n\n\nresta->TAM = %i\n\n\n\n\n\n",resta->tam);
		//
		resta->alloc = 0;
		//resta ponteiro indicando q eh a memoria seguinte
		resta->ptr = block->ptr + req; 
		//
		block->tam = req;

		//next recebe esse resta valor,  pois o bloco ainda ta no fim
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
	printf("\n\n\nLIBERANDO BLOCO %p...\n",block);
	memoryList* current = head;
	//vai andando na lista ate achar o bloco 
	do {
		printf("VERIFICANDO BLOCO DE TAMANHO %I...\n",block->tam);
		if(current->ptr == block) {
			printf("ACHADO O BLOCO %p DE TAMANHO...\n",block, block->tam);
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


//Principal funcao para trabalhar a memoria
void mymem() {
	memoryList *a=NULL;
	memoryList *arr[5]; //array de ponteiros des elementos

	for (int i = 0; i < 5; i++) {
		arr[i] = a;
	}

	int pF, vF; //variaveis aleatorias para acessar o array
	int tam;
	int candidatos[] ={50,200,300,400,500,600};
	int flag[]={0,0,0,0,0,0};
	int numCandidatos=6;
	int cont=0; //contador
	int memRemain=0, memInit; //quanto de memoria ainda tem q usar usar 
	
	memRemain= somatorio(candidatos,numCandidatos);
	memInit=memRemain;
	tamMem=520;
	startUpMem(tamMem);
	while(memRemain>0){

		print_memory();
		tam=candidatos[cont];
		printf("CONT %i\n\n\n", cont);
		printf("TAMANHO %i\n\n\n",length());
		if(tam>tamMem){
			printf("ERROR TAMANHO %i MUITO GRANDE PARA A MEMORIA. NAO CONSEGUIRA SER ALOCADO\n",tam);
			candidatos[cont]=0;
		}
		else if(tam<=0)
			printf("ERROR VALOR %i INVALIDO ou 0\n",tam);
		else{
			if(tam>0){
				printf("TENTANDO ALOCAR %i...\n",tam);
				arr[cont] = alloc(tam);
				if(arr[cont]!=NULL){
					printf("ALOCADO %i...\n",tam);
					candidatos[cont]=0;
					flag[cont]=1;
					print_memory();
				}
			}
		}
		if(memRemain<memInit){
			pF=rand()%2;
			printf("PF = %i\n",pF);
			if(pF == 1){
				vF=rand() % numCandidatos;
				printf("VF = %i \n",vF);
				printf("Flag = %i\n", flag[vF]);
				printf("candidato: %i\n", candidatos[vF]);
				if(candidatos[vF]==0 && flag[vF]==1){
					printf("LIBERANDO %p...\n", arr[vF]);
					letfree(arr[vF]);
					compact();
					flag[vF]=0;
					print_memory();
				}
			}
		}
		memRemain=somatorio(candidatos,numCandidatos);
		cont++;
		if(cont>=numCandidatos)	cont=0;
		sleep(3);
	}
}


/*********Funções Main*********/
int main(){
	mymem();
	return 0;

}
