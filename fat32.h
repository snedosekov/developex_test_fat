/*
 * fat32.h
 *
 *  Created on: Aug 15, 2018
 *      Author: snedosiekov
 */

#ifndef FAT32_H_
#define FAT32_H_

#define END_OF_CLUSTER           0x0FFFFFFF
#define END_OF_ROOT_CLUSTER      0X0FFFFFF8
#define LOW_CLUSWORD_MASK        0x0000FFFF
#define HIGH_CLUSWORD_MASK       0xFFFF0000

#define LFN_END_MARK             0xFFFF
#define LFN_TERM_MARK            0x0000
#define LONG_FNAME_MASK          0x0F

#define LAST_ORD_FIELD_SEQ       0x40
#define BYTES_PER_CLUSTER_ENTRY  4
#define LFN_LEN_PER_ENTRY        13
#define SEC_PER_FAT              36

#define FILE_SYS_TYPE_OFF        82
#define BYTES_PER_SEC_OFF        11
#define SEC_PER_CLUS_OFF         13
#define RES_SEC_CNT_OFF          14
#define FAT_CNT_OFF              16
#define TOT_SEC_CNT_OFF          32
#define ROOT_DIR_STRT_CLUS_OFF   44
#define FS_INFOSECTOR_OFF        48
#define BACKUP_BOOT_SEC_OFF      50
#define NXT_FREE_CLUS_OFF        492
#define STRT_CLUS_LOW_OFF        26
#define STRT_CLUS_HIGH_OFF       20
#define FILE_SIZE_OFF            28
#define ATTR_OFF                 11
#define FNAME_EXTN_SEP_OFF       6
#define FNAME_SEQ_NUM_OFF        7
#define LFN_FIRST_OFF            0x01
#define LFN_SIXTH_OFF            0x0E
#define LFN_TWELVETH_OFF         0x1C

#define SHRT_FILE_NAME_LEN       11
#define FILE_SYS_TYPE_LENGTH     8
#define FILE_STAT_LEN            21
#define CHECK_SUM_OFF            13
#define FLEN_NAME_LEN       	 8
#define FILE_EXT_LEN       		 3
#define LONG_FILE_NAME_LEN       255

#define LFN_FIRST_SET_LEN        10
#define LFN_SEC_SET_LEN          12
#define LFN_THIRD_SET_LEN        4
#define LFN_EMPTY_LEN            2

#define FULL_SHRT_NAME_LEN       13
#define DIR_ENTRY_LEN            32
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

#define FILE_READ                0x01
#define FILE_WRITE               0X02
#define FILE_CREATE_NEW          0x04
#define FILE_CREATE_ALWAYS       0x08
#define FILE_APPEND              0x10

#define ATTR_READ                0x01
#define ATTR_HIDDEN              0x02
#define ATTR_SYSTEM              0x04
#define ATTR_VOL_LABEL           0x08
#define ATTR_DIR                 0x10
#define ATTR_ARCHIVE             0x20
#define ATTR_LONG_FNAME          0x0F

#define FREE_DIR_ENTRY           0x00
#define DEL_DIR_ENTRY            0xE5
#define DOT_DIR_ENTRY            0x2E

#define ASCII_DIFF               32
#define FILE_SEEK_SET            0
#define FILE_SEEK_CUR            1
#define FILE_SEEK_END            2

//error code defs
#define FAT32_SUCCESS		0

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
	uint8_t ord_field;
	uint8_t fname0_4[LFN_FIRST_SET_LEN];
	uint8_t flag;
	uint8_t reserved;
	uint8_t chksum;
	uint8_t fname6_11[LFN_SEC_SET_LEN];
	uint8_t empty[LFN_EMPTY_LEN];
	uint8_t fname12_13[LFN_THIRD_SET_LEN];
}__attribute__((__packed__)) fat32_lfn_entry_t;

struct fat32_f_info_t;
typedef struct  fat32_f_info_t  fat32_file_info_t;

struct  fat32_f_info_t{
	int32_t fd;
	fat32_dir_entry_t *entry;
	const char *file_path;
	uint32_t strt_cluster;
	uint32_t cur_cluster;
	uint16_t cur_offset;
	uint32_t file_size;
	uint8_t  mode;
	uint32_t bytes_traversed;
	uint8_t *dir_entry_offset;
	fat32_file_info_t *next;
};

typedef struct{
	char name[FULL_SHRT_NAME_LEN];
	const char *lname;
	uint32_t size;
	uint16_t time;
	uint16_t date;
	uint8_t attr;
}  fat32_file_stat_t;


typedef struct{
	char      *oem_name;
	char      *label;
	uint16_t  bytes_per_sec;
	uint8_t   sec_per_clus;
	uint16_t  reserved_sec_cnt;
	uint8_t   fat_cnt;
	uint16_t  root_dir_max_cnt;
	uint16_t  tot_sectors;
	uint32_t  hidden_sec_cnt;
	uint32_t  tol_sector_cnt;
	uint32_t  sectors_per_fat;
	uint16_t  ext_flags;
	uint32_t  root_dir_strt_cluster;
	uint16_t  fs_info_sector;
	uint16_t  backup_boot_sector;
	uint8_t   reserved[RESERV_LEN];
	int		  img_fd;
	fat32_boot_sector_t boot_sector;
	uint32_t  fat0_off;
	uint32_t  fat1_off;
	uint32_t  root_dir_off;
	uint32_t  data_off;
}fat32_ctx_t;

int32_t fat32_init(void);

fat32_ctx_t *fat32_load_image(char *img_file_name, int mode);
/*
 * read directory
 *
 * @ctx 	fat control structure
 * @dir		directory to read (NULL - read root directory)
 * @result  address of pointer to store result
 */
int32_t fat32_read_dir(fat32_ctx_t *ctx, fat32_file_info_t *dir, fat32_file_info_t **result);


#endif /* FAT32_H_ */
