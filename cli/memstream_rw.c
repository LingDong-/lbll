#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F_BKEND_STD  1
#define F_BKEND_MEM  2

typedef struct C_FILE_st {
  char bkend;
  union{
    FILE* file;
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
  fd->bkend = F_BKEND_STD;
  fd->d.file = fopen(path,mode);
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
    return 0;
  }else{
    return fseek(fd->d.file, off, when);
  }
}
int C_fgetc(C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    if (fd->d.mem.pos >= fd->d.mem.len){
      return -1;
    }
    return fd->d.mem.dat[fd->d.mem.pos++];
  }else{
    return fgetc(fd->d.file);
  }
}

int C_fputc(int c, C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    if (fd->d.mem.pos >= fd->d.mem.cap){
      fd->d.mem.cap += 32;
      fd->d.mem.dat = (char*)realloc(fd->d.mem.dat,fd->d.mem.cap);
    }
    fd->d.mem.dat[fd->d.mem.pos++] = c;
    fd->d.mem.len = (fd->d.mem.len>fd->d.mem.pos)?fd->d.mem.len:fd->d.mem.pos;
  }else{
    return fputc(c,fd->d.file);
  }
  return c;
}

int C_ungetc(int c, C_FILE* fd){
  if (c == EOF) return c;
  if (fd->bkend == F_BKEND_MEM){
    fd->d.mem.dat[--fd->d.mem.pos] = c;
  }else{
    return ungetc(c,fd->d.file);
  }
  return c;
}

int C_fread(void* x, size_t n, size_t m, C_FILE* fd){
  
  if (fd->bkend == F_BKEND_MEM){
    n*=m;
    memcpy(x, fd->d.mem.dat + fd->d.mem.pos, n);
    fd->d.mem.pos += n;
    return m;
  }else{
    return fread(x,n,m,fd->d.file);
  }
}

int C_fwrite(void* x, size_t n, size_t m, C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    n*=m;
    if (fd->d.mem.pos+n >= fd->d.mem.cap){
      fd->d.mem.cap += n+28;
      fd->d.mem.dat = (char*)realloc(fd->d.mem.dat,fd->d.mem.cap);
    }
    
    memcpy(fd->d.mem.dat + fd->d.mem.pos, x, n);
    fd->d.mem.pos += n;
    fd->d.mem.len = (fd->d.mem.len>fd->d.mem.pos)?fd->d.mem.len:fd->d.mem.pos;
    return m;
  }else{
    return fwrite(x,n,m,fd->d.file);
  }
}

long C_ftell(C_FILE* fd){
  if (fd->bkend == F_BKEND_MEM){
    return fd->d.mem.pos;
  }else{
    return ftell(fd->d.file);
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
    return fflush(fd->d.file);
  }
  return 0;
}

C_FILE* C_fscanf_tmp0;
int C_fscanf_tmp1;
int C_fscanf_tmp2;

#define C_fscanf(fd,s,...) \
  (C_fscanf_tmp0 = (fd),\
  ((C_fscanf_tmp0->bkend == F_BKEND_MEM)?\
    (C_fscanf_tmp1 = sscanf(C_fscanf_tmp0->d.mem.dat + C_fscanf_tmp0->d.mem.pos,s"%n",__VA_ARGS__,&(C_fscanf_tmp2)),\
      C_fscanf_tmp0->d.mem.pos+=C_fscanf_tmp2,\
      C_fscanf_tmp1\
    ):(\
       fscanf(C_fscanf_tmp0->d.file,s,__VA_ARGS__)\
    ))\
  )

C_FILE* C_open_memstream_rw(char** buf, size_t* sz){
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
    fclose(fd->d.file);
  }
  free(fd);
  return 0;
}

#define C_printf(...) printf(__VA_ARGS__)
#define C_sprintf(...) sprintf(__VA_ARGS__)
