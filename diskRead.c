#include "diskRead.h"

int main(int argc, char* argv[]){
  char buf_r[BUFFER_SIZE]; //读取数据的缓冲区大小
  char *dev = "/dev/sdb";
  FILE *fp; // 文件描述符
  int partition_type = -1;

  GPT_HEADER g_header;

  fp = fopen(dev,"r"); 

  if (fp == NULL){
    fprintf(stderr, "open %s failed %s\n",dev,fp);
    return (-1);
  }

  partition_type = read_partition_type(fp); 

  if (partition_type == 0) {
    DOS_PARTITION dos_partitions[DOS_PARTITION_SIZE];
    BPB bpb;

    read_dos_partition_table(fp,dos_partitions);
    //output_dos_partition(dos_partitions);
    read_bpb_info(fp,dos_partitions, &bpb);
    output_bpb(&bpb);
    
  } else if(partition_type == 1) {

    //读取分区表头
    read_gpt_header(fp,&g_header);
    //output_gpt_header(&g_header);

    //读取分区表
    PARTITION partitions[128] = {0};
    read_partition_info(fp, &g_header, partitions);
    //output_partition(partitions);

    //read_first_partition(fp,partitions);
  }

  fclose(fp);
  return 0;
}

// 判断分区类型:返回值0 DOS分区；1 GPT分区
int read_partition_type(FILE* fp) {
  int data = 0;
  if(!fread(&data,sizeof(int),1,fp)){
    return -1;
  }
  if (data != 0) {
    return 0;
  } else {
    return 1;
  }


}

int read_gpt_header(FILE* fp,GPT_HEADER *g_header){

  int offset = find_gpt_header_signature(fp);
  // 重定位文件指针至GPT头的起始位置
  fseek(fp,offset,SEEK_SET);
  memset(g_header,0,sizeof(GPT_HEADER)); // 将heaher中的值全部置为0

  if (!fread(g_header,sizeof(GPT_HEADER),1,fp)) {
    fprintf(stderr,"get gpt header wrong %d",fp);
    return 0;
  }
  return 1;
}

void output_gpt_header(GPT_HEADER *g_header){
  
  fprintf(stdout,"signature is: 0x%llx\n",g_header->signature);
  fprintf(stdout,"version is: %d\n",g_header->version);
  fprintf(stdout,"header_size is: %d\n",g_header->header_size);
  fprintf(stdout,"efi_header_start: %llu\n",g_header->efi_header_start);
  fprintf(stdout,"efi_header_backup: %llu\n",g_header->efi_header_backup);
  fprintf(stdout,"gpt_partition_start: %llu\n",g_header->gpt_partition_start);
  fprintf(stdout,"gpt_partition_end: %llu\n",g_header->gpt_partition_end);
  fprintf(stdout,"gpt start is: %d\n",g_header->partition_start);
  fprintf(stdout,"gpt entry max: %d\n",g_header->partition_entry_max);
  fprintf(stdout,"gpt entry size: %d\n",g_header->partition_entry_size);
}

void hex_output(char* data,int size){
  int m=0;
  int i=0;
  int j=0;

  for (i=0; i<size/16; i++){
    fprintf(stdout,"%08x: ", i*16);
    for (j=0; j< 16; j++) {
      fprintf(stdout,"%02x",(int)data[i*16+j] & 0xFF);
      m++;
      if (m%2==0) {
        fprintf(stdout," ");
        m = 0 ;
      }
    }
    for (j=0; j<16; j++){
      fprintf(stdout,"  ");
      if  (data[i*16 + j] < 0x1F){
        fprintf(stdout,".");
      } else {
        fprintf(stdout,"%c",(int)data[i*16+j] & 0xFF);
      }
    }
    fprintf(stdout,"\n");
    fflush(stdout);
  }
}


int find_gpt_header_signature(FILE* fp){
  int i = 0,offset = 0, size = 512;
  uint64_t signature; // 8Byte
  
  for(;i < 128; i++){
    offset = i * 8;
    fread(&signature,8,1,fp);
    if (signature  == GPT_HEADER_SIGNATURE) {
      return offset;
    }
  }
  return 0;
}

int read_partition_info(FILE* fp,GPT_HEADER *g_header ,PARTITION *partitions){
  uint64_t partition_start_sector = g_header->partition_start;

  // 将文件指针定位至分区表的开头
  fseek(fp, 512 * partition_start_sector,SEEK_SET);
  for (int i = 0; i < 128; i++) {
    if (!fread(partitions + i, sizeof(PARTITION), 1, fp)) {
      fprintf(stderr, "read partition error %d", fp);
      return 0;
    }
  }
  return 1;
}

