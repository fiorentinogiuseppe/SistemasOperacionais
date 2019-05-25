#include"inodeBlock.h"
//#include"fs.h"
//#include<stdio.h>

void main (void){
	inode tmp_inode;
	int id=getEmptyInode();
	printf("%i\n",id);
	inodeAlloc(id);
	int retorno=inode_block_write(id, &tmp_inode, chama(), tmp_inode.tblock);
	printf("%i",retorno);

}
