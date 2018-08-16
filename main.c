#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "fat32.h"

void print_usage(char *name);

void print_usage(char *name){
	printf("Usage: %s img_file\n", name);
}

static int process_dir(fat32_ctx_t *ctx, fat32_dir_entry_t **entry){
	int name_len = 0;
	char name[LONG_FILE_NAME_LEN];
	memset(name,SPACE_VAL, sizeof(name));
	int i;
	static int deep = 0;
	int ret = FAT32_SUCCESS;

	deep++;

	name_len = fat32_get_long_file_name(name, (uint8_t **)entry);

	if(name_len > 0){
		name[name_len] = 0;
		for(i = 0; i < deep; i++){
			printf(" |");
		}
		printf("-%s\n", name);
		if((*entry)->attr & ATTR_DIR){
			fat32_dir_entry_t *new_entry = *entry;
			ret = fat32_read_dir(ctx, new_entry, process_dir);
		}
	}
	deep--;
	return ret;
}

int main(int argc, char **argv){

	fat32_ctx_t *fat32_ctx;
	char volume_label[VOL_LABEL_LEN +1];
	memset(volume_label, 0, sizeof(volume_label));

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

	memcpy(volume_label, fat32_ctx->boot_sector.volume_label, sizeof(fat32_ctx->boot_sector.volume_label));

	printf("Image file name: %s, volume label %s, cluster size %d\n", argv[1], volume_label, fat32_ctx->cluster_size);

	fat32_read_dir(fat32_ctx, NULL, process_dir);

	if(fat32_release(fat32_ctx) != FAT32_SUCCESS)
		perror("fat32_release");

	exit: return 0;
}

