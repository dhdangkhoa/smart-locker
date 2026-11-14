#include "camera_init.h"
#include "qrcode_reader.h"


void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
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

  delay(300);
  //displayLCD();
}

