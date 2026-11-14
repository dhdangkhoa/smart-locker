#pragma once

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

void LCDInit();
void LCDPrintQR(const char *payload, int len);
void LCDReady();


