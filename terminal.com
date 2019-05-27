#include"fs.h"
#include<stdio.h>  
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//Lista de comandos
char *commands[] = {
    "format",
    "ls",
    "create",
    "delete",
    "cd",
    "read",
    "write",
    "exit"
};

/** file **/
int is_file(uint16_t inode_id, inode *ind, char *file_name);
int file_is_exists(uint16_t inode_id, inode *ind, char *file_name);
int file_create(uint16_t inode_id, inode *ind, char *file_name);
int file_delete(uint16_t inode_id, inode *ind, char *file_name);
int file_write(uint16_t inode_id, inode *ind, char *file_name, char *buff, uint32_t size);
int file_read(uint16_t inode_id, inode *ind, char *file_name, char *buff, uint32_t size);
int file_size(uint16_t inode_id, inode *ind, char *file_name);
int dir_info(uint16_t inode_id, inode *ind);
int dir_create(uint16_t inode_id, inode *ind, char *file_name);


/** dir **/
int is_dir(uint16_t inode_id, inode *ind, char *file_name);
int is_current_dir(uint16_t inode_id, inode *ind, char *file_name);
int dir_is_exists(uint16_t inode_id, inode *ind, char *file_name);
int dir_delete(uint16_t inode_id, inode *ind, char *file_name);

int dir_child(uint16_t inode_id, inode *ind, char *file_name, inode *child);



/** ui **/
int fs_command_loop() {
    int flag = 1;

    /** command **/
    char command[16];
    uint8_t matched;
	char teste;
    while (flag >= 0) {
        printf("[giuseppe@bcc-ufrpe]:%s$ ", global_path);
        scanf("%s", &command);
        matched = 0;
        for (uint8_t i = 0; i < COMMAND_SIZE; ++i) {
		printf("%i. Press ENter...",i);
		scanf("%c",&teste);
            if (strcmp(command, commands[i]) == 0) {
                switch (i) {
                    case FORMAT:
                        flag = formatacao();
                        break; 
                    case LS:
                        flag = ext2_ls();
                        break;
                    case CREATE:
                        flag = ext2_create();
                        break;
                    case DELETE:
                        flag = ext2_delete();
                        break;
                    case CD:
                        flag = ext2_cd();
                        break;
                    case READ:
                        flag = ext2_read();
                        break;
                    case WRITE:
                        flag = ext2_write();
                        break;
                    case EXIT:
                        flag = ext2_exit();
                        break;
                }
                matched = 1;
                break;
            }
			else{
			printf("nao\n");
		}
        }
        if (! matched) {
            printf("Invalid Command '%s', Retry. \n", command);            
        }
    }
}



int dir_info(uint16_t inode_id, inode *ind) {
    if (ind == NULL) return -1;
    int dir_size = ind->tblock / DIR_SIZE;
    dir_entry *dirs = (dir_entry *)malloc(dir_size*sizeof(dir_entry));
    indBlck_read(inode_id, ind, dirs, dir_size*sizeof(dir_entry));
    inode tmp_inode;
    printf("Type\t\tFileName\tCreateTime\t\t\tLastAccessTime\t\t\tModifyTime\n");
    char ctime_buffer[80], atime_buffer[80], mtime_buffer[80];
    for (int i = 0; i < dir_size; ++i) {
        if (dirs[i].inod != 0) {
            inodeRead(dirs[i].inod, &tmp_inode);
            strftime(ctime_buffer, sizeof(ctime_buffer), "%Y-%m-%d %H:%M:%S", localtime(&tmp_inode.tmpc));
            strftime(atime_buffer, sizeof(atime_buffer), "%Y-%m-%d %H:%M:%S", localtime(&tmp_inode.tmpa));
            strftime(mtime_buffer, sizeof(mtime_buffer), "%Y-%m-%d %H:%M:%S", localtime(&tmp_inode.tmpm));
            printf("%s\t%s\t\t%s\t\t%s\t\t%s\n", 
                dirs[i].file_type == FILE_TYPE_DIR ? "Directory" : "File     ",
                dirs[i].name,
                ctime_buffer,
                atime_buffer,
                mtime_buffer
            );
        }
    }
    return 0;
}

