// lbll.c
// an interpreter for
// lbll: lingdong's bad language (with labels)

#ifndef LBLL_INCLUDE_GUARD
#define LBLL_INCLUDE_GUARD

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define LBLL_INSTR_LABL 1
#define LBLL_INSTR_GOTO 2
#define LBLL_INSTR_CALL 3
#define LBLL_INSTR_VAR  4
#define LBLL_INSTR_ASGN 5
#define LBLL_INSTR_IF   6
#define LBLL_INSTR_PUSH 7
#define LBLL_INSTR_RET  8
#define LBLL_INSTR_CLLR 9

#define LBLL_TOK_NIL  0
#define LBLL_TOK_ANS  1
#define LBLL_TOK_GOTO 2
#define LBLL_TOK_NUM  3
#define LBLL_TOK_IDT  4
#define LBLL_TOK_TEL  5

#define LBLL_MAX_CFUN  64

#ifndef LBLL_USER_IMPL_LIB_C
#define C_FILE FILE
#define C_fseek fseek
#define C_fgetc fgetc
#define C_ungetc ungetc
#define C_fread fread
#define C_fwrite fwrite
#define C_fputc fputc
#define C_ftell ftell
#define C_fflush fflush
#define C_fscanf fscanf
#define C_printf printf
#define C_sprintf sprintf
#endif

uint64_t lbll_nsp = 0;
typedef void (* lbll_cfun_impl_t) (void);

typedef struct lbll_cfun_st {
  uint64_t name;
  char arity;
  lbll_cfun_impl_t impl;
} lbll_cfun_t;


lbll_cfun_t lbll_cfuns[LBLL_MAX_CFUN];
int lbll_ncf = 0;

C_FILE* lbll_stack = NULL;
C_FILE* lbll_frame = NULL;

uint64_t lbll_str_to_word(char* s){
  int i = 0;
  uint64_t q = 0;
  while (s[i]){
    q |= ((uint64_t)s[i]<<(uint64_t)( (i) <<3));
    i++;
  }
  return q;
}

void lbll_reg_cfun(char* name, char arity, lbll_cfun_impl_t impl){
  lbll_cfuns[lbll_ncf].name  = lbll_str_to_word(name);
  lbll_cfuns[lbll_ncf].arity = arity;
  lbll_cfuns[lbll_ncf].impl  = impl;
  lbll_ncf++;
}

lbll_cfun_t lbll_find_cfun(uint64_t name){
  lbll_cfun_t f = {0};
  for (int i = 0; i < lbll_ncf; i++){
    if (lbll_cfuns[i].name == name){
      f = lbll_cfuns[i];
      return f;
    }
  }
  return f;
}

void lbll_print_word(uint64_t q){
  if (q <= 0x20){
    C_printf("\\%d",(char)q);
    return;
  }
  while (q){
    C_printf("%c", (char)(q&0xff));
    q >>= 8;
  }
}

int lbll_word_len(uint64_t q){
  int i = 0;
  while (q){
    q >>= 8;
    i ++;
  }
  return i;
}


uint64_t lbll_get_word(C_FILE* fd){
  uint64_t q = 0;
  int c,i=0;
  while ((c = C_fgetc(fd)) > 0x20){
    q |= ((uint64_t)c<<(uint64_t)( (i++) <<3));
  }
  if (q == '.'){
    q = 0;
  }else if ((q & 0xff) == '.'){
    q = (q << (lbll_word_len(lbll_nsp)<<3)) | lbll_nsp;
  }
  return q;
}

void lbll_skip_sp(C_FILE* fd){
  int c;
  while ((c = C_fgetc(fd)) <= 0x20 && c != EOF){}
  C_ungetc(c,fd);
}


int lbll_get_tok(C_FILE* fd, C_FILE* fo){
  lbll_skip_sp(fd);
  int c = C_fgetc(fd);
  uint64_t q = 0;
  if (c == '@'){
    C_fgetc(fd);
    q = lbll_get_word(fd);
    C_fputc(LBLL_TOK_GOTO,fo);
    C_fwrite(&q,8,1,fo);
    return 9;
  }else if (c == '~'){
    q = 0;
    C_fputc(LBLL_TOK_ANS,fo);
    return 1;
  }else if (c == '*'){
    q = 0;
    C_fputc(LBLL_TOK_NIL,fo);
    return 1;
  }else if (c == '#'){
    q = 0;
    C_fputc(LBLL_TOK_TEL,fo);
    return 1;
  }else if (c >= 'A' || c == '.'){
    C_ungetc(c,fd);
    q = lbll_get_word(fd);
    C_fputc(LBLL_TOK_IDT,fo);
    C_fwrite(&q,8,1,fo);
    return 9;
  }else{
    C_ungetc(c,fd);
    float x;
    C_fscanf(fd,"%f",&x);
    C_fputc(LBLL_TOK_NUM,fo);
    C_fwrite(&x,4,1,fo);
    return 5;
  }
}

