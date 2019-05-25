#include"ext2.h"
#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#define DEBUG 0

dir_info* chama(){
    dir_info *dirs=(dir_info *)malloc(8*sizeof(dir_info));
 	return dirs;
}


int inode_block_write(uint16_t inode_id, inode *ind, char *data, uint32_t data_size);
int inode_block_read(uint16_t inode_id, inode *ind, char *data, uint32_t data_size);
int inode_block_free(uint16_t inode_id, inode *ind, uint16_t num);
int inode_block_alloc(uint16_t inode_id, inode *ind, uint16_t num);
int inode_block_append(uint16_t inode_id, inode *ind, char *data, uint32_t data_size, uint32_t offset);
int inode_block_get_block_by_num(uint16_t inode_id, inode *ind, uint32_t block_num);

int inode_block_read(uint16_t  id, inode *ind, char *data, uint32_t data_size) {

    if (ind == NULL || data == NULL) return -1;
    if (data_size > ind->tblock) data_size = ind->tblock;
    int read_data_size = data_size;
    int read_block = 0;
    int read_block_id = 0;
    char *read_data = data;
    while (read_data_size > 0) {
        read_block_id = inode_block_get_block_by_num( id, ind, read_block);
        ++read_block;
        int tmp_read_data_size = (read_data_size > BLOCK_SIZE) ? BLOCK_SIZE : read_data_size;
        block_read(read_block_id, read_data, tmp_read_data_size);
        read_data_size -= tmp_read_data_size;
        read_data += tmp_read_data_size;
    }
    return 0;
}

int inode_block_write(uint16_t  id, inode *ind, char *data, uint32_t data_size) {
    return inode_block_append( id, ind, data, data_size, 0);
}

int inode_block_append(uint16_t  id, inode *ind, char *data, uint32_t data_size, uint32_t offset) {
    if (ind == NULL || data == NULL) return -1;
    int file_size = data_size + offset;
    if (file_size > INDEX_LIMIT) return -1;
    int blocks = block_get_num_by_file_size(file_size);
    if (blocks < ind->nblock) {
        inode_block_free( id, ind, ind->nblock-blocks);
    } else if (blocks > ind->nblock) {
        inode_block_alloc( id, ind, blocks-ind->nblock);
    }
    //find where to append 
    int write_block = offset / BLOCK_SIZE;
    int write_offset = offset % BLOCK_SIZE;
    int write_data_size =  data_size;
    int write_block_id = 0;
    char *write_data = data;
    while (write_data_size > 0) {
        write_block_id = inode_block_get_block_by_num( id, ind, write_block);
        int tmp_write_data_size = (write_data_size > (BLOCK_SIZE - write_offset)) ? BLOCK_SIZE-write_offset : write_data_size;
        block_append(write_block_id, write_data, tmp_write_data_size, write_offset);
        write_offset = 0;
        write_data_size -= tmp_write_data_size;
        write_data += tmp_write_data_size;
        ++write_block;
    }
    ind->tblock = file_size;
    inode_write( id, ind);
    return 0;
}

int inode_block_free(uint16_t  id, inode *ind, uint16_t num) {
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
    return inode_write( id, ind);
}

int inode_block_alloc(uint16_t  id, inode  *ind, uint16_t num) {
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
    inode_write( id, ind);
}

int inode_block_get_block_by_num(uint16_t  id, inode  *ind, uint32_t block_num) {
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
