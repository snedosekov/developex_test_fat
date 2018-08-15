#include <stdio.h>
#include <fcntl.h>

#include "fat32.h"

void print_usage(char *name);

void print_usage(char *name){
	printf("Usage: %s img_file\n", name);
}

int main(int argc, char **argv){

	fat32_ctx_t *fat32_ctx;

	if(argc != 2){
		print_usage(argv[0]);
		goto exit;
	}

	if(fat32_init() != FAT32_SUCCESS){
		perror("fat32_init");
		goto exit;
	}

	if(!(fat32_ctx = fat32_load_image(argv[1], O_RDONLY))){
		perror("fat32_load_image");
		goto exit;
	}

	exit: return 0;
}

