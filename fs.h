#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
/***********INODE***********/

//Valores default
#define EXT2_NAME_LENGTH 15
#define EXT2_PTR_LENGTH 8
#define EXT2_PAD_LENGTH 10
#define BLOCKS 4611
#define BLOCK_SIZE 1024
#define INODE_SIZE 64
#define DATA_BEGIN_BLOCK 515
#define DIR_SIZE 32
#define DIRECT_INDEX_LIMIT 3072
#define DIRECT_INDEX_BLOCKS 6
#define ONE_INDEX_LIMIT 131072
#define ONE_INDEX_BLOCKS 256
#define INDEX_LIMIT 2097152 // 4096*512
#define INDEX_BLOCKS 4096
#define PATH "DiscoVirtual"
#define COMMAND_SIZE 9

//Padroes de info
#define FORMAT 0
#define LS 2
#define CREATE 3
#define DELETE 4
#define CD 5
#define READ 6
#define WRITE 7
#define EXIT 8

/*
 * Special numbers
 */
#define FILE_TYPE_FILE 1
#define FILE_TYPE_DIR 2

/*
	Algumas definições
	http://www.science.unitn.it/~fiorella/guidelinux/tlk/node95.html
*/


//Physical Layout of the EXT2 File system
//bg de block group
//Bitmap = Keeps track of which blocks are free/used
typedef struct group_descriptor
{
    char bg_volume_name[EXT2_NAME_LENGTH + 1];          /* Nome do "volume"*/
    uint16_t bg_block_bitmap;        			/* Blocks bitmap block */ 
    uint16_t bg_inode_bitmap;        			/* Inodes bitmap block */ 
    uint16_t bg_inode_table;         			/* Inodes table block */
    uint16_t bg_free_blocks_count;   			/* Free blocks count */
    uint16_t bg_free_inodes_count;   			/* Free inodes count */
    uint16_t bg_used_dirs_count;     			/* Directories count */
} group_descriptor;

//EXT2 Inode
/*
 * second extended file system inode data in memory
 */
typedef struct inode {
    time_t tmpa;                  // data de acesso
    time_t tmpc;                  // data criação
    time_t tmpm;                  // delete modificacao
    time_t tmpd;                  // delete time
    uint16_t mod;                 //  tipo de arquivo e direitos de acesso
    uint16_t nblock;               // número de blocos de dados do arquivo
    uint16_t tblock;                 // tamanho do arquivo
    uint16_t ptrblock[EXT2_PTR_LENGTH];             // ponteiro para o bloco de dados
    char i_pad[EXT2_PAD_LENGTH];                  // preenchimento 
} inode;

/*
 * Structure of a directory entry
 */
//Mistura do ext2_dir_entry_2 e ext2_dir_entry
typedef struct dir_entry {
    uint16_t inod;                  // index number
    uint16_t rec_len;                // comprimento do item de diretório
    uint16_t name_len;               // comprimento da pasta
    uint16_t file_type;              // file type 1 diretório do arquivo 2
    char name[EXT2_NAME_LENGTH];   // nome do arquivo
    char dir_pad[EXT2_PAD_LENGTH-1];                 // preenchimento
} dir_entry;



/***********FS***********/
// global var 
group_descriptor group_desc;
FILE *fp;

uint16_t root_dir_inode_id;
uint16_t current_file_inode_id;
uint16_t current_dir_inode_id;
uint64_t inode_bitmap[BLOCK_SIZE/8] = {0}; //Mapa de bits de inode
uint64_t block_bitmap[BLOCK_SIZE/8] = {0}; //Mapa de bits de blocks
char global_path[EXT2_NAME_LENGTH] = "/";


/**** Declaracoes ****/

/**Sistema de Arquivos**/
int load_global();
int fs_init();
int fs_start();
int fs_close();
int formatacao();
int fs_check();
int fs_create(char *volume_name);
int fs_delete();

