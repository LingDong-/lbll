//#define SOFTWARE_I2C
//#define HARDWARE_I2C_USE_PA0809

#define I2C_ADDRESS 0x3C

#ifdef SOFTWARE_I2C
  #define SDA_PIN 16
  #define SCL_PIN 17
  #include <SoftWire.h>
  SoftWire WIRE(SDA_PIN, SCL_PIN);
  #define I2C_BUFFER_SIZE 129
  char ibuffer[I2C_BUFFER_SIZE];
#else
  #include <Wire.h>
  #ifdef HARDWARE_I2C_USE_PA0809
    TwoWire WIRE(&PERIPH_WIRE1,PIN_WIRE1_SDA, PIN_WIRE1_SCL);
  #else
    #define WIRE Wire
  #endif
  #define I2C_BUFFER_SIZE 32
#endif

static const uint8_t FONT[] = {0,0,0,0,125,0,96,0,96,127,20,127,50,107,38,39,8,115,62,107,42,0,96,0,28,34,65,65,34,28,107,62,107,8,62,8,2,3,0,8,8,8,3,3,0,3,28,96,62,65,62,33,127,1,35,69,57,34,73,54,60,4,127,122,73,78,62,73,38,64,79,112,54,73,54,50,73,62,0,20,0,2,20,0,8,20,34,20,20,20,34,20,8,32,77,48,63,65,125,127,80,127,127,73,54,62,65,34,127,65,62,127,73,65,127,72,72,62,65,46,127,8,127,65,127,65,2,1,126,127,8,119,127,1,1,127,62,127,127,64,127,127,65,127,127,72,48,62,69,63,127,72,55,50,73,38,64,127,64,127,1,127,124,3,124,127,14,127,119,8,119,112,15,112,71,73,113,127,65,65,96,28,3,65,65,127,32,64,32,1,1,1,64,32,0};

uint8_t OLED_BUFF[1024] = {0};
uint8_t OLED_DIRT[128] = {0};
uint8_t oled_clr_data[38] = {0,0,0x10,0,0,0x40};

void oled_i2c_write_bytes(const uint8_t *data_wr, size_t size){
  WIRE.beginTransmission(I2C_ADDRESS);
  WIRE.write(data_wr,size);
  WIRE.endTransmission();
}


void oled_clear_rows(int i0, int i1){
  oled_clr_data[1] = 0;
  oled_clr_data[2] = 0x10;
  memset(oled_clr_data+6,0,32);
  for (int j = i0; j < i1; ++j) {
    oled_clr_data[4] = 0xB0+(j);
    oled_i2c_write_bytes(oled_clr_data, 3);
    oled_i2c_write_bytes(oled_clr_data+3, 2);
    oled_i2c_write_bytes(oled_clr_data+5, 32);
    oled_i2c_write_bytes(oled_clr_data+5, 32);
    oled_i2c_write_bytes(oled_clr_data+5, 32);
    oled_i2c_write_bytes(oled_clr_data+5, 32);
    oled_i2c_write_bytes(oled_clr_data+5, 5);
  }
}

void oled_clear(){
  oled_clear_rows(0,8);
}


void oled_init(void){

  static uint8_t data1[] = {
    0,0xae, // display off
    0,0xa8,0x3f, // set multiplex ratio, ratio 63
    0,0xd3,0x00, // set display offset, no offset
    0,0x40, // set display start line
    0,0xa0, // set segment remap col 127 to seg 0
    0,0xc8, // set COM output reverse
    0,0xda,0x12, // COM pin config, alt bottom to top
    0,0x81,0xff, // set contrast, max contrast
    0,0xa4, // resume to RAM display
    0,0xa6, // normal non-inverted display
    0,0xd5,0x80, // set clock divider, default
    0,0x8d,0x14, // set charge pump, enable
  };
  static uint8_t data2[] = {
    0,0x20,0x02, // set memory mode, page addressing
    0,0xaf // display on
  };
  
  WIRE.begin();
  
  oled_i2c_write_bytes(data1, sizeof(data1));
  oled_i2c_write_bytes(data2, sizeof(data2));
  delay(100);
  oled_clear();
}


