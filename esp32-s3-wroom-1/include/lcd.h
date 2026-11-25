#ifndef LCD_H
#define LCD_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "rw_to_flash.h"

extern LiquidCrystal_I2C lcd;

void LCDInit(void);
void LCDReady(bool slot1_occupied, bool slot2_occupied);
void LCDShowDeliveryInfo(const QRCode_Data &data, uint8_t slot_id);
void LCDShowKeypadInput(const char* input_buf);
void LCDShowDoorAction(uint8_t slot_id, uint8_t flag);
void LCDShowSuccess(uint8_t slot_id, uint8_t flag);
void LCDShowError(const char* line1, const char* line2 = "");
void LCDPrintQR(const char *payload, int len);

#endif // LCD_H
