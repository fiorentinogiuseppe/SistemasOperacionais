#include <stdint.h>
#include <time.h>
#define EXT2_NAME_LENGTH 15
#define EXT2_PTR_LENGTH 8
#define EXT2_PAD_LENGTH 10

/*
Algumas definições
https://www.landley.net/code/toybox/ext2.html
*/

typedef struct group_descriptor
{
    char bg_volume_name[EXT2_NAME_LENGTH + 1];         /* Volume name*/
    uint16_t bg_block_bitmap;        /* Blocks bitmap block */ 
    uint16_t bg_inode_bitmap;        /* Inodes bitmap block */ 
    uint16_t bg_inode_table;         /* Inodes table block */
    uint16_t bg_free_blocks_count;   /* Free blocks count */
    uint16_t bg_free_inodes_count;   /* Free inodes count */
    uint16_t bg_used_dirs_count;     /* Directories count */
} group_descriptor;


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


typedef struct dir_info {
    uint16_t inode;                  // index number
    uint16_t rec_len;                // comprimento do item de diretório
    uint16_t name_len;               // comprimento da pasta
    uint16_t file_type;              // file type 1 diretório do arquivo 2
    char name[EXT2_NAME_LENGTH];   // nome do arquivo
    char dir_pad[EXT2_PAD_LENGTH-1];                 // preenchimento
} dir_entry;