int lbll_parse(C_FILE* fd, C_FILE* fo){

  lbll_nsp = 0;

  int c, c1;
  uint64_t q, q1;
  while (1){
    lbll_skip_sp(fd);
    c = C_fgetc(fd);
    if (c == EOF){
      break;
    }
    if (c == '@'){
      c1 = C_fgetc(fd);
      if (c1 == '@'){
        q = lbll_get_word(fd);
        C_fputc(LBLL_INSTR_GOTO,fo);
        C_fputc(8,fo);
        C_fwrite(&q,8,1,fo);

      }else{
        int is_nsp = 0;
        if (c1 == ':'){
          is_nsp = 1;
          
        }else{
          C_ungetc(c1,fd);
        }
        q = lbll_get_word(fd);
        if (is_nsp){
          lbll_nsp = q;
        }
        C_fputc(LBLL_INSTR_LABL,fo);
        C_fputc(8,fo);
        C_fwrite(&q,8,1,fo);
      }
    }else if (c == ':'){
      q = lbll_get_word(fd);
      lbll_nsp = q;
    }else if (c == '-'){
      c1 = C_fgetc(fd);
      if (c1 == '>'){
        lbll_skip_sp(fd);
        q = lbll_get_word(fd);
        C_fputc(LBLL_INSTR_VAR,fo);
        C_fputc(8,fo);
        C_fwrite(&q,8,1,fo);
      }else{
        C_ungetc(c1,fd);
        goto lvar;
      }
    }else if (c == '='){
      c1 = C_fgetc(fd);
      if (c1 == '>'){
        lbll_skip_sp(fd);
        q = lbll_get_word(fd);
        C_fputc(LBLL_INSTR_ASGN,fo);
        C_fputc(8,fo);
        C_fwrite(&q,8,1,fo);
      }else{
        C_ungetc(c1,fd);
        goto lvar;
      }
    }else if (c == '^'){
      c1 = C_fgetc(fd);
      C_ungetc(c1,fd);
      if (c1 != '^'){
        C_fputc(LBLL_INSTR_PUSH,fo);
        C_fputc(0,fo);
        int n = lbll_get_tok(fd,fo);
        C_fseek(fo,-n-1,SEEK_CUR);
        C_fputc(n,fo);
        C_fseek(fo,n,SEEK_CUR);
      }else{
        goto lvar;
      }
    }else if (c == '?'){
      C_fputc(LBLL_INSTR_IF,fo);
      C_fputc(0,fo);
      int n = 0;
      n += lbll_get_tok(fd,fo);
      n += lbll_get_tok(fd,fo);
      C_fseek(fo,-n-1,SEEK_CUR);
      C_fputc(n,fo);
      C_fseek(fo,n,SEEK_CUR);
    }else if (c == '>'){
      c1 = C_fgetc(fd);
      int c2 = C_fgetc(fd);
      if (c1 == '@' && c2 == '@'){
        q = 1;
        C_fputc(LBLL_INSTR_GOTO,fo);
        C_fputc(8,fo);
        C_fwrite(&q,8,1,fo);
      }else{
        C_ungetc(c2,fd);
        C_ungetc(c1,fd);
        goto lvar;
      }
    }else if (c == '"'){
      int n = 0;
      while ((c1 = C_fgetc(fd)) != '"'){
        if (c1 == '\\'){
          c1 = C_fgetc(fd);
          if (c1 == 'n'){
            c1 = '\n';
          }else if (c1 == 'r'){
            c1 = '\r';
          }else if (c1 == 't'){
            c1 = '\t';
          }
        }
        C_fputc(LBLL_INSTR_PUSH,fo);
        C_fputc(5,fo);
        C_fputc(LBLL_TOK_NUM,fo);
        float x = c1;
        C_fwrite(&x,4,1,fo);
        n++;
      }
      C_fputc(LBLL_INSTR_PUSH,fo);
      C_fputc(5,fo);
      C_fputc(LBLL_TOK_NUM,fo);
      float x = n;
      C_fwrite(&x,4,1,fo);

    }else if (c == ';'){
      while ((c1 = C_fgetc(fd)) != ';'){
      }
    }else if (c == '%'){
      c1 = C_fgetc(fd);
      if (c1 == '%'){
        C_fputc(LBLL_INSTR_RET,fo);
        C_fputc(1,fo);
        int c2 = C_fgetc(fd);
        if (c2 == '.'){
          C_fputc(0,fo);
        }else{
          C_ungetc(c2,fd);
          C_fputc(1,fo);
        }
      }else{
        C_ungetc(c1,fd);
        C_fputc(LBLL_INSTR_CLLR,fo);
        C_fputc(0,fo);
      }
    }else{
lvar:;
      C_ungetc(c,fd);
      q = lbll_get_word(fd);
      
      lbll_cfun_t f = lbll_find_cfun(q);
      if (f.name != q){
        C_printf("[error] operator not found: ");
        lbll_print_word(q);
        C_printf(" (position %ld)\n", C_ftell(fd));
        return -1;
      }
      int a = f.arity;

      int n = 9;

      C_fputc(LBLL_INSTR_CALL,fo);
      C_fputc(0,fo);
      C_fwrite(&q,8,1,fo);
      C_fputc(a,fo);
      
      for (int i = 0; i < a; i++){
        n += lbll_get_tok(fd,fo);
      }

      C_fseek(fo,-n-1,SEEK_CUR);
      C_fputc(n,fo);
      C_fseek(fo,n,SEEK_CUR);

    }
  }

  C_fflush(fo);
  return 0;
}