/**INODE**/
int get_empty_inode();
int inodeAlloc(uint16_t id);
int inodeFree(uint16_t id);
int inodeWrite(uint16_t id, inode *ind);
int inodeRead(uint16_t id, inode *ind);
int inodeInit(inode *ind);
int inodeRoot(inode *ind);

/**INODE BLOCK**/
int indBlck_write(uint16_t id, inode *ind, char *data, uint32_t data_size);
int indBlck_read(uint16_t id, inode *ind, char *data, uint32_t data_size);
int indBlck_free(uint16_t id, inode *ind, uint16_t num);
int indBlck_alloc(uint16_t id, inode *ind, uint16_t num);
int indBlck_append(uint16_t id, inode *ind, char *data, uint32_t data_size, uint32_t offset);
int indBlck_get_block_by_num(uint16_t id, inode *ind, uint32_t block_num);

/**BLOCK**/
int get_empty_block();
int block_alloc(uint16_t block_id);
int block_free(uint16_t block_id);
int block_write(uint16_t block_id, char *data, uint16_t data_size);
int block_append(uint16_t block_id, char *data, uint16_t data_size, uint16_t offset);
int block_read(uint16_t block_id, char *data, uint16_t data_size);
int block_init(inode  *inode);
int block_get_num_by_file_size(uint32_t file_size);

/**DIR ROOT**/
int dir_root_create();


/**** Implementacoes ****/
/* FS */
/**Funcoes de carga do sistema de arquivos**/
int fs_init() {
	printf("Iniciando...\n");
	// Verifica se o sistema de arquivos existe 
	int retorno = fs_check();
	//Se o retorno
	if (retorno < 0) {
        	//Criando o sistema de arquivo
		formatacao();
		load_global();
	} else {
		// load fs 
		fp = fopen(PATH, "r+");
		if (fp == NULL) {
			printf("ERROR DE INICIALIZACAO\nSaindo...\n");
			return -1;
		}
		load_global();
	}
	return 0;
}

//Fecha o arquivo
int fs_close() {
    if (fp != NULL) fclose(fp);
}

//Verifica se o arquivo existe
int fs_check() {
	fp = fopen(PATH, "r+");
	if (fp == NULL) {
		return -1;
	}
	return 0;
}

//Cria o o volume
int fs_create(char *volume_name) {
	uint16_t zero_size_64 = BLOCK_SIZE / 8;
	uint64_t zero[zero_size_64];
	//Seta zero com 0
	memset(zero, 0, sizeof(zero));
	for (uint16_t i = 0; i < BLOCKS; ++i) {
		fseek(fp, i*BLOCK_SIZE, SEEK_SET);
		fwrite(zero, sizeof(zero), 1, fp);
	}
	//Limpa o fp
	fflush(fp);
	//copia a string para o bg_volume_name
	strcpy(group_desc.bg_volume_name, volume_name);
	
	//Insere os dados
	group_desc.bg_inode_bitmap = 1;
	group_desc.bg_block_bitmap = 2;
	group_desc.bg_inode_table = 3;
	group_desc.bg_free_blocks_count = BLOCK_SIZE * 8;
	group_desc.bg_free_inodes_count = BLOCK_SIZE * 8;
	group_desc.bg_used_dirs_count = 0;
	//Continua de onde o "ponteiro de busca" parou a busca
	fseek(fp, 0, SEEK_SET);
	//Escreve na posicao encontrada, ou seja no final do arquivo
	fwrite(&group_desc, sizeof(group_desc), 1, fp);
	//limpa fp
	fflush(fp);
}

//Remove o arquivo path ou seja apaga o disco
int fs_delete() {
    return remove(PATH);
}

//Carrega as informacoes do disco
//As informacoes globais
int load_global() {
    rewind(fp);
    fread(&group_desc, sizeof(group_desc), 1, fp);
    fseek(fp, BLOCK_SIZE, SEEK_SET);
    fread(inode_bitmap, sizeof(inode_bitmap), 1, fp);
    fseek(fp, 2*BLOCK_SIZE, SEEK_SET);
    fread(block_bitmap, sizeof(block_bitmap), 1, fp);
    root_dir_inode_id = current_dir_inode_id = 1;
}

