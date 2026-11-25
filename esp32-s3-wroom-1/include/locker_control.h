#ifndef LOCKER_CONTROL_H
#define LOCKER_CONTROL_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"
#include "lcd.h"

// ==========================================
// ĐỊNH NGHĨA CHÂN SERVO VÀ BUZZER
// ==========================================
#define PIN_SERVO_1        48   // Chân tín hiệu Servo khóa ngăn 1
#define PIN_SERVO_2        45   // Chân tín hiệu Servo khóa ngăn 2
#define PIN_BUZZER         47   // Chân điều khiển còi Buzzer

// GÓC QUAY CỦA SERVO
#define SERVO_LOCK_ANGLE   0    // Góc đóng chốt khóa
#define SERVO_UNLOCK_ANGLE 90   // Góc mở chốt khóa

// ==========================================
// CÁC HÀM ĐIỀU KHIỂN CHẤP HÀNH & QUY TRÌNH TỦ
// ==========================================
void Buzzer_Beep(uint16_t duration_ms);
void Servo_ControlInit(void);
void Servo_Unlock(uint8_t slot_id);
void Servo_Lock(uint8_t slot_id);

void Locker_Init(void);

// S5: Mở ngăn tủ, chờ mở và đóng cửa, kiểm tra cảm biến hàng
// flag = 0: Luồng gửi hàng (kiểm tra sau khi đóng phải CÓ HÀNG)
// flag = 1: Luồng nhận hàng (kiểm tra sau khi đóng phải TRỐNG)
bool Locker_ProcessDoor(uint8_t slot_id, uint8_t flag);

// Các hàm tương thích ngược
bool Locker_ProcessDelivery(uint8_t slot_id);
void Locker_RunSlotDelivery(uint8_t slot_id);

#endif // LOCKER_CONTROL_H
