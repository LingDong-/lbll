#include "memstream_rw.c"
#define LBLL_USER_IMPL_LIB_C
#include "../lbll.h"

int main(int argc, char** argv){
  lbll_init_stdlib();

  C_FILE* fd = C_fopen(argv[1],"r");

  C_FILE* fo = C_open_memstream_rw(NULL,NULL);

  lbll_parse(fd,fo);

  C_fseek(fo,0,SEEK_END);

  printf("parsing used: %ld B\n",C_ftell(fo));

  C_fseek(fo,0,SEEK_SET);

  lbll_print_instrs(fo);

  C_fseek(fo,0,SEEK_SET);

  lbll_stack = C_open_memstream_rw(NULL,NULL);
  lbll_frame = C_open_memstream_rw(NULL,NULL);

  lbll_run(fo);

  lbll_print_stack();
  lbll_print_frame();
  C_fseek(lbll_stack,0,SEEK_END);
  C_fseek(lbll_frame,0,SEEK_END);

  printf("stack used: %ld B\n",C_ftell(lbll_stack));
  printf("frame used: %ld B\n",C_ftell(lbll_frame));

}