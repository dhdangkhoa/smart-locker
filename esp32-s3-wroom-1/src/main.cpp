#include "camera_init.h"
#include "qrcode_reader.h"
#include "locker_control.h"
#include "custom_keypad.h"
#include "rw_to_flash.h"
#include "lcd.h"

// Chân đánh thức khỏi Light Sleep (Nút nguồn PIN_BTN_PWR - GPIO 1)
#define WAKEUP_PIN PIN_BTN_PWR 
#define SCAN_TIMEOUT_MS 10000 
#define KEYPAD_TIMEOUT_MS 15000

// ==============================================================================
// ĐỊNH NGHĨA CÁC TRẠNG THÁI HỆ THỐNG THEO LƯU ĐỒ (raccoon.drawio)
// ==============================================================================
enum SystemState_t {
    S0_INIT,            // S0: Khởi tạo, đọc flash
    S1_SLEEP,           // S1: Ngủ
    S2_WAIT_INPUT,      // S2: Đánh thức, bật LCD, chờ QR/keypad
    S3_CHECK_DELIVERY,  // S3: Kiểm tra giao hàng (đọc QR, kiểm tra)
    S4_KEYPAD_INPUT,    // S4: Input từ keypad
    S5_DOOR_PROCESS,    // S5: Mở ngăn tủ, chờ đóng cửa
    S6_SAVE_ORDER,      // S6: Lưu đơn và số ngăn
    S7_CLEAR_ORDER      // S7: Xóa đơn hàng đã được lấy, trạng thái ngăn tủ thành trống
};

static SystemState_t currentState = S0_INIT;
static uint8_t currentFlag = 0;       // flag = 0: Gửi hàng | flag = 1: Nhận hàng
static uint8_t currentSlot = 0;       // 1 hoặc 2
static QRCode_Data currentOrder;      // Dữ liệu đơn hàng đang xử lý
static char keypadBuffer[16] = "";    // Bộ đệm chuỗi nhập từ bàn phím
static uint8_t keypadBufLen = 0;

volatile bool isAwake = true;
volatile unsigned long wakeUpTimer = 0;

void setupWakeupPin() {
    pinMode(WAKEUP_PIN, INPUT_PULLUP);
    gpio_wakeup_enable((gpio_num_t)WAKEUP_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
}

// S1: Ngủ (Light Sleep) và chờ ngắt đánh thức (IDLE Interrupt)
void enterSleepState() {
    isAwake = false;
    QR_SetScanningEnabled(false);

    Serial.println("\n[S1] He thong vao che do NGU (Light Sleep)...");
    Serial.flush(); 

    vTaskDelay(pdMS_TO_TICKS(50));

    lcd.clear();
    lcd.noBacklight();

    // Chờ người dùng nhả nút đánh thức nếu đang giữ
    while (digitalRead(WAKEUP_PIN) == LOW) {
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
    vTaskDelay(pdMS_TO_TICKS(50)); 

    // Bắt đầu Light Sleep
    esp_light_sleep_start();

    // === ĐÃ ĐƯỢC ĐÁNH THỨC (IDLE Interrupt -> S2) ===
    Serial.println("\n[S1 -> S2] ESP32-S3 da thuc day boi IDLE Interrupt!");
    isAwake = true;
    QR_SetScanningEnabled(true);
    QR_ClearCode();

    wakeUpTimer = millis();
    lcd.backlight();
    LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);

    currentState = S2_WAIT_INPUT;
}

void setup() {
    Serial.begin(115200);
    Serial.println();  
    Serial.println("==================================================");
    Serial.println("   SMART LOCKER SYSTEM - KHOI DONG (S0_INIT)     ");
    Serial.println("==================================================");

    setupWakeupPin();

    // 1. Khởi tạo toàn bộ ngoại vi phần cứng
    Locker_Init();
    Keypad_Init();
    LCDInit();

    if (!initCamera()) {
        Serial.println("[Camera] Khoi tao camera that bai!");
        return;
    }
    configureSensor();

    // 2. Tạo task quét QR chạy độc lập trên Core 1
    xTaskCreatePinnedToCore(
        qrScannerTask,    // Hàm task
        "qrScannerTask",  // Tên task
        1024 * 24,        // Kích thước stack
        NULL,             // Tham số
        5,                // Độ ưu tiên
        NULL,             // Task handle
        1                 // Chạy trên Core 1
    );

    // 3. S0: Đọc Flash để khôi phục trạng thái các ngăn tủ
    syncSlotsFromFlash();

    Serial.println(">>> S0 hoan tat! Chuyen sang trang thai S2 san sang.");
    isAwake = true;
    QR_SetScanningEnabled(true);
    wakeUpTimer = millis();
    LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);

    currentState = S2_WAIT_INPUT;
}

