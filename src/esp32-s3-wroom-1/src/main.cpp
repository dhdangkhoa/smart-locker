#include "camera_init.h"
#include "qrcode_reader.h"

#define WAKEUP_PIN 20

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  pinMode(WAKEUP_PIN, INPUT_PULLUP); // keo len nguon, nhan nut noi gnd de danh thuc


  Serial.println();  
  if (!initCamera()) {
    Serial.println("Camera init failed!");
    return;
  }

  configureSensor();

  xTaskCreatePinnedToCore(
    qrScannerTask,    // Task function
    "qrScannerTask",  // Name of task
    1024 * 24,         // Stack size (6KB is plenty for grayscale processing)
    NULL,             // Parameter passed to task
    5,                // Priority
    NULL,             // Task handle
    1                 // Pin to Core 1
  );

}

void loop() {

  delay(100); 
  gpio_wakeup_enable((gpio_num_t)WAKEUP_PIN, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  
  esp_light_sleep_start();


  delay(300);
  
}