void lbll_print_tok(C_FILE* fo){
  int c = C_fgetc(fo);
  if (c == LBLL_TOK_GOTO){
    C_printf("<@@  ");
    uint64_t q;
    C_fread(&q,8,1,fo);
    lbll_print_word(q);
    C_printf(">");
  }else if (c == LBLL_TOK_ANS){
    C_printf("<ans ~>");
  }else if (c == LBLL_TOK_NIL){
    C_printf("<nil *>");
  }else if (c == LBLL_TOK_NUM){
    float x;
    C_fread(&x,4,1,fo);
    C_printf("<num %g>",x);
  }else if (c == LBLL_TOK_IDT){
    C_printf("<idt ");
    uint64_t q;
    C_fread(&q,8,1,fo);
    lbll_print_word(q);
    C_printf(">");
  }else if (c == LBLL_TOK_TEL){
    C_printf("<tel #>");
  }
}

void lbll_print_instrs(C_FILE* fo){
  int c;
  while (1){
    long pos = C_ftell(fo);
    c = C_fgetc(fo);
    if (c == EOF){
      break;
    }
    C_printf("%.4lx ",pos);
    uint8_t n = C_fgetc(fo);
    if (c == LBLL_INSTR_LABL){
      C_printf("[LABL:%d]\t",n);
      uint64_t q;
      C_fread(&q,8,1,fo);
      lbll_print_word(q);
      C_printf("\n");
    }else if (c == LBLL_INSTR_GOTO){
      C_printf("[GOTO:%d]\t",n);
      uint64_t q;
      C_fread(&q,8,1,fo);
      lbll_print_word(q);
      C_printf("\n");
    }else if (c == LBLL_INSTR_VAR){
      C_printf("[VAR :%d]\t",n);
      uint64_t q;
      C_fread(&q,8,1,fo);
      lbll_print_word(q);
      C_printf("\n");
    }else if (c == LBLL_INSTR_ASGN){
      C_printf("[ASGN:%d]\t",n);
      uint64_t q;
      C_fread(&q,8,1,fo);
      lbll_print_word(q);
      C_printf("\n");
    }else if (c == LBLL_INSTR_IF){
      C_printf("[IF  :%d]\t",n);
      lbll_print_tok(fo);
      C_printf("\t");
      lbll_print_tok(fo);
      C_printf("\n");
    }else if (c == LBLL_INSTR_RET){
      C_printf("[RET :%d]\t",n);
      int r = C_fgetc(fo);
      C_printf("(%d)\n",r);
    }else if (c == LBLL_INSTR_CLLR){
      C_printf("[CLLR:%d]\n",n);
    }else if (c == LBLL_INSTR_PUSH){
      C_printf("[PUSH:%d]\t",n);
      lbll_print_tok(fo);
      C_printf("\n");
    }else if (c == LBLL_INSTR_CALL){
      C_printf("[CALL:%d]\t",n);
      uint64_t q;
      C_fread(&q,8,1,fo);
      lbll_print_word(q);
      C_printf("\t");
      int a = C_fgetc(fo);
      C_printf("(%d)\t",a);
      for (int i = 0; i < a; i++){
        lbll_print_tok(fo);
      }
      C_printf("\n");
    }else{
      C_printf("[?!?!:%d]\n",n);
      C_fseek(fo,n,SEEK_CUR);
    }
  }
}