void oled_3x7string_direct(uint8_t row,uint8_t col,const char* str, int inverted) {
  static uint8_t index, pointer;
  static char chr;
  uint8_t data[5] = {0,0x00+(col & 0x0F), 0x10+((col >> 4) & 0x0F), 0, 0xB0+row};
  
  oled_i2c_write_bytes(data,  3);
  oled_i2c_write_bytes(data+3,2);
  index = 0;
  while (1) {
    chr = str[index];
    if (chr == '\0')
      break;
    if (chr >= 'a'){
      chr -= ('a'-'A');
    }
    pointer = chr-' ';
    data[0] = 0x40;
    data[1] = FONT[3*pointer  ];
    data[2] = FONT[3*pointer+1];
    data[3] = FONT[3*pointer+2];
    data[4] = 0;
    if (inverted){
      data[1] = ~data[1];
      data[2] = ~data[2];
      data[3] = ~data[3];
      data[4] = ~data[4];
    }

    oled_i2c_write_bytes( data, 5);
    ++index;
  }
}

void oled_3x7string(uint8_t row,uint8_t col,const char* buf) {
  int i = 0;
  int chr, pointer;
  while ( (chr = buf[i]) ){
    char chr = buf[i];
    if (chr == '\0')
      break;
    if (chr >= 'a'){
      chr -= ('a'-'A');
    }
    pointer = chr-' ';
    OLED_BUFF[row*128+col+4*i  ] = FONT[3*pointer  ];
    OLED_BUFF[row*128+col+4*i+1] = FONT[3*pointer+1];
    OLED_BUFF[row*128+col+4*i+2] = FONT[3*pointer+2];
    OLED_BUFF[row*128+col+4*i+3] = 0;
    OLED_DIRT[col+4*i  ] |= 1<<row;
    OLED_DIRT[col+4*i+1] |= 1<<row;
    OLED_DIRT[col+4*i+2] |= 1<<row;
    OLED_DIRT[col+4*i+3] |= 1<<row;
    i++;
  }
}


void oled_px(int x, int y){
  if (x < 0 || x >= 128) return;
  if (y < 0 || y >= 64 ) return;
  y = 63-y;
  int row = y / 8;
  OLED_BUFF[row * 128 + x] |= (1 << (y % 8));
  OLED_DIRT[x] |= 1<<row;
}


void oled_unpx(int x, int y){
  if (x < 0 || x >= 128) return;
  if (y < 0 || y >= 64 ) return;
  y = 63-y;
  int row = y / 8;
  OLED_BUFF[row * 128 + x] &= ~(1 << (y % 8));
  OLED_DIRT[x] |= 1<<row;
}


int oled_pxat(int x, int y){
  if (x < 0 || x >= 128) return 0;
  if (y < 0 || y >= 64 ) return 0;
  y = 63-y;
  int row = y / 8;
  return (OLED_BUFF[row * 128 + x] >> (y % 8)) & 1;
}

void oled_flush(){
  
  for (int i = 0; i < 8; i++){
    int j = 0;
    while (j < 128){
      int d = (OLED_DIRT[j]>>i) & 1;
      if (d){
        OLED_DIRT[j] &= ~(1<<i);
        oled_clr_data[1] = 0x00+(j & 0x0F);
        oled_clr_data[2] = 0x10+((j >> 4) & 0x0F);
        oled_clr_data[4] = 0xB0+i;
        oled_clr_data[6] = OLED_BUFF[i*128+j];
        oled_i2c_write_bytes(oled_clr_data,  3);
        oled_i2c_write_bytes(oled_clr_data+3,2);
        int k = 0;
        for (k = 0; k < 30; k++){
          j++;
          if (j >= 128) break;
          int d = (OLED_DIRT[j]>>i) & 1;
          if (!d) break;
          OLED_DIRT[j] &= ~(1<<i);
          oled_clr_data[7+k] = OLED_BUFF[i*128+j];
        }
        oled_i2c_write_bytes(oled_clr_data+5,k+2);
      }else{
        j++;
      }
    }
  }
}