//Formatacao do disco
int formatacao() {

	printf("FS Formating...\n");
   	
	//Verifica se o path existe, caso exista deleta
	if (fs_check(PATH) == 0) {
		fs_delete(PATH);
	}
	
	//Abre o arquivo
	fp = fopen(PATH, "w+");
	//Verifica se foi aberto com sucesso
	if (fp == NULL) {
		printf("ERROR\nExiting...\n");	
		return -1;
	}
	
	//Pede o nome do volume, exemplo disco C
	printf("Nome do volume: \n");
	char volume_name[16];
	scanf("%s", volume_name);
    	
	//Cria o volume
	fs_create(volume_name);
	//Cria o diretorio /
	dir_root_create();
}

/* FS */
//Busca o Inode vazio
int get_empty_inode() {
	int array_index = 0;
	int bit_index = 0;
	for (int i = 0; i < INDEX_BLOCKS; ++i) {
		array_index = i / 64;
		bit_index = i % 64;
		if (! (inode_bitmap[array_index] & (1L << (63-bit_index)))) {
			return i+1;
		}
	}
	return -1;
}

//Alocacao do inode
int inodeAlloc(uint16_t id) {
	fp = fopen(PATH, "r+");
	if (fp == NULL) {
            printf("Erro ao abrir o arquivo, Exiting...\n");
            return -1;
        }
	
	//volta 1 posicao na posicao em que se quer alocar
	int array_index = ( id-1) / 64;
	int bit_index = ( id-1) % 64;
	//Verificar a posicao no mapa de bit se tiver esse arquivo ja existe
	if (inode_bitmap[array_index] & (1L << (63-bit_index))) return -1;
	
	//Se nao tiver vai buscar para adicionar
	inode_bitmap[array_index] |= (1L << (63-bit_index));
	fseek(fp, BLOCK_SIZE, SEEK_SET);
	//Escreve no inode_bitmap
	fwrite(inode_bitmap, sizeof(inode_bitmap), 1, fp);
	//Diminui a memoria livre
	--group_desc.bg_free_inodes_count;
	//Escreve no group desc
	fseek(fp, 0, SEEK_SET);
	fwrite(&group_desc, sizeof(group_desc), 1, fp);
	fflush(fp);
	//Se deu bom retorna 0
	return 0;
}

//Libera inode, ou seja o inverso do aloc
int inodeFree(uint16_t id) {
	//Igual ao de alocar
	int array_index = ( id-1) / 64;
	int bit_index = ( id-1) % 64;
	if (! (inode_bitmap[array_index] & (1L << (63-bit_index)))) return 0;
	inode_bitmap[array_index] &= (~(1L << (63-bit_index)));
	fseek(fp, BLOCK_SIZE, SEEK_SET);
	fwrite(inode_bitmap, sizeof(inode_bitmap), 1, fp);
	++group_desc.bg_free_inodes_count;
	fseek(fp, 0, SEEK_SET);
	fwrite(&group_desc, sizeof(group_desc), 1, fp);
	fflush(fp);
	return 0;
}

//Busca e escreve no armazenamento
int inodeWrite(uint16_t id, inode *ind) {
	if (ind == NULL) return -1;
	fseek(fp, 3*BLOCK_SIZE+(id-1)*INODE_SIZE, SEEK_SET);
	fwrite(ind, sizeof(inode), 1, fp);
	fflush(fp);
	return 0;
}

//Busca e lê no armazenamento. Pula o inicio, pois é o root
int inodeRead(uint16_t id, inode *ind) {
    if (ind == NULL) return -1;
    fseek(fp, 3*BLOCK_SIZE+( id-1)*INODE_SIZE, SEEK_SET);
    fread(ind, sizeof(ind), 1, fp);
    return 0;
}

