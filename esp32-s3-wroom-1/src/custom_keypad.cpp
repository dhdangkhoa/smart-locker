#include "custom_keypad.h"

// Sử dụng kênh I2C phần cứng thứ hai của ESP32-S3 (Wire1)
// để tách biệt hoàn toàn với màn hình LCD (sử dụng Wire - cổng 0 trên GPIO 3/46)
#define WireKeypad Wire1

static const byte ROWS = 4;
static const byte COLS = 4;

static const char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

static uint8_t s_keypad_addr = 0x20;
static bool s_is_connected = false;

// Biến hỗ trợ chống dội (debounce) và chỉ trả về ký tự 1 lần khi nhấn
static char s_last_raw_key = '\0';
static char s_debounced_key = '\0';
static unsigned long s_last_debounce_time = 0;
static bool s_key_released = true;

bool Keypad_IsConnected(void) {
    return s_is_connected;
}

void Keypad_Init(void) {
    // Khởi tạo cổng Wire1 với chân SDA=21, SCL=14 ở tần số 100 kHz chuẩn
    WireKeypad.begin(KEYPAD_I2C_SDA, KEYPAD_I2C_SCL, 100000);

    // Tự động dò địa chỉ I2C của mạch mở rộng PCF8574 (0x20..0x27) hoặc PCF8574A (0x38..0x3F)
    const uint8_t candidate_addrs[] = {
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
    };

    s_is_connected = false;
    for (uint8_t i = 0; i < sizeof(candidate_addrs); i++) {
        WireKeypad.beginTransmission(candidate_addrs[i]);
        if (WireKeypad.endTransmission() == 0) {
            s_keypad_addr = candidate_addrs[i];
            s_is_connected = true;
            break;
        }
    }

    if (s_is_connected) {
        Serial.printf("[Keypad] Da tim thay mach mo rong I2C tai dia chi: 0x%02X (SDA: %d, SCL: %d)\n",
                      s_keypad_addr, KEYPAD_I2C_SDA, KEYPAD_I2C_SCL);
        // Đặt tất cả chân P0..P7 lên mức HIGH (0xFF - trạng thái chờ / pullup)
        WireKeypad.beginTransmission(s_keypad_addr);
        WireKeypad.write(0xFF);
        WireKeypad.endTransmission();
    } else {
        Serial.printf("[Keypad] CANH BAO: Khong tim thay mach mo rong I2C tren SDA=%d, SCL=%d!\n",
                      KEYPAD_I2C_SDA, KEYPAD_I2C_SCL);
    }
}

// Quét ma trận phím qua mạch mở rộng PCF8574
static char Keypad_ScanRaw(void) {
    if (!s_is_connected) return '\0';

    for (uint8_t r = 0; r < ROWS; r++) {
        // Kéo hàng r xuống LOW (0), các hàng khác giữ HIGH (1).
        // 4 bit cao (P4..P7 - Cột) giữ mức HIGH (1) làm ngõ vào nhận tín hiệu.
        uint8_t out_byte = 0xF0 | (~(1 << r) & 0x0F);

        WireKeypad.beginTransmission(s_keypad_addr);
        WireKeypad.write(out_byte);
        if (WireKeypad.endTransmission() != 0) {
            return '\0';
        }

        // Đọc lại trạng thái 8 chân từ PCF8574
        WireKeypad.requestFrom((uint8_t)s_keypad_addr, (uint8_t)1);
        if (WireKeypad.available()) {
            uint8_t in_byte = WireKeypad.read();

            // Kiểm tra các chân Cột (P4..P7).
            // Nếu có phím tại giao điểm (r, c) được nhấn, chân cột đó sẽ bị kéo xuống mức LOW (bit = 0).
            for (uint8_t c = 0; c < COLS; c++) {
                if ((in_byte & (1 << (4 + c))) == 0) {
                    // Trả lại trạng thái 0xFF cho PCF8574 trước khi thoát
                    WireKeypad.beginTransmission(s_keypad_addr);
                    WireKeypad.write(0xFF);
                    WireKeypad.endTransmission();
                    return hexaKeys[r][c];
                }
            }
        }
    }

    // Đưa tất cả các chân về HIGH khi không có phím nào nhấn
    WireKeypad.beginTransmission(s_keypad_addr);
    WireKeypad.write(0xFF);
    WireKeypad.endTransmission();

    return '\0';
}

char Keypad_GetKey(void) {
    char raw = Keypad_ScanRaw();
    unsigned long now = millis();

    // Phát hiện thay đổi trạng thái đọc từ bàn phím
    if (raw != s_last_raw_key) {
        s_last_debounce_time = now;
        s_last_raw_key = raw;
    }

    // Chờ tín hiệu ổn định tối thiểu 20ms để chống dội (debounce)
    if ((now - s_last_debounce_time) >= 20) {
        if (raw != s_debounced_key) {
            s_debounced_key = raw;
            if (s_debounced_key == '\0') {
                s_key_released = true; // Đã nhả phím hoàn toàn
            }
        }
    }

    // Chỉ kích hoạt trả về ký tự 1 lần duy nhất cho mỗi chu kỳ nhấn phím
    if (s_debounced_key != '\0' && s_key_released) {
        s_key_released = false;
        return s_debounced_key;
    }

    return '\0';
}
