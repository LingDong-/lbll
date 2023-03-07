#include <locale.h>
#include "memstream_rw.c"
#define LBLL_USER_IMPL_LIB_C
#include "../lbll.h"

uint8_t pxbuf[1024] = {0};

short qdr[16] = {
  0x0020,0x2596,0x2598,0x258C,
  0x2597,0x2584,0x259A,0x2599,
  0x259D,0x259E,0x2580,0x259B,
  0x2590,0x259F,0x259C,0x2588
};

short ddr[4] = {
  0x0020,0x2584,0x2580,0x2588
};

static const uint8_t FONT[] = {0,0,0,0,125,0,96,0,96,127,20,127,50,107,38,39,8,115,62,107,42,0,96,0,28,34,65,65,34,28,107,62,107,8,62,8,2,3,0,8,8,8,3,3,0,3,28,96,62,65,62,33,127,1,35,69,57,34,73,54,60,4,127,122,73,78,62,73,38,64,79,112,54,73,54,50,73,62,0,20,0,2,20,0,8,20,34,20,20,20,34,20,8,32,77,48,63,65,125,127,80,127,127,73,54,62,65,34,127,65,62,127,73,65,127,72,72,62,65,46,127,8,127,65,127,65,2,1,126,127,8,119,127,1,1,127,62,127,127,64,127,127,65,127,127,72,48,62,69,63,127,72,55,50,73,38,64,127,64,127,1,127,124,3,124,127,14,127,119,8,119,112,15,112,71,73,113,127,65,65,96,28,3,65,65,127,32,64,32,1,1,1,64,32,0};

void c_px(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  if (x < 0 || x >= 128) return;
  if (y < 0 || y >= 64 ) return;
  y = 63-y;
  int row = y / 8;
  pxbuf[ row * 128 + x] |= (1 << (y % 8));
}
void c_unpx(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  if (x < 0 || x >= 128) return;
  if (y < 0 || y >= 64 ) return;
  y = 63-y;
  int row = y / 8;
  pxbuf[ row * 128 + x] &= ~(1 << (y % 8));
}

void c_text() {
  int col = (int)lbll_pop();
  int row = (int)lbll_pop();
  int n   = (int)lbll_pop();
  char buf[n];
  int pointer;
  for (int i= 0; i < n; i++){
    buf[n-i-1] = (int)lbll_pop();
  }
  for (int i= 0; i < n; i++){
    char chr = buf[i];
    if (chr == '\0')
      break;
    if (chr >= 'a'){
      chr -= ('a'-'A');
    }
    pointer = chr-' ';
    pxbuf[row*128+col+4*i  ] = FONT[3*pointer  ];
    pxbuf[row*128+col+4*i+1] = FONT[3*pointer+1];
    pxbuf[row*128+col+4*i+2] = FONT[3*pointer+2];
  }
}