//Inicia o inode zerado
int inodeInit(inode *ind) {
    if (ind == NULL) return -1;
    memset(ind, 0, sizeof(inode));
    return 0;
}

//Lê o inode do root que esta no incio do arquivo.
int inodeRoot(inode *ind) {
    if (ind == NULL) return -1;
    fseek(fp, 3*BLOCK_SIZE, SEEK_SET);
    fread(ind, sizeof(ind), 1, fp);
    return 0;
}

/* Block Inode */
int indBlck_write(uint16_t  id, inode *ind, char *data, uint32_t data_size) {
    return indBlck_append( id, ind, data, data_size, 0);
}

int indBlck_append(uint16_t  id, inode *ind, char *data, uint32_t data_size, uint32_t offset) {
    if (ind == NULL || data == NULL) return -1;
    int file_size = data_size + offset;
    if (file_size > INDEX_LIMIT) return -1;
    int blocks = block_get_num_by_file_size(file_size);
    if (blocks < ind->nblock) {
        indBlck_free( id, ind, ind->nblock-blocks);
    } else if (blocks > ind->nblock) {
        indBlck_alloc( id, ind, blocks-ind->nblock);
    }
    //find where to append 
    int write_block = offset / BLOCK_SIZE;
    int write_offset = offset % BLOCK_SIZE;
    int write_data_size =  data_size;
    int write_block_id = 0;
    char *write_data = data;
    while (write_data_size > 0) {
        write_block_id = indBlck_get_block_by_num( id, ind, write_block);
        int tmp_write_data_size = (write_data_size > (BLOCK_SIZE - write_offset)) ? BLOCK_SIZE-write_offset : write_data_size;
        block_append(write_block_id, write_data, tmp_write_data_size, write_offset);
        write_offset = 0;
        write_data_size -= tmp_write_data_size;
        write_data += tmp_write_data_size;
        ++write_block;
    }
    ind->tblock = file_size;
    inodeWrite( id, ind);
    return 0;
}

