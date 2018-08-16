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
#include <errno.h>

#include "fat32.h"

#define MOUNT_IMG_MASK	(O_RDONLY | O_WRONLY | O_RDWR)

inline static int fat32_read_img(fat32_ctx_t *ctx, uint32_t off, void *buf, uint32_t size){

	assert(ctx);
	assert(buf);

	if(lseek(ctx->img_fd,  off, SEEK_SET) < 0)
		return errno;

	if(read(ctx->img_fd, buf, size) != size)
		return errno;

	return FAT32_SUCCESS;
}

inline static uint32_t fat32_get_table_entry(fat32_ctx_t *ctx, uint32_t cluster){
	return *(ctx->fat + cluster);
}

inline  static uint32_t fat32_read_cluster(fat32_ctx_t *ctx, uint32_t cluster, void *buf){
	uint32_t cluster_offset = ctx->root_dir_off + (cluster - FAT32_ROOT_FS_CLUSTER_CNT) * ctx->cluster_size;

	return fat32_read_img(ctx, cluster_offset, buf, ctx->cluster_size);
}

static uint8_t calc_check_sum(uint8_t * dir_entry)
{
	int i;
	uint8_t sum = 0;
	for(i=0;i < SHRT_FILE_NAME_LEN; i++){
		sum = ((sum & 1) << 7) + (sum >> 1) + dir_entry[i];
	}
	return sum;
}

static int lfn_entry_to_name(fat32_lfn_entry_t *entry,char *lfname,int *index)
{
	int i;
	for(i = 0; i <= 8; i += 2){
		if(entry->fname0[i] == 0)
			return 1;
		lfname[*index] = entry->fname0[i];
		(*index)++;
	}
	for(i = 0; i <= 10; i += 2){
		if(entry->fname1[i] == 0)
                        return 1;
		lfname[*index] = entry->fname1[i];
		(*index)++;
	}
	for(i = 0 ; i <= 2; i += 2){
		if(entry->fname2[i] == 0)
                        return 1;
		lfname[*index] = entry->fname2[i];
		(*index)++;
	}
	return 0;
}

int32_t fat32_init(void){

	return FAT32_SUCCESS;
}

int32_t fat32_release(fat32_ctx_t *ctx){
	if(!ctx)
		return FAT32_ERR;

	free(ctx);
	return FAT32_SUCCESS;
}

fat32_ctx_t *fat32_load_image(char *img_file_name, int mode){
	fat32_ctx_t *ctx = NULL;
	fat32_boot_sector_t *bs;
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
		goto error_cleanup_ctx;

	if(memcmp(ctx->boot_sector.file_system_type, FAT32_LABEL, sizeof(FAT32_LABEL)))
		goto error_cleanup_ctx;

	bs = &ctx->boot_sector;

	ctx->sec_size = bs->bytes_per_sec;
	ctx->cluster_size = bs->sec_per_clus * ctx->sec_size;
	ctx->fs_info_off = bs->fs_info_sector * ctx->sec_size;
	ctx->fat0_off = bs->reserved_sec_cnt * ctx->sec_size;
	if(bs->fat_cnt == MAX_FAT_CNT)
		ctx->fat1_off = ctx->fat0_off + bs->sectors_per_fat * ctx->sec_size;
	ctx->root_dir_off = (bs->reserved_sec_cnt 	+ (bs->fat_cnt * bs->sectors_per_fat)  + (bs->root_dir_strt_cluster - FAT32_ROOT_FS_CLUSTER_CNT)) * ctx->sec_size ;
	ctx->root_dir_start_clus = bs->root_dir_strt_cluster;

	if(!(ctx->fat = calloc(bs->sectors_per_fat, ctx->sec_size)))
		goto error_cleanup_ctx;

	if(fat32_read_img(ctx, ctx->fat0_off, ctx->fat, bs->sectors_per_fat * ctx->sec_size) != FAT32_SUCCESS)
		goto error_cleanup_fat;

	ctx->direntry_in_cluster_cnt = ctx->cluster_size / sizeof(fat32_dir_entry_t);

	return ctx;

	error_cleanup_fat: free(ctx->fat);
	error_cleanup_ctx: free(ctx);
	error_close: close(fd);
	error:
	errno = EINVAL;
	return NULL;
}

int32_t fat32_read_dir(fat32_ctx_t *ctx, fat32_dir_entry_t *dir, fat32_process_dir_entry_t process_fnc){
	int32_t ret = FAT32_SUCCESS;

	uint32_t cur_cluster = (dir)? (((dir->strt_clus_hword) << 16) | dir->strt_clus_lword) : ctx->root_dir_start_clus;

	uint8_t	*cluster_buf = malloc(ctx->cluster_size);
	if((ret = fat32_read_cluster(ctx, cur_cluster, cluster_buf)) != FAT32_SUCCESS )
		goto error;

	fat32_dir_entry_t *dir_entry = (fat32_dir_entry_t *)cluster_buf;
	fat32_dir_entry_t *old_dir_entry;
	int cur_dir_entry_idx = 0;

	while(!((cur_cluster == END_OF_CLUSTER) && (cur_dir_entry_idx == MAX_DIR_ENTRY_CNT + 2) && cur_cluster != END_OF_ROOT_CLUSTER)){
		if((cur_dir_entry_idx >=  ctx->direntry_in_cluster_cnt) && (cur_cluster != END_OF_CLUSTER) && cur_cluster != END_OF_ROOT_CLUSTER){
			cur_cluster = fat32_get_table_entry(ctx, cur_cluster);
			if(cur_cluster == END_OF_CLUSTER || cur_cluster == END_OF_ROOT_CLUSTER)
				break;

			if((ret = fat32_read_cluster(ctx, cur_cluster, cluster_buf)) != FAT32_SUCCESS )
					goto error;

			dir_entry = (fat32_dir_entry_t *)cluster_buf;
			cur_dir_entry_idx = 0;
		}

		old_dir_entry = dir_entry;

		if(!dir_entry->name[0])
			break;

		if(process_fnc){
			if(process_fnc(ctx, &dir_entry) != FAT32_SUCCESS)
				break;
		}

		cur_dir_entry_idx += (((uint8_t *)dir_entry) - ((uint8_t *)old_dir_entry)) / sizeof(fat32_dir_entry_t);
	}

	error: free(cluster_buf);

	return ret;
}

int32_t fat32_get_long_file_name(char *fname, uint8_t **dir_strt){
	fat32_lfn_entry_t *lf_ptr;
	int index = 0;
	int counter = 0;
	uint8_t chk_sum;

	lf_ptr = (fat32_lfn_entry_t *)(*dir_strt);
	counter = lf_ptr->order & LONG_FNAME_MASK;
	while((lf_ptr->order > 1) &&
	     ((lf_ptr->order & LONG_FNAME_MASK) > 1))
		lf_ptr++;
	*dir_strt = (uint8_t *)lf_ptr + DIR_ENTRY_LEN;
	chk_sum = calc_check_sum(*dir_strt);
	while(counter--){
		if(*((uint8_t *)lf_ptr + CHECK_SUM_OFF) == chk_sum){
			if(lfn_entry_to_name(lf_ptr,fname,&index) == 1)
				break;
			lf_ptr--;
		}
	}
	return (index);
}
