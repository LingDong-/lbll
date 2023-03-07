// #include <emscripten.h>
#include "../../emsdk/upstream/emscripten/cache/sysroot/include/emscripten.h"

#include "../cli/memstream_rw.c"

int cons = 0;

EM_JS(void, js_log, (int cons, const char* s, int len), {
  if (!window.logbuf){
    window.logbuf = "";
  }
  let u = UTF8ToString(s,len);

  if (u.endsWith("\n")){
    window.logbuf += u.slice(0,-1);
    console.log(window.logbuf);
    window.logbuf = "";
  }else{
    window.logbuf += u;
  }

  if (cons){
    let out = document.getElementById("output");
    if (out) out.value += u;
  }else{
    let out = document.getElementById("log");
    if (log) out.value += u;
  }
});

#undef C_printf

char C_printbuf[64];
#define C_printf(...) {\
  size_t bufsz_ = snprintf(NULL, 0, __VA_ARGS__);\
  if (bufsz_ < 64){\
    C_sprintf(C_printbuf, __VA_ARGS__);\
    js_log(cons,C_printbuf,bufsz_);\
  }else{\
    char* buf_ = (char*)malloc(bufsz_ + 1);\
    C_sprintf(buf_, __VA_ARGS__);\
    js_log(cons,buf_,bufsz_);\
    free(buf_);\
  }\
}


#define LBLL_USER_IMPL_LIB_C
#include "../lbll.h"


static const uint8_t FONT[] = {0,0,0,0,125,0,96,0,96,127,20,127,50,107,38,39,8,115,62,107,42,0,96,0,28,34,65,65,34,28,107,62,107,8,62,8,2,3,0,8,8,8,3,3,0,3,28,96,62,65,62,33,127,1,35,69,57,34,73,54,60,4,127,122,73,78,62,73,38,64,79,112,54,73,54,50,73,62,0,20,0,2,20,0,8,20,34,20,20,20,34,20,8,32,77,48,63,65,125,127,80,127,127,73,54,62,65,34,127,65,62,127,73,65,127,72,72,62,65,46,127,8,127,65,127,65,2,1,126,127,8,119,127,1,1,127,62,127,127,64,127,127,65,127,127,72,48,62,69,63,127,72,55,50,73,38,64,127,64,127,1,127,124,3,124,127,14,127,119,8,119,112,15,112,71,73,113,127,65,65,96,28,3,65,65,127,32,64,32,1,1,1,64,32,0};



EM_JS(void, js_px, (float x, float y), {
  let cnv = document.getElementById("display");
  let ctx = cnv.getContext('2d');
  ctx.fillRect(x,y,1,1);
});

EM_JS(void, js_unpx, (float x, float y), {
  let cnv = document.getElementById("display");
  let ctx = cnv.getContext('2d');
  ctx.clearRect(x,y,1,1);
});


EM_JS(void, js_clr, (float i0, float i1), {
  let cnv = document.getElementById("display");
  let ctx = cnv.getContext('2d');
  ctx.clearRect(0,64-i1*8,128,(i1-i0)*8);
});



void c_px(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  js_px(x,y);
}
void c_unpx(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  js_unpx(x,y);
}

void c_text(){
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

    for (int x = 0; x < 4; x++){
      for (int y = 0; y < 8; y++){
        if ( ((FONT[3*pointer+x]>>(7-y))&1) && x < 3 ){
          js_px(col+4*i+x,56-row*8+y);
        }else{
          js_unpx(col+4*i+x,56-row*8+y);
        }
      }
    }
  }
}

void c_show(){

}

void c_clr(){
  int i1 = (int)lbll_pop();
  int i0 = (int)lbll_pop();
  js_clr(i0,i1);
}

int init(){
  lbll_init_stdlib();

  lbll_reg_cfun("px"   ,2, c_px );
  lbll_reg_cfun("unpx" ,2, c_unpx);
  lbll_reg_cfun("text" ,2, c_text);
  lbll_reg_cfun("show" ,0, c_show);
  lbll_reg_cfun("clr"  ,2, c_clr );
  return 0;
}

int run(char* src){
  int err;

  int len = strlen(src);
  C_FILE* fd = (C_FILE*)malloc(sizeof(C_FILE));
  fd->bkend = F_BKEND_MEM;
  fd->d.mem.cap = len;
  fd->d.mem.dat = (char*)malloc(fd->d.mem.cap);
  fd->d.mem.len = len;
  fd->d.mem.pos = 0;

  memcpy(fd->d.mem.dat, src, len);
  
  C_FILE* fo = C_open_memstream_rw(NULL,NULL);

  err = lbll_parse(fd, fo);

  C_fclose(fd);

  if (err){
    C_fclose(fo);
    return err;
  }

  C_fseek(fo,0,SEEK_END);

  C_printf("parsing used: %ld B\n",C_ftell(fo));

  C_fseek(fo,0,SEEK_SET);

  lbll_print_instrs(fo);

  C_fseek(fo,0,SEEK_SET);

  lbll_stack = C_open_memstream_rw(NULL,NULL);
  lbll_frame = C_open_memstream_rw(NULL,NULL);

  printf("running...\n");

  cons = 1;
  err = lbll_run(fo);
  cons = 0;

  lbll_print_stack();
  lbll_print_frame();
  C_fseek(lbll_stack,0,SEEK_END);
  C_fseek(lbll_frame,0,SEEK_END);

  C_printf("stack used: %ld B\n",C_ftell(lbll_stack));
  C_printf("frame used: %ld B\n",C_ftell(lbll_frame));

  C_fclose(fo);
  C_fclose(lbll_stack);
  C_fclose(lbll_frame);
  
  return err;
}