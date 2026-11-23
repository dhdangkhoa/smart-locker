#include "locker_control.h"
#include <ESP32Servo.h>

// Khai báo đối tượng Servo cho các ngăn tủ
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
        Serial.printf("-> Mo khoa Servo Ngan 1 (Goc %d do)\n", SERVO_UNLOCK_ANGLE);
    } else if (slot_id == 2) {
        servo_slot_2.write(SERVO_UNLOCK_ANGLE);
        Serial.printf("-> Mo khoa Servo Ngan 2 (Goc %d do)\n", SERVO_UNLOCK_ANGLE);
    }
    Buzzer_Beep(100);
}

void Servo_Lock(uint8_t slot_id) {
    if (slot_id == 1) {
        servo_slot_1.write(SERVO_LOCK_ANGLE);
        Serial.printf("-> Khoa chot Servo Ngan 1 (Goc %d do)\n", SERVO_LOCK_ANGLE);
    } else if (slot_id == 2) {
        servo_slot_2.write(SERVO_LOCK_ANGLE);
        Serial.printf("-> Khoa chot Servo Ngan 2 (Goc %d do)\n", SERVO_LOCK_ANGLE);
    }
    Buzzer_Beep(50);
}

void Locker_Init(void) {
    Sensor_Init();
    Servo_ControlInit();
    Serial.println(">>> Khoi tao he thong Smart Locker thanh cong!");
}

bool Locker_ProcessDelivery(uint8_t target_slot) {
    if (target_slot != 1 && target_slot != 2) return false;

    Serial.printf("He thong chon NGAN SO %d.\n", target_slot);
    Servo_Unlock(target_slot);

    // Kiểm tra nếu công tắc cửa đang mở sẵn
    if (Sensor_GetDoorState(target_slot) == DOOR_OPEN) {
        Serial.printf("Cong tac CUA %d dang o vi tri MO san!\n", target_slot);
    } else {
        Serial.printf("Dang cho ban mo CUA %d trong 10s...\n", target_slot);
    }

    if (Sensor_WaitForDoorState(target_slot, DOOR_OPEN, DOOR_OPEN_TIMEOUT)) {
        Serial.printf("-> Da xac nhan CUA %d MO! Vui long dat hang va dong cua...\n", target_slot);

        // Chờ người dùng đóng cửa
        Sensor_WaitForDoorState(target_slot, DOOR_CLOSED, 0);
        Servo_Lock(target_slot);
        Serial.printf("Da xac nhan CUA %d DONG! Khoa chot ngan %d.\n", target_slot, target_slot);

        // Cập nhật và kiểm tra hàng sau khi đóng cửa
        Sensor_UpdateAllSlots();
        if (Sensor_HasParcel(target_slot)) {
            Serial.printf("Giao hang vao NGAN %d THANH CONG! LED %d SANG.\n\n", target_slot, target_slot);
            Buzzer_Beep(150);
            delay(100);
            Buzzer_Beep(150);
            return true;
        } else {
            Serial.printf("NGAN %d van TRONG sau khi dong cua! LED %d TAT.\n\n", target_slot, target_slot);
            Buzzer_Beep(500);
            return false;
        }
    } else {
        Serial.printf("Qua 10 giay khong mo CUA %d! Huy giao dich, khoa lai chot.\n\n", target_slot);
        Servo_Lock(target_slot);
        Buzzer_Beep(400);
        return false;
    }
}

void Locker_RunSlotDelivery(uint8_t qr_button_num) {
    Serial.printf("\n[Locker] Yeu cau giao hang (Nguon/Button: %d)!\n", qr_button_num);
    Buzzer_Beep(200);

    // Tìm ngăn trống có số thứ tự nhỏ nhất
    uint8_t target_slot = Sensor_FindLowestEmptySlot();

    if (target_slot == 0) {
        Serial.println("Ca 2 ngan deu da CO HANG! Het ngan trong!\n");
        for (int i = 0; i < 3; i++) {
            Buzzer_Beep(100);
            delay(100);
        }
        return;
    }

    Locker_ProcessDelivery(target_slot);
}
