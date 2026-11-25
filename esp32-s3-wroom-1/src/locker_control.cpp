#include "locker_control.h"
#include <ESP32Servo.h>

static Servo servo_slot_1;
static Servo servo_slot_2;

void Buzzer_Beep(uint16_t duration_ms) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(duration_ms);
    digitalWrite(PIN_BUZZER, LOW);
}

void Servo_ControlInit(void) {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    servo_slot_1.attach(PIN_SERVO_1);
    servo_slot_2.attach(PIN_SERVO_2);

    servo_slot_1.write(SERVO_LOCK_ANGLE);
    servo_slot_2.write(SERVO_LOCK_ANGLE);
}

void Servo_Unlock(uint8_t slot_id) {
    if (slot_id == 1) {
        servo_slot_1.write(SERVO_UNLOCK_ANGLE);
        Serial.printf("[Servo] Mo khoa Ngan 1 (Goc %d do)\n", SERVO_UNLOCK_ANGLE);
    } else if (slot_id == 2) {
        servo_slot_2.write(SERVO_UNLOCK_ANGLE);
        Serial.printf("[Servo] Mo khoa Ngan 2 (Goc %d do)\n", SERVO_UNLOCK_ANGLE);
    }
    Buzzer_Beep(100);
}

void Servo_Lock(uint8_t slot_id) {
    if (slot_id == 1) {
        servo_slot_1.write(SERVO_LOCK_ANGLE);
        Serial.printf("[Servo] Khoa chot Ngan 1 (Goc %d do)\n", SERVO_LOCK_ANGLE);
    } else if (slot_id == 2) {
        servo_slot_2.write(SERVO_LOCK_ANGLE);
        Serial.printf("[Servo] Khoa chot Ngan 2 (Goc %d do)\n", SERVO_LOCK_ANGLE);
    }
    Buzzer_Beep(50);
}

void Locker_Init(void) {
    Sensor_Init();
    Servo_ControlInit();
    Serial.println(">>> Khoi tao he thong Locker (Sensors + Servos + Buzzer) thanh cong!");
}

// S5: Mở ngăn tủ, chờ mở và đóng cửa, kiểm tra cảm biến hàng
bool Locker_ProcessDoor(uint8_t slot_id, uint8_t flag) {
    if (slot_id != 1 && slot_id != 2) return false;

    Serial.printf("\n[S5] Bat dau mo Ngan %d (flag = %d: %s)\n",
                  slot_id, flag, (flag == 0 ? "Giao hang" : "Nhan hang"));

    // 1. Mở chốt servo và hiển thị hướng dẫn trên LCD
    Servo_Unlock(slot_id);
    LCDShowDoorAction(slot_id, flag);

    // 2. Chờ mở cửa (tối đa 10s)
    Serial.printf("[S5] Cho nguoi dung mo CUA %d trong 10s...\n", slot_id);
    if (!Sensor_WaitForDoorState(slot_id, DOOR_OPEN, DOOR_OPEN_TIMEOUT)) {
        Serial.printf("[S5] Timeout! Khong mo CUA %d trong 10s. Khoa lai chot.\n", slot_id);
        Servo_Lock(slot_id);
        LCDShowError("Qua thoi gian!", "Da khoa lai chot");
        Buzzer_Beep(400);
        delay(2000);
        return false;
    }

    Serial.printf("[S5] CUA %d da MO. Cho thao tac va dong cua lai...\n", slot_id);

    // 3. Chờ đóng cửa lại
    Sensor_WaitForDoorState(slot_id, DOOR_CLOSED, 0);
    Servo_Lock(slot_id);
    Serial.printf("[S5] CUA %d da DONG. Da khoa chot.\n", slot_id);

    // 4. Kiểm tra trạng thái hàng trong ngăn
    delay(200); // Đợi ổn định cảm biến công tắc hành trình
    Sensor_UpdateAllSlots();
    bool has_parcel = Sensor_HasParcel(slot_id);

    if (flag == 0) {
        // Luồng gửi hàng: Sau khi đóng cửa, BẮT BUỘC PHẢI CÓ HÀNG
        if (has_parcel) {
            Serial.printf("[S5] Xac nhan: Da co hang trong Ngan %d.\n", slot_id);
            return true;
        } else {
            Serial.printf("[S5] Loi: Ngan %d van trong (khong bo hang vao)!\n", slot_id);
            LCDShowError("Loi: Ngan trong!", "Chua dat hang!");
            Buzzer_Beep(500);
            delay(2000);
            return false;
        }
    } else {
        // Luồng nhận hàng: Sau khi đóng cửa, BẮT BUỘC PHẢI TRỐNG (đã lấy hàng ra)
        if (!has_parcel) {
            Serial.printf("[S5] Xac nhan: Ngan %d da duoc lay het hang.\n", slot_id);
            return true;
        } else {
            Serial.printf("[S5] Loi: Hang van con trong Ngan %d (quen lay)!\n", slot_id);
            LCDShowError("Hang chua lay!", "Vui long thu lai.");
            Buzzer_Beep(500);
            delay(2000);
            return false;
        }
    }
}

bool Locker_ProcessDelivery(uint8_t slot_id) {
    return Locker_ProcessDoor(slot_id, 0);
}

void Locker_RunSlotDelivery(uint8_t slot_id) {
    Locker_ProcessDoor(slot_id, 0);
}