int indBlck_free(uint16_t  id, inode *ind, uint16_t num) {
    if (ind == NULL) return -1;
    if (num >= ind->nblock) {
        // free direct index  
        int direct_index = (num <= DIRECT_INDEX_BLOCKS) ? num : DIRECT_INDEX_BLOCKS;
        for (int i = 0; i < direct_index; ++i) {
            block_free(ind->ptrblock[i]);
            ind->ptrblock[i] = 0;
        }
        //free one index  
        int one_index = (num-DIRECT_INDEX_BLOCKS > ONE_INDEX_BLOCKS) ? ONE_INDEX_BLOCKS : num-DIRECT_INDEX_BLOCKS;
        if (one_index > 0) {
            uint16_t *block_ids = (uint16_t *) malloc(one_index*sizeof(uint16_t));
            block_read(ind->ptrblock[6], block_ids, one_index*sizeof(uint16_t));
            for (int i = 0; i < one_index; ++i) {
                block_free(block_ids[i]);
            }
            free(block_ids);
        }
        // free two index  
        int two_index = num-DIRECT_INDEX_BLOCKS-ONE_INDEX_BLOCKS;
        if (two_index > 0) {
            int array_num = two_index / (ONE_INDEX_BLOCKS);
            int last_num = two_index % ONE_INDEX_BLOCKS;
            int read_num = array_num + ((last_num>0)?1:0);
            uint16_t *block_array = (uint16_t *)malloc(read_num*sizeof(uint16_t));
            block_read(ind->ptrblock[7], block_array, read_num*sizeof(uint16_t));
            for (int i = 0; i < array_num; ++i) {
                uint16_t *block_ids = (uint16_t *)malloc(BLOCK_SIZE);
                block_read(block_array[i], block_ids, BLOCK_SIZE);
                for (int j = 0; j < ONE_INDEX_BLOCKS; ++j) {
                    block_free(block_ids[j]);
                }
                free(block_ids);
            }
            if (last_num > 0) {
                uint16_t *block_ids = (uint16_t *)malloc(last_num*sizeof(uint16_t));
                block_read(block_array[array_num], block_ids, last_num*sizeof(uint16_t));
                for (int i = 0; i < last_num; ++i) {
                    block_free(block_ids[i]);
                }
                free(block_ids);
            }
            free(block_array);
        }
    } else  {
        if (ind->nblock <= DIRECT_INDEX_BLOCKS) {
            for (int i = 1; i <= num; ++i) {
                block_free(ind->ptrblock[ind->nblock-i]);
            }
        } else if (ind->nblock <= DIRECT_INDEX_BLOCKS+ONE_INDEX_BLOCKS) {
            int blocks = ind->nblock-DIRECT_INDEX_BLOCKS;
            uint16_t *block_ids = (uint16_t *)malloc(blocks*sizeof(uint16_t));
            block_read(ind->ptrblock[6], block_ids, blocks*sizeof(uint16_t));
            int free_blocks = (ind->nblock-num <= DIRECT_INDEX_BLOCKS) ? blocks : num;
            for (int i = 0; i < free_blocks ; ++i) {
                block_free(block_ids[blocks-i-1]);
            }
            if (free_blocks >= blocks) block_free(ind->ptrblock[6]);
            for (int i = 1, size = num-blocks; i <= size; ++i) {
                block_free(ind->ptrblock[DIRECT_INDEX_BLOCKS-i]);
                ind->ptrblock[DIRECT_INDEX_BLOCKS-i] = 0;
            }
            free(block_ids);
        } else if (ind->nblock <= INDEX_BLOCKS) {
            int two_index_blocks = ind->nblock-DIRECT_INDEX_BLOCKS-ONE_INDEX_BLOCKS;
            int one_index_blocks = ONE_INDEX_BLOCKS;
            int direct_index_blocks = DIRECT_INDEX_BLOCKS;
            int free_two_index_blocks = (ind->nblock-num <= DIRECT_INDEX_BLOCKS + ONE_INDEX_BLOCKS) ? two_index_blocks : num;
            int free_one_index_blocks = (ind->nblock-num <= DIRECT_INDEX_BLOCKS) ? one_index_blocks : num-free_two_index_blocks;
            int free_direct_index_blocks = num-free_two_index_blocks-free_one_index_blocks;

            if (free_two_index_blocks > 0) {
                int array_num = two_index_blocks / ONE_INDEX_BLOCKS;
                int last_num = two_index_blocks % ONE_INDEX_BLOCKS;
                int read_num = array_num + (last_num>0?1:0);
                uint16_t *block_array = (uint16_t *)malloc(read_num*sizeof(uint16_t));
                block_read(ind->ptrblock[7], block_array, read_num*sizeof(uint16_t));
                int free_blocks_except_last_num = free_two_index_blocks - last_num;
                int free_array_num = free_blocks_except_last_num / ONE_INDEX_BLOCKS;
                int free_last_num = free_blocks_except_last_num % ONE_INDEX_BLOCKS;
                int free_read_num = free_array_num + (free_last_num>0?1:0);

                if (last_num > 0) {
                    uint16_t *block_ids = (uint16_t *)malloc(last_num*sizeof(uint16_t));
                    block_read(block_array[array_num], block_ids, last_num*sizeof(uint16_t));
                    for (int i = 0, index = last_num-1, size = free_blocks_except_last_num<0?free_two_index_blocks:last_num; i < size; ++i, --index) {
                        block_free(block_ids[index]);
                        block_ids[index] = 0;
                    }
                    block_write(block_array[array_num], block_ids, last_num*sizeof(uint16_t));
                    free(block_ids);
                    if (free_blocks_except_last_num >= 0) block_free(block_array[array_num]);
                }
                
                if (free_array_num > 0) {
                    uint16_t *block_ids = (uint16_t *)malloc(BLOCK_SIZE*sizeof(uint16_t));                
                    for (int i = 0, index = array_num-1; i < free_array_num; ++i, --index) {
                        block_read(block_array[index], block_ids, BLOCK_SIZE*sizeof(uint16_t));
                        for (int j = 0; j < BLOCK_SIZE; ++j) {
                            block_free(block_ids[j]);
                        }
                        block_free(block_array[index]);
                        block_array[index] = 0;
                    }
                    free(block_ids);
                }

                if (free_last_num > 0) {
                    uint16_t *block_ids = (uint16_t *)malloc(BLOCK_SIZE*sizeof(uint16_t));                
                    int free_index = array_num-free_array_num-1;
                    block_read(block_array[free_index], block_ids, BLOCK_SIZE*sizeof(uint16_t));
                    for (int i = 0, index = BLOCK_SIZE-1; i < free_last_num; ++i, --index) {
                        block_free(block_ids[i]);
                        block_ids[i] = 0;
                    }
                    block_write(block_array[free_index], block_ids, BLOCK_SIZE*sizeof(uint16_t));
                    free(block_ids);
                }

                if (free_two_index_blocks == two_index_blocks) {
                    block_free(ind->ptrblock[7]);
                    ind->ptrblock[7] = 0;
                }
            }

            if (free_one_index_blocks > 0) {
                uint16_t *block_ids = (uint16_t *)malloc(one_index_blocks*sizeof(uint16_t));
                block_read(ind->ptrblock[6], block_ids, one_index_blocks*sizeof(uint16_t));
                for (int i = 0, index = one_index_blocks-1; i < free_one_index_blocks; ++i, --index) {
                    block_free(block_ids[index]);
                    block_ids[index] = 0;
                }
                block_write(ind->ptrblock[6], block_ids, one_index_blocks*sizeof(uint16_t));
                free(block_ids);
                if (free_one_index_blocks >= one_index_blocks) block_free(ind->ptrblock[6]);
            }

            if (free_direct_index_blocks > 0) {
                for (int i = 0, index = DIRECT_INDEX_BLOCKS-1; i < free_direct_index_blocks; ++i, --index) {
                    block_free(ind->ptrblock[index]);
                    ind->ptrblock[index] = 0;
                }
            }

            return -1;
        }
    }
    ind->nblock -= ((num>ind->nblock)?ind->nblock:num);
    return inodeWrite( id, ind);
}

