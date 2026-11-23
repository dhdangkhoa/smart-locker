#include "lcd.h"
#include "rw_to_flash.h"


#define I2C_SDA 3
#define I2C_SCL 46

LiquidCrystal_I2C lcd(0x27, 16, 4);

void LCDInit() {
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    LCDReady();
}

void LCDReady() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ready...");
}

void LCDPrintQR(const char *payload, int len) {
    lcd.clear();


    QRCode_Data data = processReadData((const uint8_t*)payload, len);

    if (strlen(data.MaDH) > 0) {

        lcd.setCursor(0, 0);
        lcd.printf("MaDH: %s", data.MaDH);

        lcd.setCursor(0, 1);
        lcd.printf("NV: %s", data.NVGH);

        lcd.setCursor(0, 2);
        lcd.printf("KH: %s", data.KH);
    }
}
