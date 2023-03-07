#include <SD.h>
#include <stdarg.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define F_BKEND_SD_R 0
#define F_BKEND_SD_W 1
#define F_BKEND_MEM  2

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

typedef struct C_FILE_st {
  char bkend;
  union{
    File file;
    struct m_st {
      int pos;
      int len;
      int cap;
      char* dat;
      char** u_ptr;
      size_t* u_sz;
    } mem;
  } d ;
} C_FILE;

C_FILE* C_fopen(char* path, char* mode){
  C_FILE* fd = (C_FILE*)malloc(sizeof(C_FILE));
  if (mode[0] == 'r'){
    fd->d.file = SD.open(path, FILE_READ);
    fd->bkend = F_BKEND_SD_R;
  }else if (mode[0] == 'w'){
    fd->d.file = SD.open(path, FILE_WRITE);
    fd->bkend = F_BKEND_SD_W;
  }
  return fd;
}

int C_fseek(C_FILE* fd, long off, int when){
  if (fd->bkend == F_BKEND_MEM){
    if (when == SEEK_SET){
      fd->d.mem.pos = off;
    }else if (when == SEEK_CUR){
      fd->d.mem.pos += off;
    }else if (when == SEEK_END){
      fd->d.mem.pos = fd->d.mem.len;
    }
  }else{
    if (when == SEEK_SET){
      fd->d.file.seek(off);
    }else if (when == SEEK_CUR){
      fd->d.file.seek(fd->d.file.position()+off);
    }else{
      fd->d.file.seek(fd->d.file.size()+off);
    }
  }
}
int C_fgetc(C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    if (fd->d.mem.pos >= fd->d.mem.len){
      return -1;
    }
    return fd->d.mem.dat[fd->d.mem.pos++];
  }else{
    return fd->d.file.read();
  }
}
int C_fputc(int c, C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    if (fd->d.mem.pos >= fd->d.mem.cap){
      fd->d.mem.cap += 32;
      fd->d.mem.dat = (char*)realloc(fd->d.mem.dat,fd->d.mem.cap);
      if (!fd->d.mem.dat) Serial.println("AHHHH!!!!!! OOOOMMM!!!!!!!");
      
    }
    fd->d.mem.dat[fd->d.mem.pos++] = c;
    fd->d.mem.len = MAX(fd->d.mem.len, fd->d.mem.pos);
  }else{
    return fd->d.file.read();
  }
}
int C_ungetc(int c, C_FILE* fd){
  if (c == EOF) return c;
  
  if (fd->bkend == F_BKEND_MEM){
    fd->d.mem.dat[--fd->d.mem.pos] = c;
  }else if (fd->bkend == F_BKEND_SD_R){
    fd->d.file.seek(fd->d.file.position()-1);
  }else if (fd->bkend == F_BKEND_SD_W){
    fd->d.file.seek(fd->d.file.position()-1);
    fd->d.file.write(c);
    fd->d.file.seek(fd->d.file.position()-1);
  }
  return c;
}

int C_fread(void* x, size_t n, size_t m, C_FILE* fd){
  n*=m;
  if (fd->bkend == F_BKEND_MEM){
    memcpy(x, fd->d.mem.dat + fd->d.mem.pos, n);
    fd->d.mem.pos += n;
  }else{
    for (int i = 0; i < n; i++){
      int c = fd->d.file.read();
      ((char*)x)[i] = c;
    }
  }
}
int C_fwrite(void* x, size_t n, size_t m, C_FILE* fd){
  n*=m;
  if (fd->bkend == F_BKEND_MEM){
    if (fd->d.mem.pos+n >= fd->d.mem.cap){
      fd->d.mem.cap += n+28;
      fd->d.mem.dat = (char*)realloc(fd->d.mem.dat,fd->d.mem.cap);
      if (!fd->d.mem.dat) Serial.println("AHHHH!!!!!! OOOOMMM!!!!!!!");
    }
    
    memcpy(fd->d.mem.dat + fd->d.mem.pos, x, n);
    fd->d.mem.pos += n;
    fd->d.mem.len = MAX(fd->d.mem.len, fd->d.mem.pos);
  }else{
    for (int i = 0; i < n; i++){
      char c = ((char*)x)[i];
      fd->d.file.write(c);
    }
  }
}

long C_ftell(C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    return fd->d.mem.pos;
  }else{
    return fd->d.file.position();
  }
}

int C_fflush(C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    if (fd->d.mem.u_ptr){
      (*(fd->d.mem.u_ptr)) = fd->d.mem.dat;
    }
    if (fd->d.mem.u_sz){
      (*(fd->d.mem.u_sz)) = fd->d.mem.len;
    }
  }else{
    fd->d.file.flush();
  }
  return 0;
}

