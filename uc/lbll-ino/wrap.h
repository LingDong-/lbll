void c_px(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  oled_px(x,y);
}
void c_unpx(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  oled_unpx(x,y);
}
void c_pxat(){
  int y = (int)lbll_pop();
  int x = (int)lbll_pop();
  int v = oled_pxat(x,y);
  lbll_push((float)v);
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
  oled_3x7string(row,col,buf);
}

void c_clr(){
  int i1 = (int)lbll_pop();
  int i0 = (int)lbll_pop();
  oled_clear_rows(i0,i1);
}

void c_show(){
  oled_flush();
}

void c_iomo(){
  int m = (int)lbll_pop();
  int n = (int)lbll_pop();
  pinMode(n,m);
}
void c_iowd(){
  int m = (int)lbll_pop();
  int n = (int)lbll_pop();
  digitalWrite(n,m);
}
void c_iord(){
  int n = (int)lbll_pop();
  digitalRead(n);
}
void c_iowa(){
  int m = (int)lbll_pop();
  int n = (int)lbll_pop();
  analogWrite(n,m);
}
void c_iora(){
  int n = (int)lbll_pop();
  analogRead(n);
}
void c_w8ms(){
  int n = (int)lbll_pop();
  delay(n);
}
void c_w8us(){
  int n = (int)lbll_pop();
  delayMicroseconds(n);
}
void c_tmms(){
  float x = millis();
  lbll_push(x);
}
void c_tmus(){
  float x = micros();
  lbll_push(x);
}



void wrap_init(){
  lbll_reg_cfun("px"   ,2, c_px );
  lbll_reg_cfun("unpx" ,2, c_unpx);
  lbll_reg_cfun("pxat" ,2, c_pxat);
  lbll_reg_cfun("text" ,2, c_text);
  lbll_reg_cfun("show" ,0, c_show);
  lbll_reg_cfun("clr"  ,2, c_clr );
  lbll_reg_cfun("iomo" ,2, c_iomo);
  lbll_reg_cfun("iowd" ,2, c_iowd);
  lbll_reg_cfun("iord" ,1, c_iord);
  lbll_reg_cfun("iowa" ,2, c_iowa);
  lbll_reg_cfun("iora" ,1, c_iora);

  lbll_reg_cfun("w8ms" ,1, c_w8ms);
  lbll_reg_cfun("w8us" ,1, c_w8us);
  lbll_reg_cfun("tmms" ,0, c_tmms);
  lbll_reg_cfun("tmus" ,0, c_tmus);
}