void lbll_print_stack(){
  long pos = C_ftell(lbll_stack);
  C_fseek(lbll_stack,0,SEEK_SET);
  C_printf("STACK BOTTOM< ");
  while (C_ftell(lbll_stack)<pos){
    float x;
    C_fread(&x,4,1,lbll_stack);
    C_printf("%g ",x);
  }
  C_printf(" >TOP\n");
}


void lbll_print_frame(){
  long pos = C_ftell(lbll_frame);
  C_fseek(lbll_frame,0,SEEK_SET);
  C_printf("FRAME BOTTOM< ");
  while (C_ftell(lbll_frame)<pos){
    uint64_t q;
    float x;
    C_fread(&q,8,1,lbll_frame);
    C_fread(&x,4,1,lbll_frame);
    lbll_print_word(q);
    C_printf(":%g ",x);
  }
  C_printf(" >TOP\n");
}

float lbll_pop(){
  C_fseek(lbll_stack,-4,SEEK_CUR);
  float x;
  C_fread(&x,4,1,lbll_stack);
  C_fseek(lbll_stack,-4,SEEK_CUR);
  return x;
}

void lbll_push(float x){
  C_fflush(lbll_stack);
  C_fwrite(&x, 4, 1, lbll_stack);
  C_fflush(lbll_stack);
}


void lbll_skip_tok(C_FILE* fo){
  int c = C_fgetc(fo);
  if (c == LBLL_TOK_GOTO){
    C_fseek(fo,8,SEEK_CUR);
  }else if (c == LBLL_TOK_NUM){
    C_fseek(fo,4,SEEK_CUR);
  }else if (c == LBLL_TOK_IDT){
    C_fseek(fo,8,SEEK_CUR);
  }
}

