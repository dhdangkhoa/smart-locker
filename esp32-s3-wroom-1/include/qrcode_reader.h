#ifndef QRCODE_READER_H
#define QRCODE_READER_H

#include <quirc.h>
#include <Arduino.h>
#include "esp_camera.h"
#include "rw_to_flash.h"
#include "lcd.h"

extern volatile bool isAwake;
extern volatile unsigned long wakeUpTimer;

// Quản lý trạng thái và truyền mã QR đã quét sang FSM chính
bool QR_HasNewCode(QRCode_Data &outData);
void QR_ClearCode(void);
void QR_SetScanningEnabled(bool enabled);

void qrScannerTask(void *pvParameters);

#endif // QRCODE_READER_H
