#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

// ==========================================
// ĐỊNH NGHĨA CHÂN CẢM BIẾN, CÔNG TẮC & NÚT BẤM
// ==========================================
#define PIN_SW_DOOR_1      38   // LOW: Cửa 1 mở     | HIGH: Cửa 1 đóng
#define PIN_SW_DOOR_2      39   // LOW: Cửa 2 mở     | HIGH: Cửa 2 đóng
#define PIN_SW_PARCEL_1    40   // LOW: Ngăn 1 trống | HIGH: Ngăn 1 có hàng
#define PIN_SW_PARCEL_2    41   // LOW: Ngăn 2 trống | HIGH: Ngăn 2 có hàng

#define PIN_LED_SLOT_1     42   // HIGH: LED 1 sáng  | LOW: LED 1 tắt
#define PIN_LED_SLOT_2     2    // HIGH: LED 2 sáng  | LOW: LED 2 tắt

#define PIN_BTN_PWR        1    // Nút nguồn (LOW: Nhấn)
#define PIN_BTN_QR1        14   // Nút QR1   (LOW: Nhấn)
#define PIN_BTN_QR2        21   // Nút QR2   (LOW: Nhấn)
#define PIN_BTN_QR3        47   // Nút QR3   (LOW: Nhấn)

#define DEBOUNCE_TIME_MS   30     // Chống dội 30ms
#define DOOR_OPEN_TIMEOUT  10000  // Chờ mở cửa tối đa 10 giây

// ==========================================
// CẤU TRÚC DỮ LIỆU TRẠNG THÁI NGĂN TỦ
// ==========================================
typedef enum {
    DOOR_CLOSED = 0,
    DOOR_OPEN   = 1
} DoorState_t;

typedef enum {
    SLOT_EMPTY    = 0,
    SLOT_RESERVED = 1,
    SLOT_OCCUPIED = 2,
    SLOT_FAULT    = 3
} SlotState_t;

typedef struct {
    uint8_t     slot_id;      // 1 hoặc 2
    SlotState_t state;        // EMPTY, RESERVED, OCCUPIED, FAULT
    DoorState_t door;         // DOOR_OPEN hoặc DOOR_CLOSED
    bool        has_parcel;   // true: Có hàng, false: Trống
    bool        led_on;       // true: LED sáng, false: LED tắt
} SlotData_t;

extern SlotData_t g_slots[2];

// ==========================================
// CÁC HÀM XỬ LÝ CẢM BIẾN
// ==========================================
void Sensor_Init(void);
DoorState_t Sensor_GetDoorState(uint8_t slot_id);
bool Sensor_HasParcel(uint8_t slot_id);
void Sensor_SetSlotLED(uint8_t slot_id, bool turn_on);
void Sensor_UpdateAllSlots(void);
uint8_t Sensor_FindLowestEmptySlot(void);
bool Sensor_WaitForDoorState(uint8_t slot_id, DoorState_t target_state, uint32_t timeout_ms);
bool Sensor_IsButtonPressed(uint8_t btn_pin);

#endif // SENSORS_H
