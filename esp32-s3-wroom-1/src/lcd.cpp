#include "lcd.h"

#define I2C_SDA 3
#define I2C_SCL 46

LiquidCrystal_I2C lcd(0x27, 16, 4);

void LCDInit(void) {
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    LCDReady(false, false);
}

// S2: Hiển thị màn hình chờ và trạng thái các ngăn tủ
void LCDReady(bool slot1_occupied, bool slot2_occupied) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SMART LOCKER");

    lcd.setCursor(0, 1);
    lcd.print("Quet QR/Nhap ma");

    lcd.setCursor(0, 2);
    lcd.printf("Ngan 1: [%s]", slot1_occupied ? "CO HANG" : "TRONG");

    lcd.setCursor(0, 3);
    lcd.printf("Ngan 2: [%s]", slot2_occupied ? "CO HANG" : "TRONG");
}

// S3: Hiển thị thông tin đơn hàng khi quét QR
void LCDShowDeliveryInfo(const QRCode_Data &data, uint8_t slot_id) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("DH: %s", data.MaDH);

    lcd.setCursor(0, 1);
    lcd.printf("NV: %s", data.NVGH);

    lcd.setCursor(0, 2);
    lcd.printf("KH: %s", data.KH);

    lcd.setCursor(0, 3);
    lcd.printf("Chon Ngan: %d", slot_id);
}

// S4: Hiển thị giao diện nhập mã từ Keypad
void LCDShowKeypadInput(const char* input_buf) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NHAP MA NHAN HANG:");

    lcd.setCursor(0, 1);
    lcd.printf("> %s_", input_buf ? input_buf : "");

    lcd.setCursor(0, 2);
    lcd.print("[#]: Xac nhan");

    lcd.setCursor(0, 3);
    lcd.print("[*]: Xoa/Huy");
}

// S5: Mở ngăn tủ, hướng dẫn thao tác đóng/mở
void LCDShowDoorAction(uint8_t slot_id, uint8_t flag) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("MO NGAN SO: %d", slot_id);

    lcd.setCursor(0, 1);
    if (flag == 0) {
        lcd.print("Vui long dat hang");
    } else {
        lcd.print("Vui long lay hang");
    }

    lcd.setCursor(0, 2);
    lcd.print("va dong cua lai!");

    lcd.setCursor(0, 3);
    lcd.print("Cho dong cua...");
}

// S6 & S7: Thông báo giao dịch thành công
void LCDShowSuccess(uint8_t slot_id, uint8_t flag) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("== THANH CONG ==");

    lcd.setCursor(0, 1);
    if (flag == 0) {
        lcd.printf("Da giao Ngan %d", slot_id);
    } else {
        lcd.printf("Da lay tu Ngan %d", slot_id);
    }

    lcd.setCursor(0, 2);
    lcd.print("Cam on ban!");
}

// Hiển thị thông báo lỗi
void LCDShowError(const char* line1, const char* line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("*** THONG BAO ***");

    if (line1) {
        lcd.setCursor(0, 1);
        lcd.print(line1);
    }
    if (line2 && strlen(line2) > 0) {
        lcd.setCursor(0, 2);
        lcd.print(line2);
    }
}

void LCDPrintQR(const char *payload, int len) {
    QRCode_Data data = processReadData((const uint8_t*)payload, len);
    if (strlen(data.MaDH) > 0) {
        LCDShowDeliveryInfo(data, 1);
    }
}
