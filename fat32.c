/*
 * fat32.c
 *
 *  Created on: Aug 15, 2018
 *      Author: snedosiekov
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include "fat32.h"

#define MOUNT_IMG_MASK	(O_RDONLY | O_WRONLY | O_RDWR)

int32_t fat32_init(void){

	return FAT32_SUCCESS;
}

fat32_ctx_t *fat32_load_image(char *img_file_name, int mode){
	fat32_ctx_t *ctx = NULL;
	int fd;

	if(!img_file_name)
		goto error;

	mode &= MOUNT_IMG_MASK;

	if(!(fd = open(img_file_name, mode)))
		goto error;

	if(!(ctx =calloc(1, sizeof(fat32_ctx_t))))
		goto error_close;

	ctx->img_fd = fd;

	if(read(fd, &ctx->boot_sector, sizeof(fat32_boot_sector_t)) != sizeof(fat32_boot_sector_t))
		goto error_cleanup;

	return ctx;

	error_cleanup: free(ctx);
	error_close: close(fd);
	error: return NULL;


}

int32_t fat32_read_dir(fat32_ctx_t *ctx, fat32_file_info_t *dir, fat32_file_info_t **result){
	int32_t ret = FAT32_SUCCESS;

	return ret;
}
