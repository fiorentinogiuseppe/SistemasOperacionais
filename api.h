#include "ext2.h"

/** fs **/
int fs_check();
int fs_init();
int fs_create(char *volume_name, char *psw);
int fs_delete();
int fs_close();

/** load datas **/
int load_global();

/** terminal **/
int terminal_interativo();

/** arquivo **/
int is_file(uint16_t inode_id, myext2_inode *inode, char *file_name);
int file_check(uint16_t inode_id, myext2_inode *inode, char *file_name);
int file_create(uint16_t inode_id, myext2_inode *inode, char *file_name);
int file_delete(uint16_t inode_id, myext2_inode *inode, char *file_name);
int file_write(uint16_t inode_id, myext2_inode *inode, char *file_name, char *buff, uint32_t size);
int file_read(uint16_t inode_id, myext2_inode *inode, char *file_name, char *buff, uint32_t size);
int file_size(uint16_t inode_id, myext2_inode *inode, char *file_name);

/** dir **/
int is_dir(uint16_t inode_id, myext2_inode *inode, char *file_name);
int is_current_dir(uint16_t inode_id, myext2_inode *inode, char *file_name);
int dir_is_exists(uint16_t inode_id, myext2_inode *inode, char *file_name);
int dir_create(uint16_t inode_id, myext2_inode *inode, char *file_name);
int dir_delete(uint16_t inode_id, myext2_inode *inode, char *file_name);
int dir_info(uint16_t inode_id, myext2_inode *inode);
int dir_child(uint16_t inode_id, myext2_inode *inode, char *file_name, myext2_inode *child);
int dir_root_create();

/** inode **/
int get_empty_inode();
int inode_alloc(uint16_t inode_id);
int inode_free(uint16_t inode_id);
int inode_write(uint16_t inode_id, myext2_inode *inode);
int inode_read(uint16_t inode_id, myext2_inode *inode);
int inode_init(myext2_inode *inode);
int inode_block_write(uint16_t inode_id, myext2_inode *inode, char *data, uint32_t data_size);
int inode_block_read(uint16_t inode_id, myext2_inode *inode, char *data, uint32_t data_size);
int inode_block_free(uint16_t inode_id, myext2_inode *inode, uint16_t num);
int inode_block_alloc(uint16_t inode_id, myext2_inode *inode, uint16_t num);
int inode_block_append(uint16_t inode_id, myext2_inode *inode, char *data, uint32_t data_size, uint32_t offset);
int inode_block_get_block_by_num(uint16_t inode_id, myext2_inode *inode, uint32_t block_num);
int inode_root(myext2_inode *inode);

/** block **/
int get_empty_block();
int block_alloc(uint16_t block_id);
int block_free(uint16_t block_id);
int block_write(uint16_t block_id, char *data, uint16_t data_size);
int block_append(uint16_t block_id, char *data, uint16_t data_size, uint16_t offset);
int block_read(uint16_t block_id, char *data, uint16_t data_size);
int block_init(myext2_inode *inode);
int block_get_num_by_file_size(uint32_t file_size);

/** path **/
int path_pwd(uint16_t inode_id, char *path);

/** ui **/

int fs_start();