int lbll_run(C_FILE* fo){
  uint64_t gt_name;
  int eval_ret;
  float eval_val;
  int eval_loc;
  long pos0, pos1, pos2;
  int skptok = 0;
  
  int c;
  uint64_t q, q1;
  float x, x1, y;
  int k, i, a;
  uint8_t n;
  long oldpos, newpos, toppos;

lrun:;

  int ptr = C_ftell(lbll_frame);

  while (1){
    lnext:;
    
    c = C_fgetc(fo);

    if (c == EOF){
      return 0;
    }
    n = C_fgetc(fo);
    if (c == LBLL_INSTR_LABL){
      C_fseek(fo,n,SEEK_CUR);
    }else if (c == LBLL_INSTR_GOTO){

      C_fread(&gt_name,8,1,fo);
      goto lgoto;
      
    }else if (c == LBLL_INSTR_VAR){
      C_fread(&q,8,1,fo);
      x = lbll_pop();
      C_fwrite(&q,8,1,lbll_frame);
      C_fwrite(&x,4,1,lbll_frame);
    }else if (c == LBLL_INSTR_CLLR){
      q = '%';
      x = pos0;
      C_fwrite(&q,8,1,lbll_frame);
      C_fwrite(&x,4,1,lbll_frame);
    }else if (c == LBLL_INSTR_ASGN){

      C_fread(&q,8,1,fo);
      x = lbll_pop();

      int ofr = C_ftell(lbll_frame);
      while (1){
        
        C_fseek(lbll_frame,-12,SEEK_CUR);
        C_fread(&q1,8,1,lbll_frame);
        C_fread(&x1,4,1,lbll_frame);
        if (q == q1){
          C_fseek(lbll_frame,-4,SEEK_CUR);
          C_fwrite(&x,4,1,lbll_frame);
          break;
        }
        C_fseek(lbll_frame,-12,SEEK_CUR);
      }
      C_fseek(lbll_frame,ofr,SEEK_SET);

    }else if (c == LBLL_INSTR_IF){
      x = lbll_pop();
      
      if (x != 0){
        eval_loc = 2;
        goto leval_tok;
        leval_loc2:;

        if (eval_ret) lbll_push(eval_val);
        
        lbll_skip_tok(fo);
        
      }else{
        lbll_skip_tok(fo);

        eval_loc = 3;
        goto leval_tok;
        leval_loc3:;

        if (eval_ret) lbll_push(eval_val);
      }

    }else if (c == LBLL_INSTR_RET){
      k = C_fgetc(fo);

      if (k){
        while (1){
          if (C_ftell(lbll_frame) == 0){
            
            C_fseek(fo,0,SEEK_END);
            // C_fseek(lbll_stack,0,SEEK_SET);
            return 0;
          }
          C_fseek(lbll_frame,-12,SEEK_CUR);
          C_fread(&q1,8,1,lbll_frame);
          C_fread(&x1,4,1,lbll_frame);
          if (q1 == '%'){
            C_fseek(fo,(long)x1,SEEK_SET);
            C_fseek(lbll_frame,-12,SEEK_CUR);
            goto lrun;
          }
          C_fseek(lbll_frame,-12,SEEK_CUR);
        }
      }else{
        C_fseek(lbll_frame, ptr, SEEK_SET);
        C_fseek(fo,pos0,SEEK_SET);
        goto lnext;
      }

    }else if (c == LBLL_INSTR_PUSH){
      eval_loc = 0;
      goto leval_tok;
      leval_loc0:;
      if (eval_ret) lbll_push(eval_val);
      
    }else if (c == LBLL_INSTR_CALL){
      C_fread(&q,8,1,fo);
      a = C_fgetc(fo);
      oldpos = C_ftell(lbll_stack);
      newpos = oldpos;
      toppos = oldpos;
      
      for (i = 0; i < a; i++){

        eval_loc = 1;
        goto leval_tok;
        leval_loc1:;

        newpos = C_ftell(lbll_stack);
        C_fseek(lbll_stack,toppos,SEEK_SET);
        if (eval_ret) lbll_push(eval_val);
        toppos = C_ftell(lbll_stack);
        C_fseek(lbll_stack,newpos,SEEK_SET);

      }
      int offs = oldpos - newpos;
      int cnt = toppos - oldpos;
      // C_printf("%d %d %d %d %d %d\n",a,oldpos,newpos,toppos,offs,cnt);
      if (offs){
        for (i = 0; i < cnt; i++){
          C_fseek(lbll_stack,newpos+i+offs,SEEK_SET);
          int c = C_fgetc(lbll_stack);
          C_fseek(lbll_stack,newpos+i,SEEK_SET);
          C_fputc(c,lbll_stack);
        }
      }
      C_fseek(lbll_stack,newpos+cnt,SEEK_SET);
      // lbll_print_stack();
      lbll_cfun_t fun = lbll_find_cfun(q);
      fun.impl();
    }
  }

lgoto:;

  pos0 = C_ftell(fo);

  if (gt_name == 1){
    gt_name = 0;
    n = (int)lbll_pop();
    for (i = 0; i < n; i++){
      int c = (int)lbll_pop();
      gt_name = (gt_name << 8) | c;
    }
  }
  while (1){
    pos1 = C_ftell(fo);
    c = C_fgetc(fo);
    if (c == EOF){
      C_fseek(fo,0,SEEK_SET);
      continue;
    }
    n = C_fgetc(fo);
    if (c == LBLL_INSTR_LABL){
      C_fread(&q,8,1,fo);
      if (q == gt_name){
        goto lrun;
      }
    }else{
      C_fseek(fo,n,SEEK_CUR);
    }
    pos2 = C_ftell(fo);
    if (pos1 < pos0 && pos2 >= pos0){
      C_printf("[error] label not found: ");
      lbll_print_word(gt_name);
      C_printf("\n");
      return -1;
    }
  }
  

leval_tok:;

  c = C_fgetc(fo);
  if (c == LBLL_TOK_GOTO){
    uint64_t q;
    C_fread(&q,8,1,fo);
    long pos = C_ftell(fo);
    gt_name = q;

    if (eval_loc == 2){
      lbll_skip_tok(fo);
    }
    goto lgoto;
  }else if (c == LBLL_TOK_ANS){
    eval_val = lbll_pop();
    eval_ret = 1;
    goto leval_ret;
  }else if (c == LBLL_TOK_NIL){
    eval_ret = 0;
    goto leval_ret;
  }else if (c == LBLL_TOK_NUM){
    C_fread(&eval_val,4,1,fo);
    eval_ret = 1;
    goto leval_ret;
  }else if (c == LBLL_TOK_TEL){
    long pos = C_ftell(lbll_stack);
    eval_val = (float)(pos>>2);
    eval_ret = 1;
    goto leval_ret;
  }else if (c == LBLL_TOK_IDT){
    uint64_t q;
    C_fread(&q,8,1,fo);

    int ofr = C_ftell(lbll_frame);
    while (1){
      C_fseek(lbll_frame,-12,SEEK_CUR);
      uint64_t q1;
      C_fread(&q1,8,1,lbll_frame);
      C_fread(&eval_val,4,1,lbll_frame);
      if (q == q1){
        break;
      }
      C_fseek(lbll_frame,-12,SEEK_CUR);
    }
    C_fseek(lbll_frame,ofr,SEEK_SET);
    eval_ret = 1;
    goto leval_ret;
  }
leval_ret:;
  switch (eval_loc){
    case 0: goto leval_loc0;
    case 1: goto leval_loc1;
    case 2: goto leval_loc2;
    case 3: goto leval_loc3;
    default: break;
  }

  return 0;

}


