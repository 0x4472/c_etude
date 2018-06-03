#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define READ_BS     512

#define DOS_PARTITION_OFFSET 446
#define DOS_PARTITION_SIZE 4

#define GPT_HEADER_SIGNATURE 0x5452415020494645

// GPT头
typedef struct {
  uint64_t signature;
  int version;
  int header_size;
  uint32_t crc_header;
  int reserved;
  uint64_t efi_header_start;    // EFI信息区的起始扇区
  uint64_t efi_header_backup;   // EFI信息区的备份扇区号
  uint64_t gpt_partition_start; // GPT分区区域的起始扇区号
  uint64_t gpt_partition_end;   // GPT分区区域的结束扇区号
  uint8_t  disk_guid[16];
  uint64_t partition_start;     // 分区表的起始扇区
  uint32_t partition_entry_max; // 分区表总项数,最多可以有多少个分区表
  uint32_t partition_entry_size;// 每个分区表占用的字节数
  uint32_t crc;
} GPT_HEADER;

// GPT分区表
typedef struct{
  uint8_t  partition_type[16];
  uint8_t  partition_id[16];
  uint64_t partition_start_sector;
  uint64_t partition_end_sector;
  uint64_t partition_flag;
  uint8_t partition_name[72];
} PARTITION;

// DOS分区表结构
typedef struct {
  uint8_t boot_flag; // 引导标志，是否为活动分区
  uint8_t s_header; // 起始磁头
  uint16_t s_sector_cylinder; // 起始扇区和柱面
  uint8_t partition_type; //分区类型
  uint8_t e_header; // 结束磁头
  uint16_t e_sector_cylinder; // 结束磁头和柱面
  uint32_t preceding_sectors; // 本分区之前使用的扇区数
  uint32_t total_sectors; // 本分区的总扇区数
} DOS_PARTITION;


typedef struct {
  uint8_t no_used[11];
  uint16_t sector_bytes; // 每扇区字节数
  uint8_t cluster_sectors; // 每簇扇区数
  uint16_t reserved_sectors; // dos保留扇区
  uint8_t fat_tables; // fat 表的个数
  uint32_t reserve_one;
  uint8_t medium_descriptor; // 介质描述符
  uint16_t reserve_two;
  uint16_t cylinder_sectors; // 每磁道扇区数
  uint16_t headers; // 磁头数
  uint32_t hidden_sectors; // 隐藏扇区数
  uint32_t partition_sectors; // 该分区的扇区总数
  uint32_t fat_sectors; // 每FAT扇区数
  uint16_t flag; // 标记
  uint16_t version; // 版本
  uint32_t root_cluster; // 根目录首簇号
  uint16_t filesystem_sectors; // 文件系统扇区号
  uint16_t dbr_backup_sectors; // DBR备份扇区号
  uint8_t reverse_thr[12] ;
  uint8_t bios_driver; // BIOS驱动器号
  uint8_t reserve_four;
  uint8_t extend_flag; //扩展引导标记
  uint32_t volumn_num; //卷序列号 
  uint8_t volumn_name[11]; // 卷标
  uint8_t file_system_type[8]; //文件系统类型
} BPB;



// 判断分区类型:返回值0 DOS分区；1 GPT分区
int read_partition_type(FILE *fp);

// 辅助输出
void hex_output(char* data,int size);

// gpt头相关
int find_gpt_header_signature(FILE* fp);
int read_gpt_header(FILE* fp,GPT_HEADER *gpt_header);
void output_gpt_header(GPT_HEADER *g_header);

// gpt分区表相关
int read_partition_info(FILE* fp,GPT_HEADER *g_header ,PARTITION *partition);
void output_partition(PARTITION *partition);

// 读取分区中的文件
int read_first_partition(FILE* fp, PARTITION *partition);

// 读取DOS分区表
int read_dos_partition_table(FILE* fp,DOS_PARTITION *d_partitions);
void output_dos_partition(DOS_PARTITION *d_partitions);

// 读取FATS文件系统中的BPB信息
int read_bpb_info(FILE* fp,DOS_PARTITION *d_partition,BPB *bpb);
void output_bpb(BPB *bpb);
