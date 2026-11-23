#include "camera_init.h"
#include "qrcode_reader.h"
#include "locker_control.h"
#include "custom_keypad.h"

#define WAKEUP_PIN 21 
#define SCAN_TIMEOUT_MS 10000 

volatile bool isAwake = true;
volatile unsigned long wakeUpTimer = 0;

void setupWakeupPin() {
  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  // Cấu hình đánh thức khi chân WAKEUP_PIN ở mức LOW (nhấn nút nối GND)
  gpio_wakeup_enable((gpio_num_t)WAKEUP_PIN, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
}

void enterLightSleep() {
  isAwake = false;

  Serial.println(">>> Het 10s. Vao che do Light Sleep...");
  Serial.flush(); 

  // Chờ task quét nhả frame
  vTaskDelay(pdMS_TO_TICKS(50));

  lcd.clear();
  lcd.noBacklight();

  // Chờ người dùng nhả nút nếu đang giữ
  while (digitalRead(WAKEUP_PIN) == LOW) {
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
  vTaskDelay(pdMS_TO_TICKS(50)); 

  // Vào Light Sleep
  esp_light_sleep_start();

  // === ĐÃ THỨC DẬY ===
  Serial.println(">>> ESP32-S3 da thuc day!");
  isAwake = true;
  wakeUpTimer = millis();

  lcd.backlight();
  LCDReady();
}

void setup() {
  Serial.begin(115200);
  Serial.println();  

  setupWakeupPin();

  // Khởi tạo các hệ thống cảm biến, servo, buzzer và bàn phím
  Locker_Init();
  Keypad_Init();

  if (!initCamera()) {
    Serial.println("Camera init failed!");
    return;
  }

  configureSensor();

  xTaskCreatePinnedToCore(
    qrScannerTask,    // Task function
    "qrScannerTask",  // Name of task
    1024 * 24,        // Stack size
    NULL,             // Parameter passed to task
    5,                // Priority
    NULL,             // Task handle
    1                 // Pin to Core 1
  );

  // KÍCH HOẠT ĐẾM NGƯỢC NGAY TỪ ĐẦU
  isAwake = true;
  wakeUpTimer = millis();
  Serial.println(">>> He thong san sang! Bat dau dem nguoc 10s...");
}

void loop() {
  if (isAwake) {
    // 1. Kiểm tra phím bấm từ bàn phím ma trận (nếu có bấm phím thì gia hạn 10s)
    char key = Keypad_GetKey();
    if (key) {
      Serial.printf("[Keypad] Phim: %c\n", key);
      wakeUpTimer = millis();
    }

    // 2. Kiểm tra các nút nhấn QR giao hàng
    if (Sensor_IsButtonPressed(PIN_BTN_QR1)) {
      wakeUpTimer = millis();
      Locker_RunSlotDelivery(1);
    }
    if (Sensor_IsButtonPressed(PIN_BTN_QR2)) {
      wakeUpTimer = millis();
      Locker_RunSlotDelivery(2);
    }
    if (Sensor_IsButtonPressed(PIN_BTN_QR3)) {
      wakeUpTimer = millis();
      Locker_RunSlotDelivery(3);
    }

    // 3. Kiểm tra nếu đã quá 10 giây mà không có hoạt động thì đi ngủ
    if (millis() - wakeUpTimer > SCAN_TIMEOUT_MS) {
      enterLightSleep();
    }
  }

  delay(50);
}