void lbll_c___(){}

#define LBLL_C_BNOP(_name,_op) void lbll_c_ ## _name(){\
  float x = lbll_pop();\
  float y = lbll_pop();\
  lbll_push((float)(y _op x));\
}
#define LBLL_C_UNFN(_fun) void lbll_c_ ## _fun(){\
  float x = lbll_pop();\
  lbll_push((float)_fun(x));\
}
#define LBLL_C_BNFN(_fun) void lbll_c_ ## _fun(){\
  float x = lbll_pop();\
  float y = lbll_pop();\
  lbll_push((float)_fun(y,x));\
}

#define LBLL_C_BWBNOP(_name,_op) void lbll_c_ ## _name(){\
  uint16_t x = (uint16_t)lbll_pop();\
  uint16_t y = (uint16_t)lbll_pop();\
  lbll_push((float)(uint16_t)((y _op x)&0xffff));\
}

LBLL_C_BNOP(sub, -);
LBLL_C_BNOP(add, +);
LBLL_C_BNOP(mul, *);
LBLL_C_BNOP(div, /);
LBLL_C_BNOP(vor, ||);
LBLL_C_BNOP(vand,&&);
LBLL_C_BNOP(gt,  >);
LBLL_C_BNOP(geq, >=);
LBLL_C_BNOP(lt,  <);
LBLL_C_BNOP(leq, <=);
LBLL_C_BNOP(eq,  ==);
LBLL_C_BNOP(neq, !=);

LBLL_C_BWBNOP(band,  &);
LBLL_C_BWBNOP(bor,   |);
LBLL_C_BWBNOP(bshl, <<);
LBLL_C_BWBNOP(bshr, >>);
LBLL_C_BWBNOP(bxor,  ^);

LBLL_C_UNFN(floor);
LBLL_C_UNFN(ceil);
LBLL_C_UNFN(fabs);
LBLL_C_UNFN(sin);
LBLL_C_UNFN(cos);
LBLL_C_UNFN(sqrt);
LBLL_C_UNFN(exp);
LBLL_C_UNFN(asin);
LBLL_C_UNFN(acos);
LBLL_C_UNFN(log);
LBLL_C_UNFN(round);

LBLL_C_BNFN(fmod);
LBLL_C_BNFN(pow);
LBLL_C_BNFN(atan2);


void lbll_c_srand(){
  float x = lbll_pop();
  // if (x < 0){
  //   srand(time(0));
  // }else{
  srand((unsigned int)x);
  // }
}
void lbll_c_rand(){
  lbll_push((float)rand()/(float)RAND_MAX);
}

void lbll_c_echo(){
  
  int n = (int)lbll_pop();
  C_fseek(lbll_stack,-n*4,SEEK_CUR);
  for (int i = 0; i < n; i++){
    float x;
    C_fread(&x,4,1,lbll_stack);
    C_printf("%c",(char)x);
  }
  C_fseek(lbll_stack,-n*4,SEEK_CUR);
}

void lbll_c_echn(){
  lbll_c_echo();
  C_printf("\n");
}


void lbll_c_eqz(){
  float x = lbll_pop();
  lbll_push(x == 0);
}

void lbll_c_imod(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  lbll_push((float)(x/y));
  lbll_push((float)(x%y));
}

void lbll_c_neg(){
  float x = lbll_pop();
  lbll_push(-x);
}

void lbll_c_bnot(){
  uint16_t x = (uint16_t)lbll_pop();
  lbll_push((float)(uint16_t)((~x)&0xffff));
}


void lbll_c_ntos(){
  float x = lbll_pop();
  char buf[32];
  C_sprintf(buf,"%g",x);
  char* b = buf;
  int i = 0;
  while (*b){
    lbll_push( (float)(*b) );
    b++;
    i++;
  }
  lbll_push((float)i);
}

void lbll_c_ston(){
  int n = (int)lbll_pop();
  C_fseek(lbll_stack,-n*4,SEEK_CUR);
  char buf[n+1];

  for (int i = 0; i < n; i++){
    float x;
    C_fread(&x,4,1,lbll_stack);
    buf[i] = (char)x;
  }
  buf[n] = 0;
  C_fseek(lbll_stack,-n*4,SEEK_CUR);
  lbll_push(atof(buf));
}