void loop() {
    switch (currentState) {

        // ======================================================================
        // S1: NGỦ (LIGHT SLEEP)
        // ======================================================================
        case S1_SLEEP: {
            enterSleepState();
            break;
        }

        // ======================================================================
        // S2: ĐÁNH THỨC, BẬT LCD, CHỜ QR / KEYPAD
        // ======================================================================
        case S2_WAIT_INPUT: {
            // Kiểm tra Timeout: Quá 10s không có thao tác -> Vào Light Sleep (S2 -> S1)
            if (millis() - wakeUpTimer > SCAN_TIMEOUT_MS) {
                Serial.println("[S2 -> S1] Timeout 10s khong co thao tac. Chuyen sang S1 (Ngu).");
                currentState = S1_SLEEP;
                break;
            }

            // Sự kiện 1: Đọc được mã QR từ camera (S2 -> S3)
            if (QR_HasNewCode(currentOrder)) {
                Serial.printf("[S2 -> S3] Phat hien ma QR: %s! Chuyen sang S3.\n", currentOrder.MaDH);
                wakeUpTimer = millis();
                currentState = S3_CHECK_DELIVERY;
                break;
            }

            // Sự kiện 2: Có thao tác bấm phím từ Keypad (S2 -> S4)
            char key = Keypad_GetKey();
            if (key != '\0') {
                Serial.printf("[S2 -> S4] Nhan phim Keypad: %c! Chuyen sang S4.\n", key);
                wakeUpTimer = millis();
                memset(keypadBuffer, 0, sizeof(keypadBuffer));
                keypadBufLen = 0;

                if (key != '*' && key != '#') {
                    keypadBuffer[keypadBufLen++] = key;
                    keypadBuffer[keypadBufLen] = '\0';
                }
                LCDShowKeypadInput(keypadBuffer);
                currentState = S4_KEYPAD_INPUT;
                break;
            }

            delay(30);
            break;
        }

        // ======================================================================
        // S3: KIỂM TRA GIAO HÀNG (ĐỌC QR, KIỂM TRA)
        // ======================================================================
        case S3_CHECK_DELIVERY: {
            Serial.printf("[S3] Kiem tra thong tin don hang: MaDH='%s', NV='%s', KH='%s'\n",
                          currentOrder.MaDH, currentOrder.NVGH, currentOrder.KH);

            // Kiểm tra 1: Mã đơn hàng hợp lệ
            if (strlen(currentOrder.MaDH) == 0) {
                Serial.println("[S3 -> S2] Ma QR khong hop le!");
                LCDShowError("Ma QR khong", "hop le!");
                Buzzer_Beep(300);
                delay(2000);
                wakeUpTimer = millis();
                LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);
                currentState = S2_WAIT_INPUT;
                break;
            }

            // Kiểm tra 2: Đơn hàng đã tồn tại trong tủ hay chưa
            if (isOrderCodeExists(currentOrder.MaDH)) {
                Serial.printf("[S3 -> S2] Don hang '%s' da ton tai trong tu!\n", currentOrder.MaDH);
                LCDShowError("Don da ton tai!", currentOrder.MaDH);
                Buzzer_Beep(400);
                delay(2000);
                wakeUpTimer = millis();
                LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);
                currentState = S2_WAIT_INPUT;
                break;
            }

            // Kiểm tra 3: Tìm ngăn tủ trống có số nhỏ nhất
            currentSlot = Sensor_FindLowestEmptySlot();
            if (currentSlot == 0) {
                Serial.println("[S3 -> S2] Ca 2 ngan tu deu da CO HANG! Tu da day.");
                LCDShowError("Tu da day!", "Khong con ngan trong");
                for (int i = 0; i < 3; i++) {
                    Buzzer_Beep(100);
                    delay(100);
                }
                delay(1500);
                wakeUpTimer = millis();
                LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);
                currentState = S2_WAIT_INPUT;
                break;
            }

            // Mọi điều kiện hợp lệ -> gán flag = 0 (Gửi hàng) và chuyển sang S5
            currentOrder.slot_id = currentSlot;
            currentFlag = 0;
            Serial.printf("[S3 -> S5] Hop le! Chon NGAN %d. flag = 0. Chuyen sang S5.\n", currentSlot);

            LCDShowDeliveryInfo(currentOrder, currentSlot);
            delay(1500);

            currentState = S5_DOOR_PROCESS;
            break;
        }

        // ======================================================================
        // S4: INPUT TỪ KEYPAD (NHẬP MÃ LẤY HÀNG)
        // ======================================================================
        case S4_KEYPAD_INPUT: {
            // Kiểm tra timeout khi đang nhập mã (15s)
            if (millis() - wakeUpTimer > KEYPAD_TIMEOUT_MS) {
                Serial.println("[S4 -> S2] Timeout nhap ma! Chuyen ve S2.");
                LCDShowError("Qua thoi gian!", "Huy thao tac.");
                delay(1500);
                wakeUpTimer = millis();
                LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);
                currentState = S2_WAIT_INPUT;
                break;
            }

            char key = Keypad_GetKey();
            if (key != '\0') {
                wakeUpTimer = millis(); // Gia hạn thời gian chờ

                if (key == '#') {
                    // PHÍM ENTER: XÁC NHẬN MÃ NHẬP
                    if (keypadBufLen == 0) {
                        Buzzer_Beep(50);
                        break;
                    }

                    Serial.printf("[S4] Xac nhan ma nhap: '%s'. Dang tim kiem trong Flash...\n", keypadBuffer);

                    // Tìm đơn hàng khớp mã trong Flash
                    bool found = findPackageByCode(keypadBuffer, currentOrder);

                    if (found) {
                        currentSlot = currentOrder.slot_id;
                        currentFlag = 1; // flag = 1: Nhận hàng
                        Serial.printf("[S4 -> S5] Tim thay don '%s' tai Ngan %d! flag = 1. Chuyen sang S5.\n",
                                      currentOrder.MaDH, currentSlot);
                        LCDShowError("Tim thay don!", currentOrder.MaDH);
                        delay(1200);
                        currentState = S5_DOOR_PROCESS;
                        break;
                    } else {
                        Serial.printf("[S4 -> S2] Khong tim thay don voi ma: '%s'!\n", keypadBuffer);
                        LCDShowError("Sai ma don hang!", "Khong tim thay");
                        Buzzer_Beep(400);
                        delay(2000);
                        wakeUpTimer = millis();
                        LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);
                        currentState = S2_WAIT_INPUT;
                        break;
                    }
                } else if (key == '*') {
                    // PHÍM XÓA / HỦY
                    if (keypadBufLen > 0) {
                        keypadBuffer[--keypadBufLen] = '\0';
                        LCDShowKeypadInput(keypadBuffer);
                    } else {
                        // Nhấn '*' khi rỗng -> Hủy thao tác và quay về S2
                        Serial.println("[S4 -> S2] Nguoi dung bam huy thao tac (*).");
                        LCDShowError("Da huy thao tac!");
                        delay(1000);
                        wakeUpTimer = millis();
                        LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);
                        currentState = S2_WAIT_INPUT;
                        break;
                    }
                } else {
                    // CÁC PHÍM KÝ TỰ BÌNH THƯỜNG
                    if (keypadBufLen < (uint8_t)sizeof(keypadBuffer) - 1) {
                        keypadBuffer[keypadBufLen++] = key;
                        keypadBuffer[keypadBufLen] = '\0';
                        LCDShowKeypadInput(keypadBuffer);
                        Buzzer_Beep(30);
                    }
                }
            }

            delay(30);
            break;
        }

        // ======================================================================
        // S5: MỞ NGĂN TỦ, CHỜ ĐÓNG CỬA
        // ======================================================================
        case S5_DOOR_PROCESS: {
            // Tạm dừng quét camera để nhường tài nguyên và tránh nhiễu
            QR_SetScanningEnabled(false);

            // Mở tủ, đợi thao tác mở và đóng cửa, sau đó kiểm tra cảm biến hàng
            bool success = Locker_ProcessDoor(currentSlot, currentFlag);

            QR_SetScanningEnabled(true);

            if (!success) {
                // Timeout không mở cửa hoặc cảm biến hàng không đúng điều kiện
                Serial.println("[S5 -> S2] Thao tac cua hoac cam bien hang that bai! Quay ve S2.");
                wakeUpTimer = millis();
                LCDReady(g_slots[0].has_parcel, g_slots[1].has_parcel);
                currentState = S2_WAIT_INPUT;
            } else {
                // Thao tác thành công:
                // Nếu flag == 0 -> S6 (Lưu đơn)
                // Nếu flag == 1 -> S7 (Xóa đơn)
                if (currentFlag == 0) {
                    Serial.println("[S5 -> S6] flag == 0 (Gui hang hop le). Chuyen sang S6.");
                    currentState = S6_SAVE_ORDER;
                } else {
                    Serial.println("[S5 -> S7] flag == 1 (Nhan hang hop le). Chuyen sang S7.");
                    currentState = S7_CLEAR_ORDER;
                }
            }
            break;
        }

        // ======================================================================
        // S6: LƯU ĐƠN VÀ SỐ NGĂN (KHI flag == 0)
        // ======================================================================
        case S6_SAVE_ORDER: {
            Serial.printf("[S6] Luu don hang '%s' va Ngan %d vao Flash...\n",
                          currentOrder.MaDH, currentSlot);

            // 1. Lưu thông tin đơn vào Flash
            savePackageToSlot(currentSlot, currentOrder);

            // 2. Cập nhật trạng thái bộ nhớ RAM và bật LED ngăn tủ
            g_slots[currentSlot - 1].state = SLOT_OCCUPIED;
            g_slots[currentSlot - 1].has_parcel = true;
            Sensor_SetSlotLED(currentSlot, true);

            // 3. Hiển thị thông báo thành công và phát âm thanh
            LCDShowSuccess(currentSlot, 0);
            Buzzer_Beep(150);
            delay(100);
            Buzzer_Beep(150);

            delay(3000); // Giữ thông báo 3 giây

            // Chuyển sang S1 (Ngủ) theo lưu đồ
            Serial.println("[S6 -> S1] Giao hang thanh cong. Chuyen sang S1 (Ngu).");
            currentState = S1_SLEEP;
            break;
        }

        // ======================================================================
        // S7: XÓA ĐƠN HÀNG ĐÃ LẤY, TRẠNG THÁI NGĂN THÀNH TRỐNG (KHI flag == 1)
        // ======================================================================
        case S7_CLEAR_ORDER: {
            Serial.printf("[S7] Xoa don hang tai Ngan %d khoi Flash...\n", currentSlot);

            // 1. Xóa thông tin đơn khỏi Flash
            removePackageFromSlot(currentSlot);

            // 2. Cập nhật trạng thái bộ nhớ RAM và tắt LED ngăn tủ
            g_slots[currentSlot - 1].state = SLOT_EMPTY;
            g_slots[currentSlot - 1].has_parcel = false;
            Sensor_SetSlotLED(currentSlot, false);

            // 3. Hiển thị thông báo thành công và phát âm thanh
            LCDShowSuccess(currentSlot, 1);
            Buzzer_Beep(150);
            delay(100);
            Buzzer_Beep(150);

            delay(3000); // Giữ thông báo 3 giây

            // Chuyển sang S1 (Ngủ) theo lưu đồ
            Serial.println("[S7 -> S1] Nhan hang thanh cong. Chuyen sang S1 (Ngu).");
            currentState = S1_SLEEP;
            break;
        }

        default:
            currentState = S2_WAIT_INPUT;
            break;
    }
}
