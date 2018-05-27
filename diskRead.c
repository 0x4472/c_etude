#include "diskRead.h"

int main(int argc, char* argv[]){
  char buf_r[BUFFER_SIZE]; //读取数据的缓冲区大小
  char *dev = "/dev/sda";
  int offset = 0;

  FILE *fp; // 文件描述符

  fp = fopen(dev,"r"); 
  if (fp == NULL){
    fprintf(stderr, "open %s failed %s\n",dev,fp);
    return (-1);
  }
  offset = find_gpt_header_signature(fp);
  
  gpt_header g_header;
  // 将文件指针放置在512
  fseek(fp,offset,SEEK_SET);
  print_gpt_header(fp,&g_header);

  fprintf(stdout,"%d",offset);
  putchar('\n');

  fclose(fp);
  return 0;
}

int print_gpt_header(FILE* fp,gpt_header *g_header){
  memset(g_header,0,sizeof(gpt_header)); // 将heaher中的值全部值为0

  if (!fread(g_header,sizeof(gpt_header),1,fp)) {
    fprintf(stderr,"get gpt header wrong %d",fp);
  }
  fprintf(stdout,"version is = %d\n",g_header->version);
  fprintf(stdout,"header_size is = %d\n",g_header->header_size);
  return 0;
}

void format_output(char* data,int size){
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
