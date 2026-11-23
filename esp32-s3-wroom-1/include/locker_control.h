#ifndef LOCKER_CONTROL_H
#define LOCKER_CONTROL_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"

// ==========================================
// ĐỊNH NGHĨA CHÂN SERVO VÀ BUZZER
// (Lưu ý: Camera dùng các chân 4..18, tránh dùng trùng)
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

// Khởi tạo toàn bộ hệ thống tủ (bao gồm cả Sensors, Servo và Buzzer)
void Locker_Init(void);

// Quy trình mở tủ và xác nhận giao hàng
bool Locker_ProcessDelivery(uint8_t slot_id);
void Locker_RunSlotDelivery(uint8_t qr_button_num);

#endif // LOCKER_CONTROL_H
