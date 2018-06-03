#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define READ_BS     512

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