void lbll_c_droq(){
  // int x = (int)lbll_pop();
  // int n;
  // if (x < 0){
  //   n = -x;
  // }else{
  //   n = (C_ftell(lbll_stack)>>2)-x;
  // }
  // for (int i = 0; i < n; i++){
  //   lbll_pop();
  // }
  int x = (int)lbll_pop();
  if (x < 0){
    x = (C_ftell(lbll_stack)>>2)+x;
  }
  C_fseek(lbll_stack, x<<2, SEEK_SET);

}

void lbll_c_peek(){
  int x = (int)lbll_pop();
  long pos = C_ftell(lbll_stack);
  if (x < 0){
    C_fseek(lbll_stack,x*4,SEEK_CUR);
  }else{
    C_fseek(lbll_stack,x*4,SEEK_SET);
  }
  float y;
  C_fread(&y,4,1,lbll_stack);
  C_fseek(lbll_stack,pos,SEEK_SET);
  lbll_push(y);
}

void lbll_c_edit(){
  float y = lbll_pop();
  int x = (int)lbll_pop();
  long pos = C_ftell(lbll_stack);
  if (x < 0){
    C_fseek(lbll_stack,x*4,SEEK_CUR);
  }else{
    C_fseek(lbll_stack,x*4,SEEK_SET);
  }
  C_fwrite(&y,4,1,lbll_stack);
  C_fseek(lbll_stack,pos,SEEK_SET);
}



void lbll_c_roll(){
  int m = (int)lbll_pop();
  int i0 = (int)lbll_pop();
  int n;
  long pos = C_ftell(lbll_stack);

  if (i0 < 0){
    n = -i0;
  }else{
    n = (pos>>2) - i0;
  }

  if (m == 0){
    return;
  }
  
  if (m > 0){
    for (int i = 0; i < m; i++){
      float y;
      C_fseek(lbll_stack,-4,SEEK_CUR);
      C_fread(&y,4,1,lbll_stack);

      for (int j = 0; j < n-1; j++){
        C_fseek(lbll_stack,-8,SEEK_CUR);
        float x;
        C_fread(&x,4,1,lbll_stack);
        C_fflush(lbll_stack);
        C_fwrite(&x,4,1,lbll_stack);
        C_fseek(lbll_stack,-4,SEEK_CUR);
      }
      C_fseek(lbll_stack,-4,SEEK_CUR);
      C_fwrite(&y,4,1,lbll_stack);
      C_fseek(lbll_stack,pos,SEEK_SET);
    }
  }else{
    m = -m;

    for (int i = 0; i < m; i++){
      C_fseek(lbll_stack,-4*n,SEEK_CUR);
      float y;
      C_fread(&y,4,1,lbll_stack);
      C_fseek(lbll_stack,-4,SEEK_CUR);

      for (int j = 0; j < n-1; j++){
        float x;
        C_fseek(lbll_stack,4,SEEK_CUR);
        C_fread(&x,4,1,lbll_stack);
        C_fseek(lbll_stack,-8,SEEK_CUR);
        C_fwrite(&x,4,1,lbll_stack);
      }
      C_fwrite(&y,4,1,lbll_stack);
      C_fseek(lbll_stack,pos,SEEK_SET);
    }

  }

}



void lbll_c_rev(){
  int i0 = (int)lbll_pop();
  int n;
  long pos = C_ftell(lbll_stack);

  if (i0 < 0){
    n = -i0;
  }else{
    n = (pos>>2) - i0;
  }

  for (int j = 0; j < n/2; j++){
    C_fseek(lbll_stack,pos-n*4+j*4,SEEK_SET);
    float x;
    C_fread(&x,4,1,lbll_stack);
    C_fseek(lbll_stack,pos-4-j*4,SEEK_SET);
    float y;
    C_fread(&y,4,1,lbll_stack);
    C_fseek(lbll_stack,pos-n*4+j*4,SEEK_SET);
    C_fwrite(&y,4,1,lbll_stack);
    C_fseek(lbll_stack,pos-4-j*4,SEEK_SET);
    C_fwrite(&x,4,1,lbll_stack);
  }
  C_fseek(lbll_stack,pos,SEEK_SET);
}


void lbll_c_swap(){
  int i0 = (int)lbll_pop();
  int i1 = (int)lbll_pop();

  long pos = C_ftell(lbll_stack);
  if (i0 < 0){
    i0 = (pos>>2) + i0;
  }
  if (i1 < 0){
    i1 = (pos>>2) + i1;
  }

  C_fseek(lbll_stack,i0<<2,SEEK_SET);
  float x;
  C_fread(&x,4,1,lbll_stack);
  C_fseek(lbll_stack,i1<<2,SEEK_SET);
  float y;
  C_fread(&y,4,1,lbll_stack);
  C_fseek(lbll_stack,i0<<2,SEEK_SET);
  C_fwrite(&y,4,1,lbll_stack);
  C_fseek(lbll_stack,i1<<2,SEEK_SET);
  C_fwrite(&x,4,1,lbll_stack);
  
  C_fseek(lbll_stack,pos,SEEK_SET);
}


