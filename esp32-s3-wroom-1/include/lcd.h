#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

void LCDInit();
void LCDPrintQR(const char *payload, int len);
void LCDReady();


#endif // LCD_H