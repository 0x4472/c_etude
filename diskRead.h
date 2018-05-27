#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define READ_BS     512

#define GPT_HEADER_SIGNATURE 0x5452415020494645


typedef struct {
  uint64_t signature;
  int version;
  int header_size;
  uint32_t crc_header;
  int reserved;
  uint64_t gpt_header_start;
  uint64_t gpt_header_backup;
  uint64_t gpt_partition_start; // GPT分区区域的起始扇区号
  uint64_t gpt_partition_end;   // GPT分区区域的结束扇区号
  uint8_t  disk_guid[16];
  uint64_t gpt_start;           // GPT分区表的起始扇区
  uint32_t gpt_entry_count;     // GPT分区表总项数
  uint32_t gpt_entry_size;      // GPT每个分区表占用的字节数
  uint32_t crc;
  int reser[105];               // int 4 Byte
} gpt_header;

void format_output(char* data,int size);

int find_gpt_header_signature(FILE* fp);
int print_gpt_header(FILE* fp,gpt_header *gpt_header);