int dir_delete(uint16_t inode_id, inode *ind, char *file_name) {
    if (ind == NULL || file_name == NULL) return -1;
    if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0) {
        printf("Can't delete '%s'.\n", file_name);
        return 0;
    }
    if (ind->nblock == 2) return 0;
    int dir_size = ind->tblock / DIR_SIZE;
    dir_entry *dirs = (dir_entry *)malloc(ind->tblock);
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {
        if (strcmp(dirs[i].name, file_name) == 0 && dirs[i].inod != 0) {
            is_exists = i;
            break;
        }
    }
    if (is_exists > 0 && dirs[is_exists].file_type == FILE_TYPE_DIR) {
        int delete_inode_id = dirs[is_exists].inod;
        dirs[is_exists].inod = 0;
        indBlck_write(inode_id, ind, dirs, ind->tblock);
        inode delete_inode;
        inodeRead(delete_inode_id, &delete_inode);
        indBlck_free(delete_inode_id, &delete_inode, delete_inode.nblock);
        inodeFree(delete_inode_id);
        printf("Delete dir '%s' success.\n", file_name);
    } else {
        printf("No dir named '%s'.\n", file_name);
    }
    free(dirs);
    return 0;
}

int dir_child(uint16_t inode_id, inode *ind, char *file_name, inode *child) {
    if (ind == NULL || file_name == NULL) return -1;
    int dir_size = ind->tblock / DIR_SIZE;
    dir_entry *dirs = (dir_entry *)malloc(ind->tblock);
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {
        if (strcmp(dirs[i].name, file_name) == 0 && dirs[i].inod != 0) {
            is_exists = i;
            break;
        }
    }
    if (is_exists > 0 && child != NULL) {
        inodeRead(is_exists, child);
    }
    free(dirs);
    return is_exists;
}


int dir_create(uint16_t inode_id, inode *ind, char *file_name) {
    if (ind == NULL || file_name == NULL) return -1;
    int dir_size = ind->tblock / DIR_SIZE;

    dir_entry *dirs = (dir_entry *)malloc(dir_size*sizeof(dir_entry));
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int has_deleted = -1;
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {

        if (strcmp(dirs[i].name, file_name) == 0) {
            if (dirs[i].inod != 0) {
                printf("Can't create dir '%s', it has existed.\n");
                is_exists = i;
                break;
            } else {
                has_deleted = i;
            }
        }
    }
    
    if (is_exists < 0) {
        int new_inode_id = get_empty_inode();
        int inodeAlloc_result = inodeAlloc(new_inode_id);
        if (inodeAlloc_result < 0) return -1;
        inode new_dir_inode;
        inodeInit(&new_dir_inode);
        dir_entry new_dir_entry;
        time_t now;
        time(&now);
        new_dir_inode.tmpa = now;
        new_dir_inode.tmpc = now;
        new_dir_inode.tmpm = now;
        new_dir_inode.tmpd = 0;
        new_dir_inode.mod = FILE_TYPE_DIR;
        new_dir_inode.nblock = 0;
        new_dir_inode.tblock = 0;
        inodeWrite(new_inode_id, &new_dir_inode);
        new_dir_entry.inod = new_inode_id;
        new_dir_entry.rec_len = DIR_SIZE;
        new_dir_entry.name_len = 1;
        new_dir_entry.file_type = FILE_TYPE_DIR;
        strcpy(new_dir_entry.name, ".");
        indBlck_write(new_inode_id, &new_dir_inode, &new_dir_entry, sizeof(new_dir_entry));
        new_dir_entry.inod = inode_id;
        new_dir_entry.name_len = 2;
        strcpy(new_dir_entry.name, "..");
        indBlck_append(new_inode_id, &new_dir_inode, &new_dir_entry, sizeof(new_dir_entry), sizeof(new_dir_entry));

        new_dir_entry.inod = new_inode_id;
        new_dir_entry.name_len = strlen(file_name);
        new_dir_entry.file_type = FILE_TYPE_DIR;
        strcpy(new_dir_entry.name, file_name);
        if (has_deleted >= 0) {
            dirs[has_deleted] = new_dir_entry;
            indBlck_write(inode_id, ind, dirs, dir_size*sizeof(dir_entry));
        } else {
            indBlck_append(inode_id, ind, &new_dir_entry, sizeof(new_dir_entry), dir_size*sizeof(dir_entry));
        }
        inodeWrite(inode_id, ind);
        printf("Create dir '%s' success.\n", file_name);
    }
    free(dirs);
    return 0;
}


int fs_start();

int ext2_ls() {
    inode current_dir_inode;
    inodeRead(current_dir_inode_id, &current_dir_inode);
    return dir_info(current_dir_inode_id, &current_dir_inode);
}

int ext2_create() {
    char type = ' ';
    char file_name[EXT2_NAME_LENGTH];
    while (type == ' ' || type == '\n') scanf("%c", &type);
    scanf("%s", file_name);
    inode current_dir_inode;
    inodeRead(current_dir_inode_id, &current_dir_inode);
    if (type == 'd') {
        dir_create(current_dir_inode_id, &current_dir_inode, file_name);
    } else if (type == 'f') {
        file_create(current_dir_inode_id, &current_dir_inode, file_name);
    } else {
        printf("Invalid File Type.\n");
    }
    return 0;
}