int indBlck_alloc(uint16_t  id, inode  *ind, uint16_t num) {
    if (ind == NULL) return -1;
    if (ind->nblock+num > INDEX_BLOCKS) return -1;
    while (num--) {
        if (ind->nblock < DIRECT_INDEX_BLOCKS) {
            int block_id = get_empty_block();
            if (block_id < 0) return -1;
            block_alloc(block_id);
            ind->ptrblock[ind->nblock++] = block_id;
        } else if (ind->nblock < DIRECT_INDEX_BLOCKS+ONE_INDEX_BLOCKS) {
            int block_size = ind->nblock-DIRECT_INDEX_BLOCKS;
            if (ind->ptrblock[6] == 0) {
                int block_id = get_empty_block();
                if (block_id < 0) return -1;
                block_alloc(block_id);
                ind->ptrblock[6] = block_id;
            }
            int block_id = get_empty_block();
            if (block_id < 0) return -1;
            block_alloc(block_id);
            block_append(ind->ptrblock[6], &block_id, sizeof(uint16_t), sizeof(uint16_t)*block_size);
            ++ind->nblock;
        } else if (ind->nblock < INDEX_BLOCKS) {
            int block_size = ind->nblock-DIRECT_INDEX_BLOCKS-ONE_INDEX_BLOCKS;
            int array_num = block_size / ONE_INDEX_BLOCKS;
            int last_num = block_size % ONE_INDEX_BLOCKS;
            int read_num = array_num + (last_num>0?1:0);
            uint16_t *block_array = (uint16_t *)malloc((array_num+1)*sizeof(uint16_t));
            if (ind->ptrblock[7] == 0) {
                int block_id = get_empty_block();
                if (block_id < 0) return -1;
                block_alloc(block_id);
                ind->ptrblock[7] = block_id;
            } else {
                block_read(ind->ptrblock[7], block_array, read_num*sizeof(uint16_t));
            }
            if (last_num > 0) {
                int block_id = get_empty_block();
                if (block_id < 0) return -1;
                block_alloc(block_id);
                block_append(block_array[array_num], &block_id, sizeof(uint16_t), last_num*sizeof(uint16_t));
            } else {
                int block_array_id = get_empty_block();
                if (block_array_id < 0) return -1;
                block_alloc(block_array_id);
                block_array[array_num] = block_array_id;
                int block_id = get_empty_block();
                if (block_id < 0) return -1;
                block_alloc(block_id);
                block_write(block_array_id, &block_id, sizeof(uint16_t));
            }
            block_write(ind->ptrblock[7], block_array, (array_num+1)*sizeof(uint16_t));
            free(block_array);
            ++ind->nblock;
        } else {
            return -1;
        }
    }
    inodeWrite( id, ind);
}

