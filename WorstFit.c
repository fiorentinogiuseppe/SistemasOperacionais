//Resumo

//The Worst Fit Memory alloc Algorithm
//allocates the largest free partition avai-
//lable in the memory that is sufficient
//enough to hold the process within the
//system.

//It searches the complete memory for
//available free partitions and allocates
//the process to the memory partition
//which is the largest out of all.

//This algorithm is not recommended to be
//implemented in the real world as it has
//many disadvantages.

//A process entering first may be allocated
//the largest memory space but if another
//process of larger memory requirement is
//to be allocated, space cannot be found.
//This is a serious drawback here.

//===============================================

//Include
#include<stdio.h>

void worstFit(int* tamBlock, int nblock, int* tamProc, int nArq ){
	//Onde sera guardado os Ids dos processos
	int alloc[nArq];

	//Inicializando com -1, pois não há o mapeamento bloco x processo 
	for(int i =0 ; i< nArq; i++) alloc[i]=-1;

	//prints antes para mostrar
	printf(">>>>>>>>>>ANTES<<<<<<<<<<\n");
	printf("\nProcess No.\tProcess Size\tBlock no.\tBlock Size\n");
	for(int i=0; i<nblock; i++){
		if(i+1>nArq) printf("  %d \t\t %d \t\t  %d \t\t  %d \t\t \n",-1,-1,-1,tamBlock[i]);
		else printf("  %d \t\t %d \t\t  %d \t\t  %d \t\t \n",i+1,tamProc[i],-1,tamBlock[i]);
			

	}
	
	//Rodando o Algoritmo
	printf("\n\n>>>>>>>>>>RODANDO<<<<<<<<<<\n\n");
	//Pega cada professo e encontra um bloco para ele
	//De acordo com o seu tamanho
        for(int i=0; i<nArq;i++){
		printf("\n\nProcess No.  %d \t\t \n", i+1);
		printf("Encontrando melhor bloco...\n");
		//O index do melhor bloco do processo atual
		int wstId = -1;
                for(int j=0; j<nblock; j++){
			printf("Block no.  %d \t\t \n",j);
			if(tamBlock[j] >= tamProc[i] ){
				printf("Process No. %d = %d cabe no Block No. %d = %d\n",i,tamProc[i],j,tamBlock[j] );
				//Se o index for -1 não tem ninguem é so alterar
				if(wstId == -1) {
					printf("Block No. %d = %d Vazio... Alocando Process No. %d = %d... \n",j,tamBlock[j],i,tamProc[i]);
					wstId = j;
				}
				//Se o index for diferente de -1 tem alguem lá caso o tamanho seja menor aloca lá caso contrario não é possivel alocar
				else if (tamBlock[wstId] < tamBlock[j]){
					printf("Bloco No. %d = %d alocado, porem possui espaco para o Process No. %d = %d...\n",j,tamBlock[j],i,tamProc[i]);
					wstId = j;
				}
				else
					printf("Bloco No. %d = %d já alocado e nao possui espaco para o Process No. %d = %d...\n",j,tamBlock[j],i,tamProc[i]);
                        }
			else{
				printf("Process No. %d = %d nao cabe no Block No. %d = %d\n",i,tamProc[i],j,tamBlock[j] );
			}
                }

		//Se foi encontrado a localização
		if(wstId != -1){
			//Alocamos o bloco j para o processo p[i]
			alloc[i] = wstId;
			
			//Diminui o tamanho  dql bloco
			tamBlock[wstId] -= tamProc[i];
		}
        }
	printf("\n\n>>>>>>>>>>FIM<<<<<<<<<<\n\n");

	//prints para mostrar
	printf("\n\n>>>>>>>>>>DEPOIS<<<<<<<<<<\n\n");
	printf("\nProcess No.\tProcess Size\tBlock no.\tBlock Size\n");
	for(int i=0; i<nArq; i++){
		printf("  %d \t\t %d \t\t",i+1,tamProc[i]);
		if(alloc[i] != -1)
			printf("%i \t\t ", alloc[i]+1);
		else
			printf("-1 \t\t ");
		printf("  %d \t\t \n",tamBlock[i]);

	}


}

//alterar o nome nblock e nArq
int main(void){
	int blockSize[] = {100, 500, 200, 300, 600}; 
	int processSize[] = {212, 417, 112, 426}; 
	int nblock = sizeof(blockSize)/sizeof(blockSize[0]); 
	int nArq = sizeof(processSize)/sizeof(processSize[0]); 
  
	worstFit(blockSize, nblock, processSize, nArq);
	return 0;
}
		