int ext2_delete() {
    char type = ' ';
    char file_name[EXT2_NAME_LENGTH];
    while (type == ' ' || type == '\n') scanf("%c", &type);
    scanf("%s", file_name);
    inode current_dir_inode;
    inodeRead(current_dir_inode_id, &current_dir_inode);
    if (type == 'd') {
        dir_delete(current_dir_inode_id, &current_dir_inode, file_name);
    } else if (type == 'f') {
        file_delete(current_dir_inode_id, &current_dir_inode, file_name);
    } else {
        printf("Invalid File Type.\n");
    }
    return 0;
}

int ext2_cd() {
    char path[1024];
    char tmp_path[EXT2_NAME_LENGTH];
    scanf("%s", path);
    char *p = path;
    uint16_t tmp_inode_id;
    inode tmp_inode;
    if (p[0] == '/') {
        ++p;
        strcpy(tmp_path, "/");
        tmp_inode_id = root_dir_inode_id;
    } else {
        strcpy(tmp_path, global_path);
        tmp_inode_id = current_dir_inode_id;
    }
    inodeRead(tmp_inode_id, &tmp_inode);    
    char *file_name = strtok(p, "/");
    while (file_name != NULL) {
        tmp_inode_id = dir_child(tmp_inode_id, &tmp_inode, file_name, &tmp_inode);
        if (tmp_inode_id < 0) {
            printf("Invalid path '%s'.\n", path);
            return 0;
        }

        strcpy(tmp_path, file_name);
        file_name = strtok(NULL, "/");
    }

    if (tmp_inode.mod == FILE_TYPE_DIR) {
        current_dir_inode_id = tmp_inode_id;
        strcpy(global_path, tmp_path);
    } else {
        printf("'%s' is not a dir.\n", tmp_path);
    }
    return 0;
}


int ext2_read() {
    char file_name[EXT2_NAME_LENGTH];
    scanf("%s", file_name);
    inode current_dir_inode;
    inodeRead(current_dir_inode_id, &current_dir_inode);
    int size = file_size(current_dir_inode_id, &current_dir_inode, file_name);
    if (size < 0) {
        printf("No file named '%s'.", file_name);
    } else {
        char *buff = (char *)malloc(size+1);
        file_read(current_dir_inode_id, &current_dir_inode, file_name, buff, size);
        buff[size] = '\0';
        printf("%s", buff);
        free(buff);
    }
    return 0;
}

int ext2_write() {
    char file_name[EXT2_NAME_LENGTH];
    scanf("%s", file_name);
    printf("Write to file '%s', press `ESC` to finish input.\n", file_name);
    int size = 10;
    int index = 0;
    char c;
    char *buff = (char *)malloc(size);
    while ((c = getchar()) != '\n');
    while ((c = getchar()) != 0x1B) {
        if (index+1 >= size) {
            buff = (char *)realloc(buff, size*2);
            size *= 2;
        }
        buff[index++] = c;
    }
    if (index+1 >= size) {
        buff = (char *)realloc(buff, size+1);
    }
    buff[index++] = '\0';
    inode current_dir_inode;
    inodeRead(current_dir_inode_id, &current_dir_inode);
    file_write(current_dir_inode_id, &current_dir_inode, file_name, buff, index);
    free(buff);
    return 0;
}

int fs_start() {

	int flag = 0;
	//inicializacao
	flag = fs_init();
	// chama api
	if (flag >= 0) fs_command_loop();
	//fecha o arquivo 
	fs_close();
    
}

int ext2_exit() {
    printf("Exiting...\n");
    exit(0);
}

int file_create(uint16_t inode_id, inode *ind, char *file_name) {
    if (ind == NULL || file_name == NULL) return -1;
    int dir_size = ind->tblock / DIR_SIZE;
    dir_entry *dirs = (dir_entry *)malloc(ind->tblock);
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int has_deleted = -1;
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {
        if (strcmp(dirs[i].name, file_name) == 0) {
            if (dirs[i].inod != 0) {
                printf("Can't create file '%s', it has existed.\n");
                is_exists = i;
                break;
            } else {
                has_deleted = i;
            }
        }
    }
    if (is_exists < 0) {
        int new_inode_id = get_empty_inode();
        int inodeAlloc_result = inodeAlloc(new_inode_id);
        if (inodeAlloc_result < 0) return -1;
        inode new_inode;
        inodeInit(&new_inode);
        time_t now;
        dir_entry new_dir_entry;
        time(&now);
        new_inode.tmpc = now;
        new_inode.tmpa = now;
        new_inode.tmpm = now;
        new_inode.tmpd = 0;
        new_inode.mod = FILE_TYPE_FILE;
        new_inode.nblock = 0;
        new_inode.tblock = 0;
        inodeWrite(new_inode_id, &new_inode);
        new_dir_entry.inod = new_inode_id;
        new_dir_entry.rec_len = DIR_SIZE;
        new_dir_entry.name_len = strlen(file_name);
        new_dir_entry.file_type = FILE_TYPE_FILE;
        strcpy(new_dir_entry.name, file_name);
        if (has_deleted >= 0) {
            dirs[has_deleted] = new_dir_entry;
            indBlck_write(inode_id, ind, dirs, ind->tblock);
        } else {
            indBlck_append(inode_id, ind, &new_dir_entry, DIR_SIZE, ind->tblock);
        }
        inodeWrite(inode_id, ind);
        printf("Create file '%s' success.\n", file_name);        
    }
    free(dirs);
    return 0;
}