void lbll_c_dup(){
  int i1 = (int)lbll_pop();
  int i0 = (int)lbll_pop();
  long pos = C_ftell(lbll_stack);
  if (i0 < 0){
    i0 = (pos>>2) + i0;
  }
  if (i1 < 0){
    i1 = (pos>>2) + i1;
  }

  for (int i = i0; i < i1; i++){
    C_fseek(lbll_stack,i<<2,SEEK_SET);
    float x;
    C_fread(&x,4,1,lbll_stack);
    C_fseek(lbll_stack,pos,SEEK_SET);
    lbll_push(x);
    pos = C_ftell(lbll_stack);
  }
}

void lbll_c_fill(){
  int n = (int)lbll_pop();
  float x = lbll_pop();
  for (int i = 0; i < n; i++){
    lbll_push(x);
  }
}


void lbll_init_stdlib(){
  lbll_reg_cfun("sub" ,2,lbll_c_sub );
  lbll_reg_cfun("add" ,2,lbll_c_add );
  lbll_reg_cfun("mul" ,2,lbll_c_mul );
  lbll_reg_cfun("div" ,2,lbll_c_div );
  lbll_reg_cfun("fmod",2,lbll_c_fmod );
  lbll_reg_cfun("imod",2,lbll_c_imod );
  lbll_reg_cfun("pow" ,2,lbll_c_pow );
  lbll_reg_cfun("atn2",2,lbll_c_atan2);
  lbll_reg_cfun("abs" ,1,lbll_c_fabs);
  lbll_reg_cfun("flor",1,lbll_c_floor);
  lbll_reg_cfun("ceil",1,lbll_c_ceil);
  lbll_reg_cfun("vand",2,lbll_c_vand);
  lbll_reg_cfun("vor" ,2,lbll_c_vor );
  lbll_reg_cfun("lt"  ,2,lbll_c_lt  );
  lbll_reg_cfun("gt"  ,2,lbll_c_gt  );
  lbll_reg_cfun("leq" ,2,lbll_c_leq );
  lbll_reg_cfun("geq" ,2,lbll_c_geq );
  lbll_reg_cfun("eq"  ,2,lbll_c_eq  );
  lbll_reg_cfun("neq" ,2,lbll_c_neq );
  lbll_reg_cfun("eqz" ,1,lbll_c_eqz );
  lbll_reg_cfun("neg" ,1,lbll_c_neg );
  lbll_reg_cfun("rond",1,lbll_c_round);
  lbll_reg_cfun("exp" ,1,lbll_c_exp);
  lbll_reg_cfun("sqrt",1,lbll_c_sqrt);
  lbll_reg_cfun("asin",1,lbll_c_asin);
  lbll_reg_cfun("acos",1,lbll_c_acos);
  lbll_reg_cfun("sin" ,1,lbll_c_sin);
  lbll_reg_cfun("cos" ,1,lbll_c_cos);
  lbll_reg_cfun(">>"  ,0,lbll_c_echo);
  lbll_reg_cfun(">>|" ,0,lbll_c_echn);
  lbll_reg_cfun("ntos",1,lbll_c_ntos);
  lbll_reg_cfun("ston",0,lbll_c_ston);
  lbll_reg_cfun("droq",1,lbll_c_droq);
  lbll_reg_cfun("peek",1,lbll_c_peek);
  lbll_reg_cfun("roll",2,lbll_c_roll);
  lbll_reg_cfun("rev" ,1,lbll_c_rev );
  lbll_reg_cfun("swap",2,lbll_c_swap);
  lbll_reg_cfun("rand",0,lbll_c_rand);
  lbll_reg_cfun("srnd",1,lbll_c_srand);
  lbll_reg_cfun("^^",  2,lbll_c_fill);
  lbll_reg_cfun("edit",2,lbll_c_edit);
  lbll_reg_cfun("dup" ,2,lbll_c_dup );

  lbll_reg_cfun("uand",2,lbll_c_band);
  lbll_reg_cfun("uor" ,2,lbll_c_bor );
  lbll_reg_cfun("ushl",2,lbll_c_bshl);
  lbll_reg_cfun("ushr",2,lbll_c_bshr);
  lbll_reg_cfun("uxor",2,lbll_c_bxor);
  lbll_reg_cfun("unot",1,lbll_c_bnot);
}
#endif