void printscreen_qdr(){
  printf("%lc",0x2554);
  for (int j = 0; j < 64; j++){
    printf("%lc",0x2550);
  }
  printf("%lc\n",0x2557);
  for (int i = 0; i < 32; i++){
    printf("%lc",0x2551);
    int row = (7-i/4);
    for (int j = 0; j < 64; j++){
      int b0 = (pxbuf[row*128+j*2  ] >> ((3-i%4)*2  )) & 1;
      int b1 = (pxbuf[row*128+j*2  ] >> ((3-i%4)*2+1)) & 1;
      int b2 = (pxbuf[row*128+j*2+1] >> ((3-i%4)*2  )) & 1;
      int b3 = (pxbuf[row*128+j*2+1] >> ((3-i%4)*2+1)) & 1;
      int b = (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
      printf("%lc",qdr[b]);
    }
    printf("%lc\n",0x2551);
  }
  printf("%lc",0x255a);
  for (int j = 0; j < 64; j++){
    printf("%lc",0x2550);
  }
  printf("%lc\n",0x255d);
}

void printscreen_ddr(){
  printf("%lc",0x2554);
  for (int j = 0; j < 128; j++){
    printf("%lc",0x2550);
  }
  printf("%lc\n",0x2557);
  for (int i = 0; i < 32; i++){
    printf("%lc",0x2551);
    int row = (7-i/4);
    for (int j = 0; j < 128; j++){
      int b0 = (pxbuf[row*128+j] >> ((3-i%4)*2  )) & 1;
      int b1 = (pxbuf[row*128+j] >> ((3-i%4)*2+1)) & 1;
      int b = (b1 << 1) | b0;
      printf("%lc",ddr[b]);
    }
    printf("%lc\n",0x2551);
  }
  printf("%lc",0x255a);
  for (int j = 0; j < 128; j++){
    printf("%lc",0x2550);
  }
  printf("%lc\n",0x255d);
}


void printscreen_brl(){
  printf("%lc",0x2554);
  for (int j = 0; j < 64; j++){
    printf("%lc",0x2550);
  }
  printf("%lc\n",0x2557);
  for (int i = 0; i < 16; i++){
    printf("%lc",0x2551);
    int row = (7-i/2);
    for (int j = 0; j < 64; j++){
      //03
      //14
      //25
      //67
      int b6 = (pxbuf[row*128+j*2  ] >> ((1-i%2)*4  )) & 1;//
      int b2 = (pxbuf[row*128+j*2  ] >> ((1-i%2)*4+1)) & 1;//
      int b1 = (pxbuf[row*128+j*2  ] >> ((1-i%2)*4+2)) & 1;//
      int b0 = (pxbuf[row*128+j*2  ] >> ((1-i%2)*4+3)) & 1;//

      int b7 = (pxbuf[row*128+j*2+1] >> ((1-i%2)*4  )) & 1;
      int b5 = (pxbuf[row*128+j*2+1] >> ((1-i%2)*4+1)) & 1;
      int b4 = (pxbuf[row*128+j*2+1] >> ((1-i%2)*4+2)) & 1;
      int b3 = (pxbuf[row*128+j*2+1] >> ((1-i%2)*4+3)) & 1;//

      int b = (b7<<7)|(b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|b0;
      printf("%lc",0x2800+b);
    }
    printf("%lc\n",0x2551);
  }
  printf("%lc",0x255a);
  for (int j = 0; j < 64; j++){
    printf("%lc",0x2550);
  }
  printf("%lc\n",0x255d);
}

void c_show() {
  // printscreen_brl();
}


int main(int argc, char** argv){
  int err;
  setlocale(LC_ALL,"");

  lbll_init_stdlib();

  lbll_reg_cfun("px"   ,2, c_px );
  lbll_reg_cfun("unpx" ,2, c_unpx);
  lbll_reg_cfun("text" ,2, c_text);
  lbll_reg_cfun("show" ,0, c_show);

  C_FILE* fd = C_fopen(argv[1],"r");

  C_FILE* fo = C_open_memstream_rw(NULL,NULL);

  err = lbll_parse(fd,fo);

  C_fclose(fd);

  if (err){
    C_fclose(fo);
    return err;
  }

  C_fseek(fo,0,SEEK_END);

  printf("parsing used: %ld B\n",C_ftell(fo));

  C_fseek(fo,0,SEEK_SET);

  lbll_print_instrs(fo);

  C_fseek(fo,0,SEEK_SET);

  lbll_stack = C_open_memstream_rw(NULL,NULL);
  lbll_frame = C_open_memstream_rw(NULL,NULL);

  err = lbll_run(fo);

  lbll_print_stack();
  lbll_print_frame();
  C_fseek(lbll_stack,0,SEEK_END);
  C_fseek(lbll_frame,0,SEEK_END);

  printf("stack used: %ld B\n",C_ftell(lbll_stack));
  printf("frame used: %ld B\n",C_ftell(lbll_frame));

  printscreen_brl();

  C_fclose(fo);
  C_fclose(lbll_stack);
  C_fclose(lbll_frame);

  return err;
}