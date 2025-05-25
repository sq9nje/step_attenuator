#include "display.h"

#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void display_init(void) {
  u8g2.begin();
}

void display_splash(void) {
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);

  // Title
  u8g2.setFont(u8g2_font_spleen8x16_mr);
  u8g2.drawStr(20, 16, "STEP");
  u8g2.drawStr(12, 32, "ATTENUATOR");
  
  // Version/Model info
  u8g2.setFont(u8g2_font_spleen5x8_mr);
  u8g2.drawStr(40, 48, "v1.0");
  
  // Loading indicator
  u8g2.setFont(u8g2_font_squeezed_b6_tr);
  u8g2.drawStr(25, 60, "Initializing...");
  
  u8g2.sendBuffer();
}

void display_update(uint8_t attenuation, const char *net_status) {
  u8g2.clearBuffer();          // clear the internal memory

  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);

  // IP Address
  u8g2.setFont(u8g2_font_spleen5x8_mr); 
  u8g2.drawStr(0,10,net_status);   

  // Attenuation label
  u8g2.drawRBox(0, 22, 23, 10, 2);
  u8g2.setFont(u8g2_font_squeezed_b6_tr);
  u8g2.setDrawColor(2);
  u8g2.drawStr(4,30,"ATT");
  u8g2.setDrawColor(1);

  // Attenuation value
  u8g2.setFont(u8g2_font_spleen16x32_mr); 
  u8g2.setCursor(18, 58);
  if (attenuation < 100) u8g2.print(" ");
  if (attenuation < 10) u8g2.print(" ");
  u8g2.print(attenuation);
  u8g2.print(" dB"); 

  u8g2.sendBuffer();          // transfer internal memory to the display
}