void output_partition(PARTITION *partitions) {
  PARTITION *partition = NULL;

  for (int i = 0 ; i < 128; i++ ) {
    partition = partitions + i;

    if (partition->partition_start_sector == 0) {
      break;
    }

    fprintf(stdout,"partition type: ");
    for (int j = 0; j < 16; j++){
      fprintf(stdout," %02x ",*(partition->partition_id + j));
    }
    fprintf(stdout,"\n\rpartition id: ");
    for (int j = 0; j < 16; j++){
      fprintf(stdout," %02x ",*(partition->partition_type + j));
    }
    printf("\n");
    fprintf(stdout,"partition start sector: %llu\n", partition->partition_start_sector);
    fprintf(stdout,"partition end sector: %llu\n", partition->partition_end_sector);
    fprintf(stdout,"partition type: 0x%llx\n", partition->partition_flag);
  }
}

int read_first_partition(FILE* fp, PARTITION *partition){
  char data[1024] = {0};

  // 定位文件指针至第一个扇区
  fseek(fp,partition->partition_start_sector * 512,SEEK_SET);
  fread(data, 512,2 ,fp);
  hex_output(data,1024);
  return 0;
}


int read_dos_partition_table(FILE* fp,DOS_PARTITION *d_partitions) {
  fseek(fp,DOS_PARTITION_OFFSET,SEEK_SET);
  for(int i=0;i < DOS_PARTITION_SIZE; i++ ){
    memset(d_partitions+i,0,sizeof(DOS_PARTITION));
    if (!fread(d_partitions+i,sizeof(DOS_PARTITION),1,fp)) {
      fprintf(stdout,"read dos partition error %x",fp);
      return 0;
    }
  }
  return 1;
}

void output_dos_partition(DOS_PARTITION *d_partitions) {
  DOS_PARTITION *d_partition = NULL;
  for (int i=0; i < DOS_PARTITION_SIZE; i++) {
    d_partition = d_partitions + i;
    fprintf(stdout,"boot flag: 0x%02x\n",d_partition->boot_flag);
    fprintf(stdout,"start header: %02d\n",d_partition->s_header);
    fprintf(stdout,"start sector: %02d\n",d_partition->s_sector_cylinder & 0x3F);
    fprintf(stdout,"start cylinder: %02d\n",(d_partition->s_sector_cylinder & 0xFFC0) >> 6);
    fprintf(stdout,"partition type: 0x%02x\n",d_partition->partition_type);
    fprintf(stdout,"end header: 0x%02d\n",d_partition->e_header);
    fprintf(stdout,"end sector: %02d\n",d_partition->e_sector_cylinder & 0x3F);
    fprintf(stdout,"end cylinder: %02d\n",(d_partition->e_sector_cylinder & 0xFFC0) >> 6);
    fprintf(stdout,"preceding sectors: %02d\n",d_partition->preceding_sectors);
    fprintf(stdout,"end cylinder: %02d\n",d_partition->total_sectors);
    printf("\n");
  }
}

// 读取分区表
int read_bpb_info(FILE* fp,DOS_PARTITION *d_partition,BPB *bpb) {
  int preceding_sectors = d_partition->preceding_sectors;
  // 将文件指针定位至bpb的起始位置
  fseek(fp,preceding_sectors * 512,SEEK_SET);
  /*
  char data[79] = {'\0'};
  hex_output(data,79);
  fread(data,sizeof(data),1,fp);

  hex_output(data,79);
  */
  memset(bpb,0,sizeof(BPB));
  if(!fread(bpb,sizeof(BPB),1,fp)) {
    fprintf(stdout,"read bpb error %x",fp);
    return -1; 
  }
  return 0;
}

void output_bpb(BPB *bpb) {
  fprintf(stdout,"bytes per sector: %x\n", bpb->sector_bytes);
  fprintf(stdout,"sectors per cluster: %x \n", bpb->cluster_sectors);
  fprintf(stdout,"reserved sectors: %d \n", bpb->reserved_sectors);
  fprintf(stdout,"fat tables: %x\n", bpb->fat_tables);
  fprintf(stdout,"ss %d",sizeof(BPB));
  /*
  fprintf(stdout,"medium descriptor %d \n", bpb->medium_descriptor);
  fprintf(stdout,"sectors per cylinder: %d \n", bpb->cylinder_sectors);
  fprintf(stdout,"headers: %d \n", bpb->headers);
  fprintf(stdout,"hidden sectors: %d \n", bpb->hidden_sectors);
  fprintf(stdout,"flag: %d \n", bpb->flag);
  fprintf(stdout,"version: %d \n", bpb->version);
  fprintf(stdout,"root cluseter: %d \n", bpb->root_cluster);
  fprintf(stdout,"filesystem sectors: %d \n", bpb->filesystem_sectors);
  fprintf(stdout,"dbr backup sectors: %d \n", bpb->dbr_backup_sectors);
  fprintf(stdout,"extend_flag: %d \n", bpb->extend_flag);
  fprintf(stdout,"volumn num: %d \n", bpb->volumn_num);
  */
}
