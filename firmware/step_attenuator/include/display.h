#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <U8g2lib.h>
#include <Wire.h>

#include "config.h"

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;


void display_init(void);
void display_splash(void);
void display_update(uint8_t attenuation, const char *net_status);

#endif /* _DISPLAY_H_ */