int C_fscanf(C_FILE* fd,char* s,void* x){
  if (fd->bkend == F_BKEND_MEM){
  }else{
    char buf[32];
    int c, i=0;
    while ((c = C_fgetc(fd)) != EOF && (
      c < 0x20 || 
      ('0' <= c && c <= '9') || 
      (c == '-' && !i) ||
      c == '.' ||
      (i && (c == 'e' || c =='E') )
     )){
      buf[i++] = c;
    }
    buf[i] = 0;
    C_ungetc(c, fd);
    (* (float *)x) = atof(buf);
  }
  return 1;
}

C_FILE* C_open_memstream(char** buf, size_t* sz){
  C_FILE* fd = (C_FILE*)malloc(sizeof(C_FILE));
  fd->bkend = F_BKEND_MEM;
  fd->d.mem.cap = 32;
  fd->d.mem.dat = (char*)malloc(fd->d.mem.cap);
  fd->d.mem.len = 0;
  fd->d.mem.pos = 0;
  fd->d.mem.u_ptr = buf;
  fd->d.mem.u_sz = sz;
  return fd;
}

int C_fclose(C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    if (fd->d.mem.u_ptr){
      (*(fd->d.mem.u_ptr)) = fd->d.mem.dat;
    }else{
      free(fd->d.mem.dat);
    }
    if (fd->d.mem.u_sz){
      (*(fd->d.mem.u_sz)) = fd->d.mem.len;
    }
  }else{
    fd->d.file.close();
  }
  free(fd);
  return 0;
}


int C_ftoa(float x, char* buf){
  int len = 0;
  if (x == 0){
    buf[0] = '0';
    buf[1] = 0;
    return 1;
  }if (x < 0){
    x = -x;
    buf[len++] = '-';
  }
  int xi = (int)x;
  int xf = (int)((x - xi)*10000);
  itoa(xi,buf+len,10);
  len = strlen(buf);
  
  if (xf){
    buf[len++] = '.';
    char tmp[8];
    itoa(xf,tmp,10);
    int nf = strlen(tmp);
    int n0 = 4 - nf;
    for (int i = 0; i < n0; i++){
      buf[len++] = '0';
    }
    strcpy(buf+len, tmp);
    len += nf;
    buf[len] = 0;
  }
  
  
  return len;
}

int C_sprintf(char* buf, char const *fmt, ...) {
  va_list arg;
  va_start(arg, fmt);
  char c;
  char tmp[32];
  int len = 0;
  if (buf) buf[0] = 0;
  while ((c = *(fmt++))){
    if (c == '%'){
      L_0:;
      c = *(fmt++);
      if (c == 'd'){
        L_i10:;
        int x = va_arg(arg, int);
        itoa(x, tmp, 10);
      }else if (c == 'x'){
        L_i16:;
        int x = va_arg(arg, int);
        itoa(x, tmp, 16);
      }else if (c == 'l'){
        c = *(fmt++);
        if (c == 'd'){
          goto L_i10;
        }else if (c == 'x'){
          goto L_i16;
        }
      }else if (c == '.'){
        c = *(fmt++);
        goto L_0;
      }else if (c == 'f' || c == 'g'){
        float x = va_arg(arg, double);
        C_ftoa(x, tmp);
      }else if (c == 's'){
        char* x = va_arg(arg, char*);
        if (buf) strcpy(buf+len, x);
        len += strlen(x);
        tmp[0] = 0;
      }else if (c == 'c'){
        char x = va_arg(arg, int);
        if (buf) buf[len] = x;
        len += 1;
        tmp[0] = 0;
      }else{
        tmp[0] = 0;
      }
      if (tmp[0]){
        if (buf) strcpy(buf+len, tmp);
        len += strlen(tmp);
      }
    }else{
      if (buf){
        buf[len++] = c;
      }else{
        len++;
      }
    }
  }
  if (buf) buf[len] = 0;
  va_end(arg);
  return len;
}

char C_printbuf[64];

#define C_printf(...) {\
  size_t bufsz_ = C_sprintf(NULL, __VA_ARGS__);\
  if (bufsz_ < 64){\
    C_sprintf(C_printbuf, __VA_ARGS__);\
    Serial.print(C_printbuf);\
  }else{\
    char* buf_ = (char*)malloc(bufsz_ + 1);\
    C_sprintf(buf_, __VA_ARGS__);\
    Serial.print(buf_);\
    free(buf_);\
  }\
}
