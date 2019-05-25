#include "ext2.h"
#include<stdio.h>
#include<string.h>

// global var 
group_descriptor group_desc;

uint16_t root_dir_inode_id;
uint16_t current_file_inode_id;
uint16_t current_dir_inode_id;
char global_path[EXT2_NAME_LENGTH] = "/";

FILE *fp;

uint64_t inode_bitmap[BLOCK_SIZE/8] = {0};
uint64_t block_bitmap[BLOCK_SIZE/8] = {0};

/**** Declaracoes ****/
/**Sistema de Arquivos**/
//Carregar informacoes globais
int load_global();

//Funcoes de carga do sistema de arquivos
int fs_init();
int fs_start();
int fs_close();

//CRUD do sistema de arquivos
int formatacao();
int fs_check();
int fs_create(char *volume_name/*, char *psw*/);
int fs_delete();

/**** Implementacoes ****/
/* FS */
/**Funcoes de carga do sistema de arquivos**/
int fs_init() {
	printf("Iniciando Sistema...\n");
	// Verifica se o sistema de arquivos existe 
	int retorno = fs_check();
	//Se o retorno
	if (fs_check < 0) {
        	//Criando o sistema de arquivo
		formatacao();
		load_global();
	} else {
		// load fs 
		fp = fopen(PATH, "r+");
		if (fp == NULL) {
			printf("ERROR DE INICIALIZACAO\nExiting...\n");
			return -1;
		}
		load_global();
	}
	return 0;
}

int fs_start() {

	int flag = 0;
	//inicializacao
	flag = fs_init();
	// chama api
	if (flag >= 0) //fs_command_loop();
		printf("LOOP");
	//fecha o arquivo 
	fs_close();
    
}

int fs_close() {
    if (fp != NULL) fclose(fp);
}

/**CRUD do sistema de arquivos**/
int fs_check() {
	fp = fopen(PATH, "r+");
	if (fp == NULL) {
		return -1;
	}
	return 0;
}


int fs_create(char *volume_name/*, char *psw*/) {
    uint16_t zero_size_64 = BLOCK_SIZE / 8;
    uint64_t zero[zero_size_64];
    memset(zero, 0, sizeof(zero));
    for (uint16_t i = 0; i < BLOCKS; ++i) {
        fseek(fp, i*BLOCK_SIZE, SEEK_SET);
        fwrite(zero, sizeof(zero), 1, fp);
    }
    fflush(fp);
    strcpy(group_desc.bg_volume_name, volume_name);
    group_desc.bg_inode_bitmap = 1;
    group_desc.bg_block_bitmap = 2;
    group_desc.bg_inode_table = 3;
    group_desc.bg_free_blocks_count = BLOCK_SIZE * 8;
    group_desc.bg_free_inodes_count = BLOCK_SIZE * 8;
    group_desc.bg_used_dirs_count = 0;
    //strcpy(group_desc.psw, psw);
    fseek(fp, 0, SEEK_SET);
    fwrite(&group_desc, sizeof(group_desc), 1, fp);
    fflush(fp);
}


int fs_delete() {
    return remove(PATH);
}

int load_global() {
    rewind(fp);
    fread(&group_desc, sizeof(group_desc), 1, fp);
    fseek(fp, BLOCK_SIZE, SEEK_SET);
    fread(inode_bitmap, sizeof(inode_bitmap), 1, fp);
    fseek(fp, 2*BLOCK_SIZE, SEEK_SET);
    fread(block_bitmap, sizeof(block_bitmap), 1, fp);
    root_dir_inode_id = current_dir_inode_id = 1;
}

int formatacao() {

    printf("FS Formating...\n");
    
    if (fs_check(PATH) == 0) {
        fs_delete(PATH);
    }

    fp = fopen(PATH, "w+");
    if (fp == NULL) {
        printf("ERROR\nExiting...\n");
        return -1;
    }

    printf("Input Volume Name\n");
    char volume_name[16];
    scanf("%s", volume_name);
    
    fs_create(volume_name);

   // dir_root_create();

}