int indBlck_get_block_by_num(uint16_t  id, inode  *ind, uint32_t block_num) {
    if (ind == NULL) return -1;
    int block_id = 0;
    if (block_num < DIRECT_INDEX_BLOCKS) {
        block_id = ind->ptrblock[block_num];
    } else if (block_num < DIRECT_INDEX_BLOCKS+ONE_INDEX_BLOCKS) {
        int one_index_block = block_num-DIRECT_INDEX_BLOCKS;
        uint16_t *block_ids = (uint16_t *)malloc(ONE_INDEX_BLOCKS*sizeof(uint16_t));
        block_read(ind->ptrblock[6], block_ids, ONE_INDEX_BLOCKS*sizeof(uint16_t));
        block_id = block_ids[one_index_block-1];
        free(block_ids);
    } else {
        int two_index_blocks = ind->nblock-DIRECT_INDEX_BLOCKS-ONE_INDEX_BLOCKS;
        int two_index_block = block_num-DIRECT_INDEX_BLOCKS-ONE_INDEX_BLOCKS;
        int array_num = two_index_block / ONE_INDEX_BLOCKS;
        int last_num = two_index_block % ONE_INDEX_BLOCKS;
        int read_num = array_num + (last_num>0?1:0);
        uint16_t *block_array = (uint16_t *)malloc(read_num*sizeof(uint16_t));
        block_read(ind->ptrblock[7], block_array, read_num*sizeof(uint16_t));
        uint16_t *block_ids = (uint16_t *)malloc(BLOCK_SIZE*sizeof(uint16_t));
        block_read(block_array[array_num-1], block_ids, BLOCK_SIZE*sizeof(uint16_t));
        block_id = block_ids[(last_num-1+ONE_INDEX_BLOCKS)%ONE_INDEX_BLOCKS];
        free(block_ids);
        free(block_array);
    }
    return block_id;
}

/* Block */

int get_empty_block() {
    int array_index = 0;
    int bit_index = 0;
    for (int i = 0; i < INDEX_BLOCKS; ++i) {
        array_index = i / 64;
        bit_index = i % 64;
        if (! (block_bitmap[array_index] & (1L << (63-bit_index)))) {
            return i;
        }
    }
    return -1;
}


int block_alloc(uint16_t id) {
    int array_index = id / 64;
    int bit_index = id % 64;
    if (block_bitmap[array_index] & (1L << (63-bit_index))) return -1;
    block_bitmap[array_index] |= (1L << (63-bit_index));
    fseek(fp, 2*BLOCK_SIZE, SEEK_SET);
    fwrite(block_bitmap, sizeof(block_bitmap), 1, fp);
    --group_desc.bg_free_blocks_count;
    fseek(fp, 0, SEEK_SET);
    fwrite(&group_desc, sizeof(group_desc), 1, fp);
    fflush(fp);
    return 0;
}

