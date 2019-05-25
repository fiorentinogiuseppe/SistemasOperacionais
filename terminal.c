#include"fs.h"
#include<stdio.h>

int fs_start() {

	int flag = 0;
	//inicializacao
	flag = fs_init();
	// chama api
	if (flag >= 0) //fs_command_loop();
		printf("LOOP\n");
	//fecha o arquivo 
	fs_close();
    
}



void main (void){
	formatacao();
	fs_start();		
}