int file_delete(uint16_t inode_id, inode *ind, char *file_name) {  
    if (ind == NULL || file_name == NULL) return -1;
    if (ind->ptrblock == 2) return 0;
    int dir_size = ind->tblock / DIR_SIZE;
    dir_entry *dirs = (dir_entry *)malloc(ind->tblock);
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {
        if (strcmp(dirs[i].name, file_name) == 0 && dirs[i].inod != 0) {
            is_exists = i;
            break;
        }
    }

    if (is_exists > 0 && dirs[is_exists].file_type == FILE_TYPE_FILE) {
        int delete_inode_id = dirs[is_exists].inod;

        dirs[is_exists].inod = 0;
        indBlck_write(inode_id, ind, dirs, ind->tblock);
        inode delete_inode;
        inodeRead(delete_inode_id, &delete_inode);
        indBlck_free(delete_inode_id, &delete_inode, delete_inode.ptrblock);
        inodeFree(delete_inode_id);
        printf("Delete file '%s' success.\n", file_name);
    } else {
        printf("No file named '%s'.\n", file_name);
    }
    free(dirs);
    return 0;
}

int file_write(uint16_t inode_id, inode *ind, char *file_name, char *buff, uint32_t size) {
    if (ind == NULL || file_name == NULL || buff == NULL) return -1;
    if (ind->ptrblock == 2) return 0;
    int dir_size = ind->tblock / DIR_SIZE;
    dir_entry *dirs = (dir_entry *)malloc(ind->tblock);
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {
        if (strcmp(dirs[i].name, file_name) == 0 && dirs[i].inod != 0) {
            is_exists = i;
            break;
        }
    }
    if (is_exists > 0 && dirs[is_exists].file_type == FILE_TYPE_FILE) {
        int write_inode_id = dirs[is_exists].inod;
        inode write_inode;
        inodeRead(write_inode_id, &write_inode);
        indBlck_write(write_inode_id, &write_inode, buff, size);
        printf("Write to file '%s' success.\n", file_name);
    } else {
        printf("No file named '%s'.\n", file_name);
    }
    free(dirs);
    return 0;
}

int file_read(uint16_t inode_id, inode *ind, char *file_name, char *buff, uint32_t size) {
    if (ind == NULL || file_name == NULL || buff == NULL) return -1;
    if (ind->ptrblock == 2) return 0;
    int dir_size = ind->tblock / DIR_SIZE;
    dir_entry *dirs = (dir_entry *)malloc(ind->tblock);
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {
        if (strcmp(dirs[i].name, file_name) == 0 && dirs[i].inod != 0) {
            is_exists = i;
            break;
        }
    }
    if (is_exists > 0 && dirs[is_exists].file_type == FILE_TYPE_FILE) {
        int read_inode_id = dirs[is_exists].inod;
        inode read_inode;
        inodeRead(read_inode_id, &read_inode);
        indBlck_read(read_inode_id, &read_inode, buff, size);
    } else {
        printf("No file named '%s'.\n", file_name);
    }
    free(dirs);
    return 0;
}

int file_size(uint16_t inode_id, inode *ind, char *file_name) {
    if (ind == NULL || file_name == NULL) return -1;
    if (ind->ptrblock == 2) return 0;
    int dir_size = ind->tblock / DIR_SIZE;
    int file_size = -1;
    dir_entry *dirs = (dir_entry *)malloc(ind->tblock);
    indBlck_read(inode_id, ind, dirs, ind->tblock);
    int is_exists = -1;
    for (int i = 0; i < dir_size; ++i) {
        if (strcmp(dirs[i].name, file_name) == 0 && dirs[i].inod != 0) {
            is_exists = i;
            break;
        }
    }
    if (is_exists > 0 && dirs[is_exists].file_type == FILE_TYPE_FILE) {
        int read_inode_id = dirs[is_exists].inod;
        inode read_inode;
        inodeRead(read_inode_id, &read_inode);
        file_size = read_inode.tblock;
    }
    free(dirs);
    return file_size;
}


/** main **/

int main(void) {
    fs_start();
    // test();
}
