#ifndef CUSTOM_KEYPAD_H
#define CUSTOM_KEYPAD_H

#include <Arduino.h>
#include <Wire.h>

// ==============================================================================
// CẤU HÌNH GIAO TIẾP I2C CHO MẠCH MỞ RỘNG KEYPAD (PCF8574 / PCF8574A)
// ==============================================================================
#define KEYPAD_I2C_SDA     21   // Chân I2C SDA nối GPIO 21 của ESP32-S3
#define KEYPAD_I2C_SCL     14   // Chân I2C SCL nối GPIO 14 của ESP32-S3

// ==============================================================================
// SƠ ĐỒ KẾT NỐI BÀN PHÍM MA TRẬN 4x4 VỚI CÁC CHÂN P0 -> P7 TRÊN MẠCH MỞ RỘNG:
// - P0: Hàng 1 (Row 1)
// - P1: Hàng 2 (Row 2)
// - P2: Hàng 3 (Row 3)
// - P3: Hàng 4 (Row 4)
// - P4: Cột 1  (Col 1)
// - P5: Cột 2  (Col 2)
// - P6: Cột 3  (Col 3)
// - P7: Cột 4  (Col 4)
//
// Ma trận phím:
//        P4(C1)  P5(C2)  P6(C3)  P7(C4)
// P0(R1)   '1'     '2'     '3'     'A'
// P1(R2)   '4'     '5'     '6'     'B'
// P2(R3)   '7'     '8'     '9'     'C'
// P3(R4)   '*'     '0'     '#'     'D'
// ==============================================================================

void Keypad_Init(void);
char Keypad_GetKey(void);
bool Keypad_IsConnected(void);

#endif // CUSTOM_KEYPAD_H
