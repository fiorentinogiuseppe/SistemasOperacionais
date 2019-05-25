#include"ext2.h"
#include<stdio.h>
#include<string.h>

FILE *fp;

group_descriptor group_desc;
uint64_t inode_bitmap[BLOCK_SIZE/8] = {0};
uint64_t block_bitmap[BLOCK_SIZE/8] = {0};

/** inode  **/
/**int inodeBlockWrite(uint16_t id, inode *ind, char *data, uint32_t data_size);
int inodeBlockRead(uint16_t id, inode *ind, char *data, uint32_t data_size);
int inodeBlockFree(uint16_t id, inode *ind, uint16_t num);
int inodeBlockAlloc(uint16_t id, inode *ind, uint16_t num);
int inodeBlockAppend(uint16_t id, inode *ind, char *data, uint32_t data_size, uint32_t offset);
int inodeBlockGetBlockByNum(uint16_t id, inode *ind, uint32_t block_num);**/

int getEmptyInode() {
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

int inodeAlloc(uint16_t id) {
	fp = fopen(PATH, "r+");
	if (fp == NULL) {
            printf("Open FS Failed, Exiting...\n");
            return -1;
        }

	int array_index = ( id-1) / 64;
	int bit_index = ( id-1) % 64;
	if (inode_bitmap[array_index] & (1L << (63-bit_index))) return -1;
	inode_bitmap[array_index] |= (1L << (63-bit_index));

	printf("[inode_alloc] array_index = %d, bit_index = %d, | = %llx\n", array_index, bit_index, (1L << (63-bit_index)));
	printf("[inode_alloc] inode_bitmap[array_index] = %llx\n", inode_bitmap[array_index]);

	fseek(fp, BLOCK_SIZE, SEEK_SET);
	fwrite(inode_bitmap, sizeof(inode_bitmap), 1, fp);
	--group_desc.bg_free_inodes_count;
	fseek(fp, 0, SEEK_SET);
	fwrite(&group_desc, sizeof(group_desc), 1, fp);
	fflush(fp);
	return 0;
}

int inodeFree(uint16_t id) {
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

int inodeWrite(uint16_t id, inode *ind) {
	if (ind == NULL) return -1;
	fseek(fp, 3*BLOCK_SIZE+(id-1)*INODE_SIZE, SEEK_SET);
	fwrite(ind, sizeof(inode), 1, fp);
	fflush(fp);
	return 0;
}

int inodeRead(uint16_t id, inode *ind) {
    if (ind == NULL) return -1;
    fseek(fp, 3*BLOCK_SIZE+( id-1)*INODE_SIZE, SEEK_SET);
    fread(ind, sizeof(inode), 1, fp);
    return 0;
}

int inodeInit(inode *ind) {
    if (ind == NULL) return -1;
    memset(ind, 0, sizeof(inode));
    return 0;
}

int inodeRoot(inode *ind) {
    if (ind == NULL) return -1;
    fseek(fp, 3*BLOCK_SIZE, SEEK_SET);
    fread(ind, sizeof(ind), 1, fp);
    return 0;
}
