/*
 * fat32.h
 *
 *  Created on: Aug 15, 2018
 *      Author: snedosiekov
 */

#ifndef FAT32_H_
#define FAT32_H_

#define FAT32_LABEL					"FAT32   "

#define FAT32_ROOT_FS_CLUSTER_CNT	2

#define END_OF_CLUSTER           0x0FFFFFFF
#define END_OF_ROOT_CLUSTER      0X0FFFFFF8
#define LOW_CLUSWORD_MASK        0x0000FFFF
#define HIGH_CLUSWORD_MASK       0xFFFF0000

#define LONG_FNAME_MASK          0x0F

#define SHRT_FILE_NAME_LEN       11
#define FILE_SYS_TYPE_LENGTH     8
#define CHECK_SUM_OFF            13
#define FLEN_NAME_LEN       	 8
#define FILE_EXT_LEN       		 3
#define LONG_FILE_NAME_LEN       255

#define LFN_FIRST_SET_LEN        10
#define LFN_SEC_SET_LEN          12
#define LFN_THIRD_SET_LEN        4
#define LFN_EMPTY_LEN            2

#define FULL_SHRT_NAME_LEN       13
#define VOL_ID_LEN               4
#define VOL_LABEL_LEN            11
#define RESERV_LEN               12
#define FS_VER_LEN               2
#define OEM_NAME_LEN             8
#define JUMP_INS_LEN             3

#define LFN_FIRST_SET_CNT        5
#define LFN_SEC_SET_CNT          6
#define LFN_THIRD_SET_CNT        2
#define MAX_DIR_ENTRY_CNT        16
#define MAX_FAT_CNT              2

#define SPACE_VAL                32

#define ATTR_READ                0x01
#define ATTR_HIDDEN              0x02
#define ATTR_SYSTEM              0x04
#define ATTR_VOL_LABEL           0x08
#define ATTR_DIR                 0x10
#define ATTR_ARCHIVE             0x20
#define ATTR_LONG_FNAME          0x0F

//error code defs
#define FAT32_SUCCESS		0
#define FAT32_ERR			-1

#define DIR_ENTRY_LEN            sizeof(fat32_dir_entry_t)

#define FAT32_CLASTER(dir) (((dir->strt_clus_hword) << 16) | dir->strt_clus_lword)

//correct for 32-bit arch

typedef unsigned char 			uint8_t;
typedef unsigned short int 		uint16_t;
typedef short int 				int16_t;
typedef unsigned int 			uint32_t;
typedef int 					int32_t;
typedef unsigned long long int 	uint64_t;

typedef struct{
	uint8_t   jump[JUMP_INS_LEN];
	uint8_t   fs_oem_name[OEM_NAME_LEN];
	uint16_t  bytes_per_sec;
	uint8_t   sec_per_clus;
	uint16_t  reserved_sec_cnt;
	uint8_t   fat_cnt;
	uint16_t  root_dir_max_cnt;
	uint16_t  tot_sectors;
	uint8_t   media_desc;
	uint16_t  sec_per_fat_fat16;
	uint16_t  sec_per_track;
	uint16_t  number_of_heads;
	uint32_t  hidden_sec_cnt;
	uint32_t  tol_sector_cnt;
	uint32_t  sectors_per_fat;
	uint16_t  ext_flags;
	uint8_t   fs_version[FS_VER_LEN];
	uint32_t  root_dir_strt_cluster;
	uint16_t  fs_info_sector;
	uint16_t  backup_boot_sector;
	uint8_t   reserved[RESERV_LEN];
	uint8_t   drive_number;
	uint8_t   reserved1;
	uint8_t   boot_sig;
	uint8_t   volume_id[VOL_ID_LEN];
	uint8_t   volume_label[VOL_LABEL_LEN];
	uint8_t   file_system_type[FILE_SYS_TYPE_LENGTH];
}__attribute__((__packed__)) fat32_boot_sector_t;

typedef struct{
	uint8_t  name[FLEN_NAME_LEN];
	uint8_t  extn[FILE_EXT_LEN];
	uint8_t  attr;
	uint8_t  reserved;
	uint8_t  crt_time_tenth;
	uint16_t crt_time;
	uint16_t crt_date;
	uint16_t lst_access_date;
	uint16_t strt_clus_hword;
	uint16_t lst_mod_time;
	uint16_t lst_mod_date;
	uint16_t strt_clus_lword;
	uint32_t size;
}__attribute__((__packed__)) fat32_dir_entry_t;

typedef struct{
	uint8_t order;
	uint8_t fname0[LFN_FIRST_SET_LEN];
	uint8_t flag;
	uint8_t reserved;
	uint8_t chksum;
	uint8_t fname1[LFN_SEC_SET_LEN];
	uint8_t empty[LFN_EMPTY_LEN];
	uint8_t fname2[LFN_THIRD_SET_LEN];
}__attribute__((__packed__)) fat32_lfn_entry_t;

typedef struct{
	char      *oem_name;
	char      *label;
	uint32_t  sec_size;
	uint32_t  cluster_size;
	uint32_t  root_dir_start_clus;
	uint32_t  direntry_in_cluster_cnt;
	int		  img_fd;
	fat32_boot_sector_t boot_sector;
	uint32_t  fat0_off;
	uint32_t  fat1_off;
	uint32_t  fs_info_off;
	uint32_t  root_dir_off;
	uint32_t  *fat;
}fat32_ctx_t;

typedef int (*fat32_process_dir_entry_t)(fat32_ctx_t *ctx, fat32_dir_entry_t **entry);

int32_t fat32_init(void);

/*
 * @brief   load fs image
 *
 * @param img_file_name filename to load
 * @param mode	   		mode to open (O_RDONLY | O_WRONLY | O_RDWR)
 *
 * @return 				allocated fs control structure
 */
fat32_ctx_t *fat32_load_image(char *img_file_name, int mode);

/*
 * @brief   read directory
 *
 * @param ctx 			fat control structure
 * @param dir			directory to read (NULL - read root directory)
 * @param process_fnc  	function to process each directory entry
 *
 * @return 				FAT32_SUCCESS on success or error code
 */
int32_t fat32_read_dir(fat32_ctx_t *ctx, fat32_dir_entry_t *dir, fat32_process_dir_entry_t process_fnc);

/*
 * @brief   get long file name of directory entry
 *
 * @param fname 		pointer to store file name
 * @param dir_entry		pointer to pointer of dir entry, modify while parsing name
 * @return  			length of file name, 0 if no file name found
 */
int32_t fat32_get_long_file_name(char *fname, uint8_t **dir_entry);

/*
 * @brief   release fs control structure
 *
 * @param ctx 	fat control structure
 *
 * @return 		FAT32_SUCCESS on success or error code
 */
int32_t fat32_release(fat32_ctx_t *ctx);

#endif /* FAT32_H_ */
