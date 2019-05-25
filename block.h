/** block **/
int get_empty_block();
int block_alloc(uint16_t block_id);
int block_free(uint16_t block_id);
int block_write(uint16_t block_id, char *data, uint16_t data_size);
int block_append(uint16_t block_id, char *data, uint16_t data_size, uint16_t offset);
int block_read(uint16_t block_id, char *data, uint16_t data_size);
int block_init(myext2_inode *inode);
int block_get_num_by_file_size(uint32_t file_size);
