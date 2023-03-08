#include <SD.h>

#include "polyfill.h"

#define LBLL_USER_IMPL_LIB_C

#include "lbll.h"

#include "oled.h"

#include "wrap.h"

#define SD_SS_PIN 23

#define BASEPATH "lbll/"

void run_file(char* path){
  oled_clear();

  C_FILE* fd = C_fopen(path,"r");
  
  C_printf("parsing %s...\n", path);

  C_FILE* fo = C_open_memstream(NULL,NULL);

  lbll_parse(fd,fo);

  C_fclose(fd);

  C_fseek(fo,0,SEEK_END);

  C_printf("parsing used: %ld B\n",C_ftell(fo));

  C_fseek(fo,0,SEEK_SET);

  lbll_print_instrs(fo);
  
  C_printf("done parsing, now executing...\n");

  C_fseek(fo,0,SEEK_SET);

  lbll_stack = C_open_memstream(NULL,NULL);
  lbll_frame = C_open_memstream(NULL,NULL);

  lbll_run(fo);

  lbll_print_stack();
  lbll_print_frame();
  C_fseek(lbll_stack,0,SEEK_END);
  C_fseek(lbll_frame,0,SEEK_END);

  C_printf("stack used: %ld B\n",C_ftell(lbll_stack));
  C_printf("frame used: %ld B\n",C_ftell(lbll_frame));

  oled_flush();

  Serial.println("ok.");

  C_fclose(lbll_stack);
  C_fclose(lbll_frame);
  C_fclose(fo);
  
}



void setup() {
  Serial.begin(9600);

  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);

  SD.begin(SD_SS_PIN);

  oled_init();

  lbll_init_stdlib();
  
  wrap_init();

  if (SD.exists(BASEPATH"index.lbl")){
    Serial.println("found index file, running...");
    run_file(BASEPATH"index.lbl");
  }

  oled_3x7string_direct(0 ,0, "waiting for serial connection...", 0);

  int i = 0;
  while (!Serial){}
  File root = SD.open(BASEPATH);

  oled_clear();
  while (1){
    File entry =  root.openNextFile();
    if (!entry){
      break;
    }
    if (i < 16){
      oled_3x7string_direct(i%8, (i/8) * 64 , entry.name(), 0);
    }
    Serial.println(entry.name());
    entry.close();
    i++;
  }
  Serial.println("enter base name of file to run.");

  
}

void loop() {
  char buf[32] = BASEPATH;
  int len = strlen(buf);
  while (1){
    if (Serial.available()){
      int b = Serial.read();
      if (b == '\n' || b == '\r'){
        break;
      }
      buf[len++] = b;
    }
  }
  buf[len++] = '.';
  buf[len++] = 'l';
  buf[len++] = 'b';
  buf[len++] = 'l';
  buf[len] = 0;

  Serial.println(buf);
  
  run_file(buf);
}