int block_free(uint16_t id) {
    int array_index = id / 64;
    int bit_index = id % 64;
    if (! (block_bitmap[array_index] & (1L << (63-bit_index)))) return 0;
    block_bitmap[array_index] &= (~(1L << (63-bit_index)));
    fseek(fp, 2*BLOCK_SIZE, SEEK_SET);
    fwrite(block_bitmap, sizeof(block_bitmap), 1, fp);
    ++group_desc.bg_free_blocks_count;
    fseek(fp, 0, SEEK_SET);
    fwrite(&group_desc, sizeof(group_desc), 1, fp);
    fflush(fp);
    return 0;
}

int block_append(uint16_t id, char *data, uint16_t data_size, uint16_t offset) {
    if (data == NULL) return -1;
    if (data_size+offset > BLOCK_SIZE) return -1;
    fseek(fp, (DATA_BEGIN_BLOCK+id)*BLOCK_SIZE+offset, SEEK_SET);
    fwrite(data, data_size, 1, fp);
    fflush(fp);
    return 0;
}

int block_write(uint16_t id, char *data, uint16_t data_size) {
    return block_append(id, data, data_size, 0);
}

int block_read(uint16_t id, char *data, uint16_t data_size) {
    if (data == NULL) return -1;
    fseek(fp, (DATA_BEGIN_BLOCK+id)*BLOCK_SIZE, SEEK_SET);
    fread(data, data_size, 1, fp);
    return 0;
}

int block_get_num_by_file_size(uint32_t file_size) {
    if (file_size <= 0) return -1;
    int blocks = file_size / BLOCK_SIZE;
    if (file_size % BLOCK_SIZE) ++blocks;
    return blocks;
}

int indBlck_read(uint16_t  id, inode *ind, char *data, uint32_t data_size) {

    if (ind == NULL || data == NULL) return -1;
    if (data_size > ind->tblock) data_size = ind->tblock;
    int read_data_size = data_size;
    int read_block = 0;
    int read_block_id = 0;
    char *read_data = data;
    while (read_data_size > 0) {
        read_block_id = indBlck_get_block_by_num( id, ind, read_block);
        ++read_block;
        int tmp_read_data_size = (read_data_size > BLOCK_SIZE) ? BLOCK_SIZE : read_data_size;
        block_read(read_block_id, read_data, tmp_read_data_size);
        read_data_size -= tmp_read_data_size;
        read_data += tmp_read_data_size;
    }
    return 0;
}

/* Dir root */

int dir_root_create() {
    int inode_id = get_empty_inode(); // 1
    int inodeAlloc_result = inodeAlloc(inode_id);
    if (inodeAlloc_result < 0) {
        return -1;
    }
    inode root_inode;
    inodeInit(&root_inode);
    dir_entry root_dir_entry;
    time_t now;
    time(&now);
    root_inode.tmpa = now;
    root_inode.tmpc = now;
    root_inode.tmpm = now;
    root_inode.tmpd = 0;
    root_inode.mod = FILE_TYPE_DIR;
    root_inode.nblock = 0;
    root_inode.tblock = 0;
    inodeWrite(inode_id, &root_inode);
    root_dir_entry.inod = inode_id;
    root_dir_entry.rec_len = DIR_SIZE;
    root_dir_entry.name_len = 1;
    root_dir_entry.file_type = FILE_TYPE_DIR;
    strcpy(root_dir_entry.name, ".");
    indBlck_write(inode_id, &root_inode, &root_dir_entry, sizeof(root_dir_entry));
    root_dir_entry.name_len = 2;
    strcpy(root_dir_entry.name, "..");
    indBlck_append(inode_id, &root_inode, &root_dir_entry, sizeof(root_dir_entry), sizeof(root_dir_entry));
    return 0;
}
