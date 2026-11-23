#include "custom_keypad.h"

const byte ROWS = 4;
const byte COLS = 4;

static char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// ==============================================================================
// CẢNH BÁO PHẦN CỨNG:
// Các chân 4, 5, 6, 7, 10, 11, 12, 13 trùng với chân dữ liệu Camera DVP ESP32-S3.
// Hãy điều chỉnh các chân này tương ứng với các chân bạn cắm thực tế trên mạch.
// ==============================================================================
static byte rowPins[ROWS] = {4, 5, 6, 7};
static byte colPins[COLS] = {10, 11, 12, 13};

static Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void Keypad_Init(void) {
    // Thư viện Keypad tự động quản lý chế độ chân
}

char Keypad_GetKey(void) {
    return customKeypad.getKey();
}

void receive_key(void) {
    char key = Keypad_GetKey();
    if (key) {
        Serial.printf("[Keypad] Phim nhan: %c\n", key);
    }
}
